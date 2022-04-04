/*************************************************************************/
/*  tvos.mm                                                              */
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

#include "tvos.h"

#import "app_delegate.h"
#import "godot_view_controller.h"
#include "os_tvos.h"

#import <UIKit/UIKit.h>
#include <sys/sysctl.h>

void tvOS::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rate_url", "app_id"), &tvOS::get_rate_url);

	ClassDB::bind_method(D_METHOD("get_overrides_menu_button"), &tvOS::get_overrides_menu_button);
	ClassDB::bind_method(D_METHOD("set_overrides_menu_button", "p_flag"), &tvOS::set_overrides_menu_button);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "overrides_menu_button"), "set_overrides_menu_button", "get_overrides_menu_button");
};

void tvOS::alert(const char *p_alert, const char *p_title) {
	NSString *title = [NSString stringWithUTF8String:p_title];
	NSString *message = [NSString stringWithUTF8String:p_alert];

	UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
	UIAlertAction *button = [UIAlertAction actionWithTitle:@"OK"
													 style:UIAlertActionStyleCancel
												   handler:^(id){
												   }];

	[alert addAction:button];

	[AppDelegate.viewController presentViewController:alert animated:YES completion:nil];
}

String tvOS::get_model() const {
	size_t size;
	sysctlbyname("hw.machine", NULL, &size, NULL, 0);
	char *model = (char *)malloc(size);
	if (model == NULL) {
		return "";
	}
	sysctlbyname("hw.machine", model, &size, NULL, 0);
	NSString *platform = [NSString stringWithCString:model encoding:NSUTF8StringEncoding];
	free(model);
	const char *str = [platform UTF8String];
	return String(str != NULL ? str : "");
}

String tvOS::get_rate_url(int p_app_id) const {
	String app_url_path = "itms-apps://itunes.apple.com/app/idAPP_ID";

	String ret = app_url_path.replace("APP_ID", String::num(p_app_id));

	printf("returning rate url %s\n", ret.utf8().get_data());

	return ret;
};

bool tvOS::get_overrides_menu_button() const {
	if (!OSAppleTV::get_singleton()) {
		return false;
	}

	return OSAppleTV::get_singleton()->get_overrides_menu_button();
}

void tvOS::set_overrides_menu_button(bool p_flag) {
	if (!OSAppleTV::get_singleton()) {
		return;
	}

	OSAppleTV::get_singleton()->set_overrides_menu_button(p_flag);
}

tvOS::tvOS(){};
