/*************************************************************************/
/*  label.cpp                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
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
#include "label.h"
#include "print_string.h"
#include "globals.h"
#include "translation.h"


void Label::set_autowrap(bool p_autowrap) {

	autowrap=p_autowrap;
	word_cache_dirty=true;
	minimum_size_changed();
	update();

}
bool Label::has_autowrap() const {

	return autowrap;
}


void Label::set_uppercase(bool p_uppercase) {

	uppercase=p_uppercase;
	word_cache_dirty=true;
	minimum_size_changed();
	update();
}
bool Label::is_uppercase() const {

	return uppercase;
}


int Label::get_line_height() const {

	return get_font("font")->get_height();

}


void Label::_notification(int p_what) {

	if (p_what==NOTIFICATION_DRAW) {

		if (clip || autowrap)
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(),true);

		if (word_cache_dirty)
			regenerate_word_cache();


		RID ci = get_canvas_item();

		Size2 string_size;
		Size2 size=get_size();

		Ref<Font> font = get_font("font");
		Color font_color = get_color("font_color");
		Color font_color_shadow = get_color("font_color_shadow");
		bool use_outlinde = get_constant("shadow_as_outline");
		Point2 shadow_ofs(get_constant("shadow_offset_x"),get_constant("shadow_offset_y"));

		VisualServer::get_singleton()->canvas_item_set_distance_field_mode(get_canvas_item(),font.is_valid() && font->is_distance_field_hint());

		int font_h = font->get_height();
		int lines_visible = size.y/font_h;
		int space_w=font->get_char_size(' ').width;
		int chars_total=0;
		bool text_is_rtl = text.is_rtl();

		int vbegin=0,vsep=0;

		if (lines_visible > line_count) {
			lines_visible = line_count;

		}

		if (max_lines_visible >= 0 && lines_visible > max_lines_visible) {
			lines_visible = max_lines_visible;

		}

		if (lines_visible > 0) {

			switch(valign) {

				case VALIGN_TOP: {
					//nothing
				} break;
				case VALIGN_CENTER: {
					vbegin=(size.y - lines_visible * font_h) / 2;
					vsep=0;

				} break;
				case VALIGN_BOTTOM: {
					vbegin=size.y - lines_visible * font_h;
					vsep=0;

				} break;
				case VALIGN_FILL: {
					vbegin=0;
					if (lines_visible>1) {
						vsep=(size.y - lines_visible * font_h) / (lines_visible - 1);
					} else {
						vsep=0;
					}

				} break;
			}
		}


		WordCache *wc = word_cache;
		if (!wc)
			return;

		int c = 0;
		int line=0;
		int line_to=lines_skipped + (lines_visible>0?lines_visible:1);
		while(wc) {
		/* handle lines not meant to be drawn quickly */
			if  (line>=line_to)
				break;
			if  (line<lines_skipped) {

				while (wc && wc->char_pos>=0)
					wc=wc->next;
				if (wc)
					wc=wc->next;
				line++;
				continue;
			}

		/* handle lines normally */

			if (wc->char_pos<0) {
			//empty line
				wc=wc->next;
				line++;
				continue;
			}

			WordCache *from=wc;
			WordCache *to=wc;

			int taken=0;
			int spaces=0;
			while(to && to->char_pos>=0) {

				taken+=to->pixel_width;
				if (to!=from && to->space_count) {
					spaces+=to->space_count;
				}
				to=to->next;
			}

			bool can_fill = to && to->char_pos==WordCache::CHAR_WRAPLINE;

			float x_ofs=0;

			switch (align) {

				case ALIGN_FILL: {

					if (!text_is_rtl) {
						x_ofs=0;
					} else {
						x_ofs=int(size.width);
					}

				} break;
				case ALIGN_LEFT: {

					if (!text_is_rtl) {
						x_ofs=0;
					} else {
						x_ofs=int(taken+spaces*space_w);
					}

				} break;
				case ALIGN_CENTER: {

					if (!text_is_rtl) {
						x_ofs=int(size.width-(taken+spaces*space_w))/2;;
					} else {
						x_ofs=int(size.width+(taken+spaces*space_w))/2;;
					}

				} break;
				case ALIGN_RIGHT: {

					if (!text_is_rtl) {
						x_ofs=int(size.width-(taken+spaces*space_w));
					} else {
						x_ofs=int(size.width);
					}

				} break;
				}

			int y_ofs=(line-lines_skipped)*font_h + font->get_ascent();
			y_ofs+=vbegin + line*vsep;

			while(from!=to) {

			// draw a word
				int pos = from->char_pos;
				if (from->char_pos<0) {

					ERR_PRINT("BUG");
					return;
				}
				if (from->space_count) {
				/* spacing */
					if (!text_is_rtl) {
						x_ofs+=space_w*from->space_count;
					} else {
						x_ofs-=space_w*from->space_count;
					}

					if (can_fill && align==ALIGN_FILL && spaces && !text_is_rtl) {

						x_ofs+=int((size.width-(taken+space_w*spaces))/spaces);
					}

				}


				if (!text_is_rtl) {

					if (font_color_shadow.a>0) {

						int chars_total_shadow = chars_total; //save chars drawn
						float x_ofs_shadow=x_ofs;
						for (int i=0;i<from->word_len;i++) {

							if (visible_chars < 0 || chars_total_shadow<visible_chars) {
								CharType c = text[i+pos];
								CharType n = text[i+pos+1];
								if (uppercase) {
									c=String::char_uppercase(c);
									n=String::char_uppercase(c);
								}

								float move=font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+shadow_ofs, c, n,font_color_shadow );
								if (use_outlinde) {
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(-shadow_ofs.x,shadow_ofs.y), c, n,font_color_shadow );
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(shadow_ofs.x,-shadow_ofs.y), c, n,font_color_shadow );
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(-shadow_ofs.x,-shadow_ofs.y), c, n,font_color_shadow );
								}
								x_ofs_shadow+=move;
								chars_total_shadow++;
							}
						}


					}
					for (int i=0;i<from->word_len;i++) {

						if (visible_chars < 0 || chars_total<visible_chars) {
							CharType c = text[i+pos];
							CharType n = text[i+pos+1];
							if (uppercase) {
								c=String::char_uppercase(c);
								n=String::char_uppercase(c);
							}

							x_ofs+=font->draw_char(ci,Point2( x_ofs, y_ofs ), c, n, font_color );
							chars_total++;
						}

					}
				} else { //RTL text

					String bidi_text = text.bidi_visual_string();
					x_ofs-=from->pixel_width;
					if (font_color_shadow.a>0) {

						int chars_total_shadow = chars_total; //save chars drawn
						float x_ofs_shadow=x_ofs;

						for (int i=from->word_len-1; i>-1;i--) {

							if (visible_chars < 0 || chars_total_shadow<visible_chars) {
								CharType c = bidi_text[pos-i];
								CharType n = bidi_text[pos-i+1];
								if (uppercase) {
									c=String::char_uppercase(c);
									n=String::char_uppercase(c);
								}

								float move=font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+shadow_ofs, c, n,font_color_shadow );
								if (use_outlinde) {
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(-shadow_ofs.x,shadow_ofs.y), c, n,font_color_shadow );
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(shadow_ofs.x,-shadow_ofs.y), c, n,font_color_shadow );
									font->draw_char(ci, Point2( x_ofs_shadow, y_ofs )+Vector2(-shadow_ofs.x,-shadow_ofs.y), c, n,font_color_shadow );
								}
								x_ofs_shadow+=move;
								chars_total_shadow++;
							}
						}


					}

					for (int i=from->word_len-1; i>-1;i--) {

						if (visible_chars < 0 || chars_total<visible_chars) {
							CharType c = bidi_text[pos-i];
							CharType n = bidi_text[pos-i+1];
							if (uppercase) {
								c=String::char_uppercase(c);
								n=String::char_uppercase(c);
							}

							x_ofs+=font->draw_char(ci,Point2( x_ofs, y_ofs ), c, n, font_color );
							chars_total++;
						}

					}

					x_ofs -= from->pixel_width;
					if (can_fill && align==ALIGN_FILL && spaces) {

						x_ofs-=int((size.width-(taken+space_w*spaces))/spaces);
					}
				}

				from=from->next;
			}

			wc=to?to->next:0;
			line++;

		}
	}

	if (p_what==NOTIFICATION_THEME_CHANGED) {

		word_cache_dirty=true;
		update();
	}
	if (p_what==NOTIFICATION_RESIZED) {

		word_cache_dirty=true;
	}

}

Size2 Label::get_minimum_size() const {

	if (autowrap)
		return Size2(1,1);
	else {

		// don't want to mutable everything
		if(word_cache_dirty)
			const_cast<Label*>(this)->regenerate_word_cache();

		Size2 ms=minsize;
		if (clip)
			ms.width=1;
		return ms;
	}
}

int Label::get_longest_line_width() const {

	Ref<Font> font = get_font("font");
	int max_line_width=0;
	int line_width=0;

	String bidi_text = text.bidi_visual_string();
	for (int i=0;i<bidi_text.size()+1;i++) {

		CharType current=i<bidi_text.length()?bidi_text[i]:' '; //always a space at the end, so the algo works
		if (uppercase)
			current=String::char_uppercase(current);

		if (current<32) {

			if (current=='\n') {

				if (line_width>max_line_width)
					max_line_width=line_width;
				line_width=0;
			}
		} else {

			int char_width=font->get_char_size(current).width;
			line_width+=char_width;
		}

	}

	if (line_width>max_line_width)
		max_line_width=line_width;

	return max_line_width;
}

int Label::get_line_count() const {

	if (!is_inside_tree())
		return 1;
	if (word_cache_dirty)
		const_cast<Label*>(this)->regenerate_word_cache();

	return line_count;
}

void Label::regenerate_word_cache() {

	while (word_cache) {

		WordCache *current=word_cache;
		word_cache=current->next;
		memdelete( current );
	}


	int width=autowrap?get_size().width:get_longest_line_width();
	Ref<Font> font = get_font("font");

	int current_word_size=0;
	int word_pos=0;
	int line_width=0;
	int space_count=0;
	int space_width=font->get_char_size(' ').width;
	line_count=1;
	total_char_cache=0;

	WordCache *last=NULL;

	int i;
	bool next_char_available;
	bool text_is_rtl = text.is_rtl();
	CharType current;

	String bidi_text = text.bidi_visual_string();

	if (!text_is_rtl) {

		i=0;
		next_char_available= i<=bidi_text.size()+1;
		current=i<bidi_text.length()?bidi_text[i]:' '; //always a space at the end, so the algo works
	} else {

		i=bidi_text.size()-1;
		next_char_available= i>-1;
		current= next_char_available?bidi_text[i]:' ';
	}

	while (next_char_available) {

		if (uppercase)
			current=String::char_uppercase(current);

		// ranges taken from http://www.unicodemap.org/
		// if your language is not well supported, consider helping improve
		// the unicode support in Godot.
		bool separatable = (current>=0x2E08 && current<=0xFAFF) || (current>=0xFE30 && current<=0xFE4F);
				//current>=33 && (current < 65||current >90) && (current<97||current>122) && (current<48||current>57);
		bool insert_newline=false;
		int char_width;

		if (current<33) {

			if (current_word_size>0) {
				WordCache *wc = memnew( WordCache );
				if (word_cache) {
					last->next=wc;
				} else {
					word_cache=wc;
				}
				last=wc;

				wc->pixel_width=current_word_size;
				wc->char_pos=word_pos;
				if (!text_is_rtl) {
					wc->word_len=i-word_pos;
				} else {
					wc->word_len=word_pos-i;
				}

				wc->space_count = space_count;
				current_word_size=0;
				space_count=0;

			}


			if (current=='\n') {
				insert_newline=true;
			} else {
				total_char_cache++;
			}

			if ( (!text_is_rtl && i<bidi_text.length() && bidi_text[i] == ' ')
				|| (text_is_rtl && i>-1 && bidi_text[i] == ' ') ) {
				total_char_cache--;  // do not count spaces
				if (line_width > 0 || last==NULL || last->char_pos!=WordCache::CHAR_WRAPLINE) {
					space_count++;
					line_width+=space_width;
				}else {
					space_count=0;
				}
			}


		} else {
			// latin characters
			if (current_word_size==0) {
				word_pos=i;
			}

			char_width=font->get_char_size(current).width;
			current_word_size+=char_width;
			line_width+=char_width;
			total_char_cache++;

		}

		if ((autowrap && (line_width >= width) && ((last && last->char_pos >= 0) || separatable)) || insert_newline) {
			if (separatable) {
				if (current_word_size>0) {
					WordCache *wc = memnew( WordCache );
					if (word_cache) {
						last->next=wc;
					} else {
						word_cache=wc;
					}
					last=wc;

					wc->pixel_width=current_word_size-char_width;
					wc->char_pos=word_pos;
					wc->word_len=i-word_pos;
					wc->space_count = space_count;
					current_word_size=char_width;
					space_count=0;
					word_pos=i;
				}
			}

			WordCache *wc = memnew( WordCache );
			if (word_cache) {
				last->next=wc;
			} else {
				word_cache=wc;
			}
			last=wc;

			wc->pixel_width=0;
			wc->char_pos=insert_newline?WordCache::CHAR_NEWLINE:WordCache::CHAR_WRAPLINE;

			line_width=current_word_size;
			line_count++;
			space_count=0;

		}

		if (!text_is_rtl) {

			i++;
			next_char_available= i<bidi_text.size()+1;
			current=i<bidi_text.length()?bidi_text[i]:' '; //always a space at the end, so the algo works
		} else {

			i--;
			next_char_available= i>=-1;
			current= i>-1?bidi_text[i]:' ';
		}

	}

	if (!autowrap) {
		minsize.width=width;
		if (max_lines_visible > 0 && line_count > max_lines_visible) {
			minsize.height=font->get_height()*max_lines_visible;
		} else {
			minsize.height=font->get_height()*line_count;
		}
	}

	word_cache_dirty=false;

}

void Label::set_align(Align p_align) {

	ERR_FAIL_INDEX(p_align,4);
	align=p_align;
	update();
}

Label::Align Label::get_align() const{

	return align;
}

void Label::set_valign(VAlign p_align) {

	ERR_FAIL_INDEX(p_align,4);
	valign=p_align;
	update();
}

Label::VAlign Label::get_valign() const{

	return valign;
}

void Label::set_text(const String& p_string) {

	String str = XL_MESSAGE(p_string);

	if (text==str)
		return;

	text=str;
	word_cache_dirty=true;
	if (percent_visible<1)
		visible_chars=get_total_character_count()*percent_visible;
	update();
	minimum_size_changed();

}

void Label::set_clip_text(bool p_clip) {

	clip=p_clip;
	update();
	minimum_size_changed();
}

bool Label::is_clipping_text() const {

	return clip;
}

String Label::get_text() const {

	return text;
}

void Label::set_visible_characters(int p_amount) {

	visible_chars=p_amount;
	if (get_total_character_count() > 0) {
		percent_visible=(float)p_amount/(float)total_char_cache;
	}
	update();
}

int Label::get_visible_characters() const {

	return visible_chars;
}

void Label::set_percent_visible(float p_percent) {

	if (p_percent<0 || p_percent>=1) {

		visible_chars=-1;
		percent_visible=1;

	} else {

		visible_chars=get_total_character_count()*p_percent;
		percent_visible=p_percent;

	}
	update();

}

float Label::get_percent_visible() const{

	return percent_visible;
}

void Label::set_lines_skipped(int p_lines) {

	lines_skipped=p_lines;
	update();
}

int Label::get_lines_skipped() const{

	return lines_skipped;
}

void Label::set_max_lines_visible(int p_lines) {

	max_lines_visible=p_lines;
	update();
}

int Label::get_max_lines_visible() const{

	return max_lines_visible;
}

int Label::get_total_character_count() const {

	if (word_cache_dirty)
		const_cast<Label*>(this)->regenerate_word_cache();

	return total_char_cache;
}

void Label::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_align","align"),&Label::set_align);
	ObjectTypeDB::bind_method(_MD("get_align"),&Label::get_align);
	ObjectTypeDB::bind_method(_MD("set_valign","valign"),&Label::set_valign);
	ObjectTypeDB::bind_method(_MD("get_valign"),&Label::get_valign);
	ObjectTypeDB::bind_method(_MD("set_text","text"),&Label::set_text);
	ObjectTypeDB::bind_method(_MD("get_text"),&Label::get_text);
	ObjectTypeDB::bind_method(_MD("set_autowrap","enable"),&Label::set_autowrap);
	ObjectTypeDB::bind_method(_MD("has_autowrap"),&Label::has_autowrap);
	ObjectTypeDB::bind_method(_MD("set_clip_text","enable"),&Label::set_clip_text);
	ObjectTypeDB::bind_method(_MD("is_clipping_text"),&Label::is_clipping_text);
	ObjectTypeDB::bind_method(_MD("set_uppercase","enable"),&Label::set_uppercase);
	ObjectTypeDB::bind_method(_MD("is_uppercase"),&Label::is_uppercase);
	ObjectTypeDB::bind_method(_MD("get_line_height"),&Label::get_line_height);
	ObjectTypeDB::bind_method(_MD("get_line_count"),&Label::get_line_count);
	ObjectTypeDB::bind_method(_MD("get_total_character_count"),&Label::get_total_character_count);
	ObjectTypeDB::bind_method(_MD("set_visible_characters","amount"),&Label::set_visible_characters);
	ObjectTypeDB::bind_method(_MD("get_visible_characters"),&Label::get_visible_characters);
	ObjectTypeDB::bind_method(_MD("set_percent_visible","percent_visible"),&Label::set_percent_visible);
	ObjectTypeDB::bind_method(_MD("get_percent_visible"),&Label::get_percent_visible);
	ObjectTypeDB::bind_method(_MD("set_lines_skipped","lines_skipped"),&Label::set_lines_skipped);
	ObjectTypeDB::bind_method(_MD("get_lines_skipped"),&Label::get_lines_skipped);
	ObjectTypeDB::bind_method(_MD("set_max_lines_visible","lines_visible"),&Label::set_max_lines_visible);
	ObjectTypeDB::bind_method(_MD("get_max_lines_visible"),&Label::get_max_lines_visible);

	BIND_CONSTANT( ALIGN_LEFT );
	BIND_CONSTANT( ALIGN_CENTER );
	BIND_CONSTANT( ALIGN_RIGHT );
	BIND_CONSTANT( ALIGN_FILL );

	BIND_CONSTANT( VALIGN_TOP );
	BIND_CONSTANT( VALIGN_CENTER );
	BIND_CONSTANT( VALIGN_BOTTOM );
	BIND_CONSTANT( VALIGN_FILL );

	ADD_PROPERTYNZ( PropertyInfo( Variant::STRING, "text",PROPERTY_HINT_MULTILINE_TEXT,"",PROPERTY_USAGE_DEFAULT_INTL), _SCS("set_text"),_SCS("get_text") );
	ADD_PROPERTYNZ( PropertyInfo( Variant::INT, "align", PROPERTY_HINT_ENUM,"Left,Center,Right,Fill" ),_SCS("set_align"),_SCS("get_align") );
	ADD_PROPERTYNZ( PropertyInfo( Variant::INT, "valign", PROPERTY_HINT_ENUM,"Top,Center,Bottom,Fill" ),_SCS("set_valign"),_SCS("get_valign") );
	ADD_PROPERTYNZ( PropertyInfo( Variant::BOOL, "autowrap"),_SCS("set_autowrap"),_SCS("has_autowrap") );
	ADD_PROPERTYNZ( PropertyInfo( Variant::BOOL, "clip_text"),_SCS("set_clip_text"),_SCS("is_clipping_text") );
	ADD_PROPERTYNZ( PropertyInfo( Variant::BOOL, "uppercase"),_SCS("set_uppercase"),_SCS("is_uppercase") );
	ADD_PROPERTY( PropertyInfo( Variant::REAL, "percent_visible", PROPERTY_HINT_RANGE,"0,1,0.001"),_SCS("set_percent_visible"),_SCS("get_percent_visible") );
	ADD_PROPERTY( PropertyInfo( Variant::INT, "lines_skipped", PROPERTY_HINT_RANGE,"0,999,1"),_SCS("set_lines_skipped"),_SCS("get_lines_skipped") );
	ADD_PROPERTY( PropertyInfo( Variant::INT, "max_lines_visible", PROPERTY_HINT_RANGE,"-1,999,1"),_SCS("set_max_lines_visible"),_SCS("get_max_lines_visible") );

}

Label::Label(const String &p_text) {

	align=ALIGN_LEFT;
	valign=VALIGN_TOP;
	text="";
	word_cache=NULL;
	word_cache_dirty=true;
	autowrap=false;
	line_count=0;
	set_v_size_flags(0);
	clip=false;
	set_ignore_mouse(true);
	total_char_cache=0;
	visible_chars=-1;
	percent_visible=1;
	lines_skipped=0;
	max_lines_visible=-1;
	set_text(p_text);
	uppercase=false;
}


Label::~Label() {

	while (word_cache) {

		WordCache *current=word_cache;
		word_cache=current->next;
		memdelete( current );
	}
}


