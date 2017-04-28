/*************************************************************************/
/*  power_osx.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef PLATFORM_OSX_POWER_OSX_H_
#define PLATFORM_OSX_POWER_OSX_H_

#include "dir_access_osx.h"
#include "os/file_access.h"
#include "os/power.h"
#include <CoreFoundation/CoreFoundation.h>

class power_osx {

private:
	int nsecs_left;
	int percent_left;
	PowerState power_state;
	void checkps(CFDictionaryRef dict, bool *have_ac, bool *have_battery, bool *charging);
	bool GetPowerInfo_MacOSX(/*PowerState * state, int *seconds, int *percent*/);
	bool UpdatePowerInfo();

public:
	power_osx();
	virtual ~power_osx();

	PowerState get_power_state();
	int get_power_seconds_left();
	int get_power_percent_left();
};

#endif /* PLATFORM_OSX_POWER_OSX_H_ */
