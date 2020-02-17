/*************************************************************************/
/*  global_constants.cpp                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "global_constants.h"

#include "core/object.h"
#include "core/os/input_event.h"
#include "core/os/keyboard.h"
#include "core/variant.h"

struct _GlobalConstant {

#ifdef DEBUG_METHODS_ENABLED
	StringName enum_name;
#endif
	const char *name;
	int value;

	_GlobalConstant() {}

#ifdef DEBUG_METHODS_ENABLED
	_GlobalConstant(const StringName &p_enum_name, const char *p_name, int p_value) :
			enum_name(p_enum_name),
			name(p_name),
			value(p_value) {
	}
#else
	_GlobalConstant(const char *p_name, int p_value) :
			name(p_name),
			value(p_value) {
	}
#endif
};

static Vector<_GlobalConstant> _global_constants;

#ifdef DEBUG_METHODS_ENABLED

#define BIND_GLOBAL_CONSTANT(m_constant) \
	_global_constants.push_back(_GlobalConstant(StringName(), #m_constant, m_constant));

#define BIND_GLOBAL_ENUM_CONSTANT(m_constant) \
	_global_constants.push_back(_GlobalConstant(__constant_get_enum_name(m_constant, #m_constant), #m_constant, m_constant));

#define BIND_GLOBAL_ENUM_CONSTANT_CUSTOM(m_custom_name, m_constant) \
	_global_constants.push_back(_GlobalConstant(__constant_get_enum_name(m_constant, #m_constant), m_custom_name, m_constant));

#else

#define BIND_GLOBAL_CONSTANT(m_constant) \
	_global_constants.push_back(_GlobalConstant(#m_constant, m_constant));

#define BIND_GLOBAL_ENUM_CONSTANT(m_constant) \
	_global_constants.push_back(_GlobalConstant(#m_constant, m_constant));

#define BIND_GLOBAL_ENUM_CONSTANT_CUSTOM(m_custom_name, m_constant) \
	_global_constants.push_back(_GlobalConstant(m_custom_name, m_constant));

#endif

VARIANT_ENUM_CAST(KeyList);
VARIANT_ENUM_CAST(KeyModifierMask);
VARIANT_ENUM_CAST(ButtonList);
VARIANT_ENUM_CAST(JoystickList);
VARIANT_ENUM_CAST(MidiMessageList);

void register_global_constants() {

	//{ KEY_BACKSPACE, VK_BACK },// (0x08) // backspace

	BIND_GLOBAL_ENUM_CONSTANT(MARGIN_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(MARGIN_TOP);
	BIND_GLOBAL_ENUM_CONSTANT(MARGIN_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(MARGIN_BOTTOM);

	BIND_GLOBAL_ENUM_CONSTANT(CORNER_TOP_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(CORNER_TOP_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(CORNER_BOTTOM_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(CORNER_BOTTOM_LEFT);

	BIND_GLOBAL_ENUM_CONSTANT(VERTICAL);
	BIND_GLOBAL_ENUM_CONSTANT(HORIZONTAL);

	BIND_GLOBAL_ENUM_CONSTANT(HALIGN_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(HALIGN_CENTER);
	BIND_GLOBAL_ENUM_CONSTANT(HALIGN_RIGHT);

	BIND_GLOBAL_ENUM_CONSTANT(VALIGN_TOP);
	BIND_GLOBAL_ENUM_CONSTANT(VALIGN_CENTER);
	BIND_GLOBAL_ENUM_CONSTANT(VALIGN_BOTTOM);

	// hueg list of keys
	BIND_GLOBAL_CONSTANT(SPKEY);

	BIND_GLOBAL_ENUM_CONSTANT(KEY_ESCAPE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_TAB);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BACKTAB);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BACKSPACE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ENTER);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_ENTER);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_INSERT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DELETE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PAUSE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PRINT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SYSREQ);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CLEAR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HOME);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_END);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PAGEUP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PAGEDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SHIFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CONTROL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_META);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ALT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CAPSLOCK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_NUMLOCK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SCROLLLOCK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F1);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F2);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F3);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F4);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F5);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F6);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F7);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F8);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F9);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F10);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F11);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F12);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F13);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F14);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F15);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F16);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_MULTIPLY);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_DIVIDE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_SUBTRACT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_PERIOD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_ADD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_0);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_1);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_2);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_3);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_4);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_5);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_6);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_7);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_8);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_KP_9);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SUPER_L);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SUPER_R);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MENU);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HYPER_L);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HYPER_R);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HELP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DIRECTION_L);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DIRECTION_R);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BACK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_FORWARD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_STOP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_REFRESH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_VOLUMEDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_VOLUMEMUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_VOLUMEUP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BASSBOOST);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BASSUP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BASSDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_TREBLEUP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_TREBLEDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MEDIAPLAY);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MEDIASTOP);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MEDIAPREVIOUS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MEDIANEXT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MEDIARECORD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HOMEPAGE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_FAVORITES);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SEARCH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_STANDBY);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OPENURL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHMAIL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHMEDIA);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH0);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH1);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH2);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH3);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH4);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH5);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH6);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH7);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH8);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCH9);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHA);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHB);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHC);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LAUNCHF);

	BIND_GLOBAL_ENUM_CONSTANT(KEY_UNKNOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SPACE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EXCLAM);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_QUOTEDBL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_NUMBERSIGN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DOLLAR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PERCENT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_AMPERSAND);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_APOSTROPHE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PARENLEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PARENRIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ASTERISK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PLUS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_COMMA);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MINUS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PERIOD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SLASH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_0);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_1);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_2);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_3);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_4);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_5);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_6);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_7);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_8);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_9);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_COLON);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SEMICOLON);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_LESS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EQUAL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_GREATER);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_QUESTION);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_AT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_A);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_B);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_C);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_D);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_E);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_F);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_G);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_H);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_I);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_J);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_K);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_L);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_M);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_N);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_O);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_P);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_Q);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_R);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_S);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_T);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_U);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_V);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_W);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_X);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_Y);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_Z);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BRACKETLEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BACKSLASH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BRACKETRIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ASCIICIRCUM);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UNDERSCORE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_QUOTELEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BRACELEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BAR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BRACERIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ASCIITILDE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_NOBREAKSPACE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EXCLAMDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CENT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_STERLING);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CURRENCY);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_YEN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_BROKENBAR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SECTION);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_COPYRIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ORDFEMININE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_GUILLEMOTLEFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_NOTSIGN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_HYPHEN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_REGISTERED);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MACRON);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_DEGREE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PLUSMINUS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_TWOSUPERIOR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_THREESUPERIOR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MU);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PARAGRAPH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_PERIODCENTERED);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CEDILLA);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ONESUPERIOR);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASCULINE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_GUILLEMOTRIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ONEQUARTER);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ONEHALF);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_THREEQUARTERS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_QUESTIONDOWN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_AGRAVE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_AACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ACIRCUMFLEX);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ATILDE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ADIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ARING);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_AE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_CCEDILLA);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EGRAVE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ECIRCUMFLEX);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_EDIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_IGRAVE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_IACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ICIRCUMFLEX);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_IDIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ETH);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_NTILDE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OGRAVE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OCIRCUMFLEX);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OTILDE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_ODIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MULTIPLY);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_OOBLIQUE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UGRAVE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UCIRCUMFLEX);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_UDIAERESIS);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_YACUTE);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_THORN);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_SSHARP);

	BIND_GLOBAL_ENUM_CONSTANT(KEY_DIVISION);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_YDIAERESIS);

	BIND_GLOBAL_ENUM_CONSTANT(KEY_CODE_MASK);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MODIFIER_MASK);

	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_SHIFT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_ALT);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_META);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_CTRL);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_CMD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_KPAD);
	BIND_GLOBAL_ENUM_CONSTANT(KEY_MASK_GROUP_SWITCH);

	// mouse
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MIDDLE);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_XBUTTON1);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_XBUTTON2);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_WHEEL_UP);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_WHEEL_DOWN);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_WHEEL_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_WHEEL_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MASK_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MASK_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MASK_MIDDLE);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MASK_XBUTTON1);
	BIND_GLOBAL_ENUM_CONSTANT(BUTTON_MASK_XBUTTON2);

	//joypads
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_0);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_1);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_2);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_3);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_4);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_5);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_6);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_7);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_8);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_9);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_10);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_11);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_12);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_13);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_14);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_15);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_BUTTON_MAX);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_SONY_CIRCLE);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_SONY_X);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_SONY_SQUARE);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_SONY_TRIANGLE);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_XBOX_B);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_XBOX_A);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_XBOX_X);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_XBOX_Y);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_DS_A);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DS_B);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DS_X);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DS_Y);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_VR_GRIP);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_VR_PAD);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_VR_TRIGGER);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_OCULUS_AX);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_OCULUS_BY);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_OCULUS_MENU);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_OPENVR_MENU);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_SELECT);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_START);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DPAD_UP);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DPAD_DOWN);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DPAD_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_DPAD_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_L);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_L2);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_L3);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_R);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_R2);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_R3);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_0);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_1);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_2);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_3);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_4);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_5);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_6);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_7);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_8);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_9);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_AXIS_MAX);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_LX);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_LY);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_RX);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_RY);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_L2);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_ANALOG_R2);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_VR_ANALOG_TRIGGER);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_VR_ANALOG_GRIP);

	BIND_GLOBAL_ENUM_CONSTANT(JOY_OPENVR_TOUCHPADX);
	BIND_GLOBAL_ENUM_CONSTANT(JOY_OPENVR_TOUCHPADY);

	// midi
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_NOTE_OFF);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_NOTE_ON);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_AFTERTOUCH);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_CONTROL_CHANGE);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_PROGRAM_CHANGE);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_CHANNEL_PRESSURE);
	BIND_GLOBAL_ENUM_CONSTANT(MIDI_MESSAGE_PITCH_BEND);

	// error list

	BIND_GLOBAL_ENUM_CONSTANT(OK); // (0)
	BIND_GLOBAL_ENUM_CONSTANT(FAILED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_UNAVAILABLE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_UNCONFIGURED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_UNAUTHORIZED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_PARAMETER_RANGE_ERROR); // (5)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_OUT_OF_MEMORY);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_NOT_FOUND);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_BAD_DRIVE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_BAD_PATH);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_NO_PERMISSION); // (10)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_ALREADY_IN_USE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_CANT_OPEN);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_CANT_WRITE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_CANT_READ);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_UNRECOGNIZED); // (15)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_CORRUPT);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_MISSING_DEPENDENCIES);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_FILE_EOF);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_OPEN);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_CREATE); // (20)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_QUERY_FAILED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_ALREADY_IN_USE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_LOCKED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_TIMEOUT);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_CONNECT); // (25)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_RESOLVE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CONNECTION_ERROR);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_ACQUIRE_RESOURCE);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CANT_FORK);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_INVALID_DATA); // (30)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_INVALID_PARAMETER);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_ALREADY_EXISTS);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_DOES_NOT_EXIST);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_DATABASE_CANT_READ);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_DATABASE_CANT_WRITE); // (35)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_COMPILATION_FAILED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_METHOD_NOT_FOUND);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_LINK_FAILED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_SCRIPT_FAILED);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_CYCLIC_LINK); // (40)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_INVALID_DECLARATION);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_DUPLICATE_SYMBOL);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_PARSE_ERROR);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_BUSY);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_SKIP); // (45)
	BIND_GLOBAL_ENUM_CONSTANT(ERR_HELP);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_BUG);
	BIND_GLOBAL_ENUM_CONSTANT(ERR_PRINTER_ON_FIRE);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_NONE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_RANGE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_EXP_RANGE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_ENUM);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_EXP_EASING);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_LENGTH);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_KEY_ACCEL);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_FLAGS);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_LAYERS_2D_RENDER);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_LAYERS_2D_PHYSICS);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_LAYERS_3D_RENDER);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_LAYERS_3D_PHYSICS);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_FILE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_DIR);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_GLOBAL_FILE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_GLOBAL_DIR);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_RESOURCE_TYPE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_MULTILINE_TEXT);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_PLACEHOLDER_TEXT);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_COLOR_NO_ALPHA);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_IMAGE_COMPRESS_LOSSY);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_HINT_IMAGE_COMPRESS_LOSSLESS);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_STORAGE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_EDITOR);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_NETWORK);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_EDITOR_HELPER);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_CHECKABLE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_CHECKED);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_INTERNATIONALIZED);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_GROUP);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_CATEGORY);
	//deprecated, replaced by ClassDB function to check default value
	//BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_STORE_IF_NONZERO);
	//BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_STORE_IF_NONONE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_NO_INSTANCE_STATE);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_RESTART_IF_CHANGED);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_SCRIPT_VARIABLE);

	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_DEFAULT);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_DEFAULT_INTL);
	BIND_GLOBAL_ENUM_CONSTANT(PROPERTY_USAGE_NOEDITOR);

	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_NORMAL);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_EDITOR);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_NOSCRIPT);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_CONST);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_REVERSE);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_VIRTUAL);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAG_FROM_SCRIPT);
	BIND_GLOBAL_ENUM_CONSTANT(METHOD_FLAGS_DEFAULT);

	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_NIL", Variant::NIL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_BOOL", Variant::BOOL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_INT", Variant::INT);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_REAL", Variant::REAL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_STRING", Variant::STRING);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_VECTOR2", Variant::VECTOR2); // 5
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_RECT2", Variant::RECT2);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_VECTOR3", Variant::VECTOR3);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_TRANSFORM2D", Variant::TRANSFORM2D);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_PLANE", Variant::PLANE);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_QUAT", Variant::QUAT); // 10
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_AABB", Variant::AABB);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_BASIS", Variant::BASIS);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_TRANSFORM", Variant::TRANSFORM);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_COLOR", Variant::COLOR);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_NODE_PATH", Variant::NODE_PATH); // 15
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_RID", Variant::_RID);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_OBJECT", Variant::OBJECT);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_DICTIONARY", Variant::DICTIONARY); // 20
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_ARRAY", Variant::ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_RAW_ARRAY", Variant::PACKED_BYTE_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_INT_ARRAY", Variant::PACKED_INT_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_REAL_ARRAY", Variant::PACKED_REAL_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_STRING_ARRAY", Variant::PACKED_STRING_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_VECTOR2_ARRAY", Variant::PACKED_VECTOR2_ARRAY); // 25
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_VECTOR3_ARRAY", Variant::PACKED_VECTOR3_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_COLOR_ARRAY", Variant::PACKED_COLOR_ARRAY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("TYPE_MAX", Variant::VARIANT_MAX);

	//comparison
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_EQUAL", Variant::OP_EQUAL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_NOT_EQUAL", Variant::OP_NOT_EQUAL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_LESS", Variant::OP_LESS);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_LESS_EQUAL", Variant::OP_LESS_EQUAL);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_GREATER", Variant::OP_GREATER);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_GREATER_EQUAL", Variant::OP_GREATER_EQUAL);
	//mathematic
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_ADD", Variant::OP_ADD);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_SUBTRACT", Variant::OP_SUBTRACT);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_MULTIPLY", Variant::OP_MULTIPLY);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_DIVIDE", Variant::OP_DIVIDE);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_NEGATE", Variant::OP_NEGATE);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_POSITIVE", Variant::OP_POSITIVE);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_MODULE", Variant::OP_MODULE);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_STRING_CONCAT", Variant::OP_STRING_CONCAT);
	//bitwise
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_SHIFT_LEFT", Variant::OP_SHIFT_LEFT);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_SHIFT_RIGHT", Variant::OP_SHIFT_RIGHT);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_BIT_AND", Variant::OP_BIT_AND);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_BIT_OR", Variant::OP_BIT_OR);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_BIT_XOR", Variant::OP_BIT_XOR);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_BIT_NEGATE", Variant::OP_BIT_NEGATE);
	//logic
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_AND", Variant::OP_AND);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_OR", Variant::OP_OR);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_XOR", Variant::OP_XOR);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_NOT", Variant::OP_NOT);
	//containment
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_IN", Variant::OP_IN);
	BIND_GLOBAL_ENUM_CONSTANT_CUSTOM("OP_MAX", Variant::OP_MAX);
}

void unregister_global_constants() {

	_global_constants.clear();
}

int GlobalConstants::get_global_constant_count() {

	return _global_constants.size();
}

#ifdef DEBUG_METHODS_ENABLED
StringName GlobalConstants::get_global_constant_enum(int p_idx) {

	return _global_constants[p_idx].enum_name;
}
#else
StringName GlobalConstants::get_global_constant_enum(int p_idx) {

	return StringName();
}
#endif

const char *GlobalConstants::get_global_constant_name(int p_idx) {

	return _global_constants[p_idx].name;
}

int GlobalConstants::get_global_constant_value(int p_idx) {

	return _global_constants[p_idx].value;
}
