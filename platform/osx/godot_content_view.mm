/*************************************************************************/
/*  godot_content_view.mm                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "godot_content_view.h"

#include "display_server_osx.h"
#include "key_mapping_osx.h"

@implementation GodotContentView

- (id)init {
	self = [super init];
	window_id = DisplayServer::INVALID_WINDOW_ID;
	tracking_area = nil;
	ime_input_event_in_progress = false;
	mouse_down_control = false;
	ignore_momentum_scroll = false;
	[self updateTrackingAreas];

	if (@available(macOS 10.13, *)) {
		[self registerForDraggedTypes:[NSArray arrayWithObject:NSPasteboardTypeFileURL]];
#if !defined(__aarch64__) // Do not build deprectead 10.13 code on ARM.
	} else {
		[self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
#endif
	}
	marked_text = [[NSMutableAttributedString alloc] init];
	return self;
}

- (void)setWindowID:(DisplayServerOSX::WindowID)wid {
	window_id = wid;
}

// MARK: Backing Layer

- (CALayer *)makeBackingLayer {
	return [[CAMetalLayer class] layer];
}

- (void)updateLayer {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	ds->window_update(window_id);
	[super updateLayer];
}

- (BOOL)wantsUpdateLayer {
	return YES;
}

- (BOOL)isOpaque {
	return YES;
}

// MARK: IME

- (BOOL)hasMarkedText {
	return (marked_text.length > 0);
}

- (NSRange)markedRange {
	return NSMakeRange(0, marked_text.length);
}

- (NSRange)selectedRange {
	static const NSRange kEmptyRange = { NSNotFound, 0 };
	return kEmptyRange;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
	if ([aString isKindOfClass:[NSAttributedString class]]) {
		marked_text = [[NSMutableAttributedString alloc] initWithAttributedString:aString];
	} else {
		marked_text = [[NSMutableAttributedString alloc] initWithString:aString];
	}
	if (marked_text.length == 0) {
		[self unmarkText];
		return;
	}

	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	if (wd.im_active) {
		ime_input_event_in_progress = true;
		ds->update_im_text(Point2i(selectedRange.location, selectedRange.length), String::utf8([[marked_text mutableString] UTF8String]));
	}
}

- (void)doCommandBySelector:(SEL)aSelector {
	[self tryToPerform:aSelector with:self];
}

- (void)unmarkText {
	ime_input_event_in_progress = false;
	[[marked_text mutableString] setString:@""];

	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	if (wd.im_active) {
		ds->update_im_text(Point2i(), String());
	}
}

- (NSArray *)validAttributesForMarkedText {
	return [NSArray array];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange {
	return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint {
	return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return NSMakeRect(0, 0, 0, 0);
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	const NSRect content_rect = [wd.window_view frame];
	const float scale = ds->screen_get_max_scale();
	NSRect point_in_window_rect = NSMakeRect(wd.im_position.x / scale, content_rect.size.height - (wd.im_position.y / scale) - 1, 0, 0);
	NSPoint point_on_screen = [wd.window_object convertRectToScreen:point_in_window_rect].origin;

	return NSMakeRect(point_on_screen.x, point_on_screen.y, 0, 0);
}

- (void)cancelComposition {
	[self unmarkText];
	[[NSTextInputContext currentInputContext] discardMarkedText];
}

- (void)insertText:(id)aString {
	[self insertText:aString replacementRange:NSMakeRange(0, 0)];
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange {
	NSEvent *event = [NSApp currentEvent];

	NSString *characters;
	if ([aString isKindOfClass:[NSAttributedString class]]) {
		characters = [aString string];
	} else {
		characters = (NSString *)aString;
	}

	NSCharacterSet *ctrl_chars = [NSCharacterSet controlCharacterSet];
	NSCharacterSet *wsnl_chars = [NSCharacterSet whitespaceAndNewlineCharacterSet];
	if ([characters rangeOfCharacterFromSet:ctrl_chars].length && [characters rangeOfCharacterFromSet:wsnl_chars].length == 0) {
		[[NSTextInputContext currentInputContext] discardMarkedText];
		[self cancelComposition];
		return;
	}

	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		[self cancelComposition];
		return;
	}

	Char16String text;
	text.resize([characters length] + 1);
	[characters getCharacters:(unichar *)text.ptrw() range:NSMakeRange(0, [characters length])];

	String u32text;
	u32text.parse_utf16(text.ptr(), text.length());

	for (int i = 0; i < u32text.length(); i++) {
		const char32_t codepoint = u32text[i];
		if ((codepoint & 0xFF00) == 0xF700) {
			continue;
		}

		DisplayServerOSX::KeyEvent ke;

		ke.window_id = window_id;
		ke.osx_state = [event modifierFlags];
		ke.pressed = true;
		ke.echo = false;
		ke.raw = false; // IME input event.
		ke.keycode = Key::NONE;
		ke.physical_keycode = Key::NONE;
		ke.unicode = codepoint;

		ds->push_to_key_event_buffer(ke);
	}
	[self cancelComposition];
}

// MARK: Drag and drop

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
	return NSDragOperationCopy;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
	return NSDragOperationCopy;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return NO;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	if (!wd.drop_files_callback.is_null()) {
		Vector<String> files;
		NSPasteboard *pboard = [sender draggingPasteboard];

		if (@available(macOS 10.13, *)) {
			NSArray *items = pboard.pasteboardItems;
			for (NSPasteboardItem *item in items) {
				NSString *url = [item stringForType:NSPasteboardTypeFileURL];
				NSString *file = [NSURL URLWithString:url].path;
				files.push_back(String::utf8([file UTF8String]));
			}
#if !defined(__aarch64__) // Do not build deprectead 10.13 code on ARM.
		} else {
			NSArray *filenames = [pboard propertyListForType:NSFilenamesPboardType];
			for (NSString *file in filenames) {
				files.push_back(String::utf8([file UTF8String]));
			}
#endif
		}

		Variant v = files;
		Variant *vp = &v;
		Variant ret;
		Callable::CallError ce;
		wd.drop_files_callback.call((const Variant **)&vp, 1, ret, ce);
	}

	return NO;
}

// MARK: Focus

- (BOOL)canBecomeKeyView {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return YES;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	return !wd.no_focus;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

// MARK: Mouse

- (void)cursorUpdate:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds) {
		return;
	}

	ds->cursor_update_shape();
}

- (void)processMouseEvent:(NSEvent *)event index:(MouseButton)index mask:(MouseButton)mask pressed:(bool)pressed {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	MouseButton last_button_state = ds->mouse_get_button_state();

	if (pressed) {
		last_button_state |= mask;
	} else {
		last_button_state &= (MouseButton)~mask;
	}
	ds->mouse_set_button_state(last_button_state);

	Ref<InputEventMouseButton> mb;
	mb.instantiate();
	mb->set_window_id(window_id);
	ds->update_mouse_pos(wd, [event locationInWindow]);
	ds->get_key_modifier_state([event modifierFlags], mb);
	mb->set_button_index(index);
	mb->set_pressed(pressed);
	mb->set_position(wd.mouse_pos);
	mb->set_global_position(wd.mouse_pos);
	mb->set_button_mask(last_button_state);
	if (index == MouseButton::LEFT && pressed) {
		mb->set_double_click([event clickCount] == 2);
	}

	Input::get_singleton()->parse_input_event(mb);
}

- (void)mouseDown:(NSEvent *)event {
	if (([event modifierFlags] & NSEventModifierFlagControl)) {
		mouse_down_control = true;
		[self processMouseEvent:event index:MouseButton::RIGHT mask:MouseButton::MASK_RIGHT pressed:true];
	} else {
		mouse_down_control = false;
		[self processMouseEvent:event index:MouseButton::LEFT mask:MouseButton::MASK_LEFT pressed:true];
	}
}

- (void)mouseDragged:(NSEvent *)event {
	[self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event {
	if (mouse_down_control) {
		[self processMouseEvent:event index:MouseButton::RIGHT mask:MouseButton::MASK_RIGHT pressed:false];
	} else {
		[self processMouseEvent:event index:MouseButton::LEFT mask:MouseButton::MASK_LEFT pressed:false];
	}
}

- (void)mouseMoved:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);

	NSPoint delta = NSMakePoint([event deltaX], [event deltaY]);
	NSPoint mpos = [event locationInWindow];

	if (ds->update_mouse_wrap(wd, delta, mpos, [event timestamp])) {
		return;
	}

	Ref<InputEventMouseMotion> mm;
	mm.instantiate();

	mm->set_window_id(window_id);
	mm->set_button_mask(ds->mouse_get_button_state());
	ds->update_mouse_pos(wd, mpos);
	mm->set_position(wd.mouse_pos);
	mm->set_pressure([event pressure]);
	if ([event subtype] == NSEventSubtypeTabletPoint) {
		const NSPoint p = [event tilt];
		mm->set_tilt(Vector2(p.x, p.y));
	}
	mm->set_global_position(wd.mouse_pos);
	mm->set_velocity(Input::get_singleton()->get_last_mouse_velocity());
	const Vector2i relativeMotion = Vector2i(delta.x, delta.y) * ds->screen_get_max_scale();
	mm->set_relative(relativeMotion);
	ds->get_key_modifier_state([event modifierFlags], mm);

	Input::get_singleton()->parse_input_event(mm);
}

- (void)rightMouseDown:(NSEvent *)event {
	[self processMouseEvent:event index:MouseButton::RIGHT mask:MouseButton::MASK_RIGHT pressed:true];
}

- (void)rightMouseDragged:(NSEvent *)event {
	[self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event {
	[self processMouseEvent:event index:MouseButton::RIGHT mask:MouseButton::MASK_RIGHT pressed:false];
}

- (void)otherMouseDown:(NSEvent *)event {
	if ((int)[event buttonNumber] == 2) {
		[self processMouseEvent:event index:MouseButton::MIDDLE mask:MouseButton::MASK_MIDDLE pressed:true];
	} else if ((int)[event buttonNumber] == 3) {
		[self processMouseEvent:event index:MouseButton::MB_XBUTTON1 mask:MouseButton::MASK_XBUTTON1 pressed:true];
	} else if ((int)[event buttonNumber] == 4) {
		[self processMouseEvent:event index:MouseButton::MB_XBUTTON2 mask:MouseButton::MASK_XBUTTON2 pressed:true];
	} else {
		return;
	}
}

- (void)otherMouseDragged:(NSEvent *)event {
	[self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event {
	if ((int)[event buttonNumber] == 2) {
		[self processMouseEvent:event index:MouseButton::MIDDLE mask:MouseButton::MASK_MIDDLE pressed:false];
	} else if ((int)[event buttonNumber] == 3) {
		[self processMouseEvent:event index:MouseButton::MB_XBUTTON1 mask:MouseButton::MASK_XBUTTON1 pressed:false];
	} else if ((int)[event buttonNumber] == 4) {
		[self processMouseEvent:event index:MouseButton::MB_XBUTTON2 mask:MouseButton::MASK_XBUTTON2 pressed:false];
	} else {
		return;
	}
}

- (void)mouseExited:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	if (ds->mouse_get_mode() != DisplayServer::MOUSE_MODE_CAPTURED) {
		ds->send_window_event(wd, DisplayServerOSX::WINDOW_EVENT_MOUSE_EXIT);
	}
}

- (void)mouseEntered:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	if (ds->mouse_get_mode() != DisplayServer::MOUSE_MODE_CAPTURED) {
		ds->send_window_event(wd, DisplayServerOSX::WINDOW_EVENT_MOUSE_ENTER);
	}

	ds->cursor_update_shape();
}

- (void)magnifyWithEvent:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);

	Ref<InputEventMagnifyGesture> ev;
	ev.instantiate();
	ev->set_window_id(window_id);
	ds->get_key_modifier_state([event modifierFlags], ev);
	ds->update_mouse_pos(wd, [event locationInWindow]);
	ev->set_position(wd.mouse_pos);
	ev->set_factor([event magnification] + 1.0);

	Input::get_singleton()->parse_input_event(ev);
}

- (void)updateTrackingAreas {
	if (tracking_area != nil) {
		[self removeTrackingArea:tracking_area];
	}

	NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingCursorUpdate | NSTrackingInVisibleRect;
	tracking_area = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];

	[self addTrackingArea:tracking_area];
	[super updateTrackingAreas];
}

// MARK: Keyboard

- (void)keyDown:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);

	ignore_momentum_scroll = true;

	// Ignore all input if IME input is in progress.
	if (!ime_input_event_in_progress) {
		NSString *characters = [event characters];
		NSUInteger length = [characters length];

		if (!wd.im_active && length > 0 && keycode_has_unicode(KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]))) {
			// Fallback unicode character handler used if IME is not active.
			Char16String text;
			text.resize([characters length] + 1);
			[characters getCharacters:(unichar *)text.ptrw() range:NSMakeRange(0, [characters length])];

			String u32text;
			u32text.parse_utf16(text.ptr(), text.length());

			for (int i = 0; i < u32text.length(); i++) {
				const char32_t codepoint = u32text[i];

				DisplayServerOSX::KeyEvent ke;

				ke.window_id = window_id;
				ke.osx_state = [event modifierFlags];
				ke.pressed = true;
				ke.echo = [event isARepeat];
				ke.keycode = KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]);
				ke.physical_keycode = KeyMappingOSX::translate_key([event keyCode]);
				ke.raw = true;
				ke.unicode = codepoint;

				ds->push_to_key_event_buffer(ke);
			}
		} else {
			DisplayServerOSX::KeyEvent ke;

			ke.window_id = window_id;
			ke.osx_state = [event modifierFlags];
			ke.pressed = true;
			ke.echo = [event isARepeat];
			ke.keycode = KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]);
			ke.physical_keycode = KeyMappingOSX::translate_key([event keyCode]);
			ke.raw = false;
			ke.unicode = 0;

			ds->push_to_key_event_buffer(ke);
		}
	}

	// Pass events to IME handler
	if (wd.im_active) {
		[self interpretKeyEvents:[NSArray arrayWithObject:event]];
	}
}

- (void)flagsChanged:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}
	ignore_momentum_scroll = true;

	// Ignore all input if IME input is in progress
	if (!ime_input_event_in_progress) {
		DisplayServerOSX::KeyEvent ke;

		ke.window_id = window_id;
		ke.echo = false;
		ke.raw = true;

		int key = [event keyCode];
		int mod = [event modifierFlags];

		if (key == 0x36 || key == 0x37) {
			if (mod & NSEventModifierFlagCommand) {
				mod &= ~NSEventModifierFlagCommand;
				ke.pressed = true;
			} else {
				ke.pressed = false;
			}
		} else if (key == 0x38 || key == 0x3c) {
			if (mod & NSEventModifierFlagShift) {
				mod &= ~NSEventModifierFlagShift;
				ke.pressed = true;
			} else {
				ke.pressed = false;
			}
		} else if (key == 0x3a || key == 0x3d) {
			if (mod & NSEventModifierFlagOption) {
				mod &= ~NSEventModifierFlagOption;
				ke.pressed = true;
			} else {
				ke.pressed = false;
			}
		} else if (key == 0x3b || key == 0x3e) {
			if (mod & NSEventModifierFlagControl) {
				mod &= ~NSEventModifierFlagControl;
				ke.pressed = true;
			} else {
				ke.pressed = false;
			}
		} else {
			return;
		}

		ke.osx_state = mod;
		ke.keycode = KeyMappingOSX::remap_key(key, mod);
		ke.physical_keycode = KeyMappingOSX::translate_key(key);
		ke.unicode = 0;

		ds->push_to_key_event_buffer(ke);
	}
}

- (void)keyUp:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);

	// Ignore all input if IME input is in progress.
	if (!ime_input_event_in_progress) {
		NSString *characters = [event characters];
		NSUInteger length = [characters length];

		// Fallback unicode character handler used if IME is not active.
		if (!wd.im_active && length > 0 && keycode_has_unicode(KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]))) {
			Char16String text;
			text.resize([characters length] + 1);
			[characters getCharacters:(unichar *)text.ptrw() range:NSMakeRange(0, [characters length])];

			String u32text;
			u32text.parse_utf16(text.ptr(), text.length());

			for (int i = 0; i < u32text.length(); i++) {
				const char32_t codepoint = u32text[i];
				DisplayServerOSX::KeyEvent ke;

				ke.window_id = window_id;
				ke.osx_state = [event modifierFlags];
				ke.pressed = false;
				ke.echo = [event isARepeat];
				ke.keycode = KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]);
				ke.physical_keycode = KeyMappingOSX::translate_key([event keyCode]);
				ke.raw = true;
				ke.unicode = codepoint;

				ds->push_to_key_event_buffer(ke);
			}
		} else {
			DisplayServerOSX::KeyEvent ke;

			ke.window_id = window_id;
			ke.osx_state = [event modifierFlags];
			ke.pressed = false;
			ke.echo = [event isARepeat];
			ke.keycode = KeyMappingOSX::remap_key([event keyCode], [event modifierFlags]);
			ke.physical_keycode = KeyMappingOSX::translate_key([event keyCode]);
			ke.raw = true;
			ke.unicode = 0;

			ds->push_to_key_event_buffer(ke);
		}
	}
}

// MARK: Scroll and pan

- (void)processScrollEvent:(NSEvent *)event button:(MouseButton)button factor:(double)factor {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	MouseButton mask = mouse_button_to_mask(button);

	Ref<InputEventMouseButton> sc;
	sc.instantiate();

	sc->set_window_id(window_id);
	ds->get_key_modifier_state([event modifierFlags], sc);
	sc->set_button_index(button);
	sc->set_factor(factor);
	sc->set_pressed(true);
	sc->set_position(wd.mouse_pos);
	sc->set_global_position(wd.mouse_pos);
	MouseButton last_button_state = ds->mouse_get_button_state() | (MouseButton)mask;
	sc->set_button_mask(last_button_state);
	ds->mouse_set_button_state(last_button_state);

	Input::get_singleton()->parse_input_event(sc);

	sc.instantiate();
	sc->set_window_id(window_id);
	sc->set_button_index(button);
	sc->set_factor(factor);
	sc->set_pressed(false);
	sc->set_position(wd.mouse_pos);
	sc->set_global_position(wd.mouse_pos);
	last_button_state &= (MouseButton)~mask;
	sc->set_button_mask(last_button_state);
	ds->mouse_set_button_state(last_button_state);

	Input::get_singleton()->parse_input_event(sc);
}

- (void)processPanEvent:(NSEvent *)event dx:(double)dx dy:(double)dy {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);

	Ref<InputEventPanGesture> pg;
	pg.instantiate();

	pg->set_window_id(window_id);
	ds->get_key_modifier_state([event modifierFlags], pg);
	pg->set_position(wd.mouse_pos);
	pg->set_delta(Vector2(-dx, -dy));

	Input::get_singleton()->parse_input_event(pg);
}

- (void)scrollWheel:(NSEvent *)event {
	DisplayServerOSX *ds = (DisplayServerOSX *)DisplayServer::get_singleton();
	if (!ds || !ds->has_window(window_id)) {
		return;
	}

	DisplayServerOSX::WindowData &wd = ds->get_window(window_id);
	ds->update_mouse_pos(wd, [event locationInWindow]);

	double delta_x = [event scrollingDeltaX];
	double delta_y = [event scrollingDeltaY];

	if ([event hasPreciseScrollingDeltas]) {
		delta_x *= 0.03;
		delta_y *= 0.03;
	}

	if ([event momentumPhase] != NSEventPhaseNone) {
		if (ignore_momentum_scroll) {
			return;
		}
	} else {
		ignore_momentum_scroll = false;
	}

	if ([event phase] != NSEventPhaseNone || [event momentumPhase] != NSEventPhaseNone) {
		[self processPanEvent:event dx:delta_x dy:delta_y];
	} else {
		if (fabs(delta_x)) {
			[self processScrollEvent:event button:(0 > delta_x ? MouseButton::WHEEL_RIGHT : MouseButton::WHEEL_LEFT) factor:fabs(delta_x * 0.3)];
		}
		if (fabs(delta_y)) {
			[self processScrollEvent:event button:(0 < delta_y ? MouseButton::WHEEL_UP : MouseButton::WHEEL_DOWN) factor:fabs(delta_y * 0.3)];
		}
	}
}

@end
