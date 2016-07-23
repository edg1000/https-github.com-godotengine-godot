/*************************************************************************/
/*  power_x11.cpp                                                        */
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

#include "power_x11.h"

#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

// CODE CHUNK IMPORTED FROM SDL 2.0

static const char* proc_apm_path = "/proc/apm";
static const char* proc_acpi_battery_path = "/proc/acpi/battery";
static const char* proc_acpi_ac_adapter_path = "/proc/acpi/ac_adapter";
static const char* sys_class_power_supply_path = "/sys/class/power_supply";

FileAccessRef PowerX11::open_power_file(const char* base, const char* node, const char* key)
{
	String path = String(base) + String("/") + String(node) + String("/") + String(key);
	FileAccessRef f = FileAccess::open(path,FileAccess::READ);
	return f;
}


bool PowerX11::read_power_file(const char* base, const char* node, const char* key, char* buf, size_t buflen)
{
	ssize_t br = 0;
	FileAccessRef fd = open_power_file(base, node, key);
	if (!fd) {
		return false;
	}
	br = fd->get_buffer(reinterpret_cast<uint8_t*>(buf), buflen-1);
	fd->close();
	if (br < 0) {
		return false;
	}
	buf[br] = '\0'; // null-terminate the string
	return true;
}


bool PowerX11::make_proc_acpi_key_val(char **_ptr, char **_key, char **_val)
{
	char *ptr = *_ptr;

	while (*ptr == ' ') {
		ptr++;  /* skip whitespace. */
	}

	if (*ptr == '\0') {
		return false;  /* EOF. */
	}

	*_key = ptr;

	while ((*ptr != ':') && (*ptr != '\0')) {
		ptr++;
	}

	if (*ptr == '\0') {
		return false;  /* (unexpected) EOF. */
	}

	*(ptr++) = '\0';  /* terminate the key. */

	while ((*ptr == ' ') && (*ptr != '\0')) {
		ptr++;  /* skip whitespace. */
	}

	if (*ptr == '\0') {
		return false;  /* (unexpected) EOF. */
	}

	*_val = ptr;

	while ((*ptr != '\n') && (*ptr != '\0')) {
		ptr++;
	}

	if (*ptr != '\0') {
		*(ptr++) = '\0';  /* terminate the value. */
	}

	*_ptr = ptr;  /* store for next time. */
	return true;
}

void
PowerX11::check_proc_acpi_battery(const char * node, bool * have_battery, bool * charging)
{
	const char *base = proc_acpi_battery_path;
	char info[1024];
	char state[1024];
	char *ptr = NULL;
	char *key = NULL;
	char *val = NULL;
	bool charge = false;
	bool choose = false;
	int maximum = -1;
	int remaining = -1;
	int secs = -1;
	int pct = -1;

	if (!read_power_file(base, node, "state", state, sizeof (state))) {
		return;
	} else {
		if (!read_power_file(base, node, "info", info, sizeof (info)))
			return;
	}

	ptr = &state[0];
	while (make_proc_acpi_key_val(&ptr, &key, &val)) {
		if (String(key) == "present") {
			if (String(val) == "yes") {
				*have_battery = true;
			}
		} else if (String(key) == "charging state") {
			/* !!! FIXME: what exactly _does_ charging/discharging mean? */
			if (String(val) == "charging/discharging") {
				charge = true;
			} else if (String(val) == "charging") {
				charge = true;
			}
		} else if (String(key) == "remaining capacity") {
			char *endptr = NULL;
			//const int cvt = (int) strtol(val, &endptr, 10);
			String sval = val;
			const int cvt = sval.to_int();
			if (*endptr == ' ') {
				remaining = cvt;
			}
		}
	}

	ptr = &info[0];
	while (make_proc_acpi_key_val(&ptr, &key, &val)) {
		if (String(key) == "design capacity") {
			char *endptr = NULL;
			String sval = val;
			const int cvt = sval.to_int();
			if (*endptr == ' ') {
				maximum = cvt;
			}
		}
	}

	if ((maximum >= 0) && (remaining >= 0)) {
		pct = (int) ((((float) remaining) / ((float) maximum)) * 100.0f);
		if (pct < 0) {
			pct = 0;
		} else if (pct > 100) {
			pct = 100;
		}
	}

	/* !!! FIXME: calculate (secs). */

	/*
	 * We pick the battery that claims to have the most minutes left.
	 *  (failing a report of minutes, we'll take the highest percent.)
	 */
	if ((secs < 0) && (this->nsecs_left < 0)) {
		if ((pct < 0) && (this->percent_left < 0)) {
			choose = true;  /* at least we know there's a battery. */
		}
		if (pct > this->percent_left) {
			choose = true;
		}
	} else if (secs > this->nsecs_left) {
		choose = true;
	}

	if (choose) {
		this->nsecs_left = secs;
		this->percent_left = pct;
		*charging = charge;
	}
}

void PowerX11::check_proc_acpi_ac_adapter(const char * node, bool * have_ac)
{
	const char *base = proc_acpi_ac_adapter_path;
	char state[256];
	char *ptr = NULL;
	char *key = NULL;
	char *val = NULL;

	if (!read_power_file(base, node, "state", state, sizeof (state))) {
		return;
	}

	ptr = &state[0];
	while (make_proc_acpi_key_val(&ptr, &key, &val)) {
		String skey = key;
		if (skey == "state") {
			String sval = val;
			if (sval == "on-line") {
				*have_ac = true;
			}
		}
	}
}


bool PowerX11::GetPowerInfo_Linux_proc_acpi()
{
	String node;
	DirAccess *dirp = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	bool have_battery = false;
	bool have_ac = false;
	bool charging = false;

	this->nsecs_left = -1;
	this->percent_left = -1;
	this->power_state = POWERSTATE_UNKNOWN;


	dirp->change_dir(proc_acpi_battery_path);
	dirp->list_dir_begin();

	if (dirp == NULL) {
		return false;  /* can't use this interface. */
	} else {
		node = dirp->get_next();
		while (node != "") {
			check_proc_acpi_battery(node.utf8().get_data(), &have_battery, &charging/*, seconds, percent*/);
			node = dirp->get_next();
		}
		memdelete(dirp);
	}

	dirp->change_dir(proc_acpi_ac_adapter_path);
	dirp->list_dir_begin();
	if (dirp == NULL) {
		return false;  /* can't use this interface. */
	} else {
		node = dirp->get_next();
		while (node != "") {
			check_proc_acpi_ac_adapter(node.utf8().get_data(), &have_ac);
			node = dirp->get_next();
		}
		memdelete(dirp);
	}

	if (!have_battery) {
		this->power_state = POWERSTATE_NO_BATTERY;
	} else if (charging) {
		this->power_state = POWERSTATE_CHARGING;
	} else if (have_ac) {
		this->power_state = POWERSTATE_CHARGED;
	} else {
		this->power_state = POWERSTATE_ON_BATTERY;
	}

	return true;   /* definitive answer. */
}


bool PowerX11::next_string(char **_ptr, char **_str)
{
	char *ptr = *_ptr;
	char *str = *_str;

	while (*ptr == ' ') {       /* skip any spaces... */
		ptr++;
	}

	if (*ptr == '\0') {
		return false;
	}

	str = ptr;
	while ((*ptr != ' ') && (*ptr != '\n') && (*ptr != '\0'))
		ptr++;

	if (*ptr != '\0')
		*(ptr++) = '\0';

	*_str = str;
	*_ptr = ptr;
	return true;
}

bool PowerX11::int_string(char *str, int *val)
{
	String sval = str;
	*val = sval.to_int();
	return (*str != '\0');
}

/* http://lxr.linux.no/linux+v2.6.29/drivers/char/apm-emulation.c */
bool PowerX11::GetPowerInfo_Linux_proc_apm()
{
	bool need_details = false;
	int ac_status = 0;
	int battery_status = 0;
	int battery_flag = 0;
	int battery_percent = 0;
	int battery_time = 0;
	FileAccessRef fd = FileAccess::open(proc_apm_path,FileAccess::READ);
	char buf[128];
	char *ptr = &buf[0];
	char *str = NULL;
	ssize_t br;

	if (!fd) {
		return false;       /* can't use this interface. */
	}

	br = fd->get_buffer(reinterpret_cast<uint8_t*>(buf), sizeof (buf) - 1);
	fd->close();

	if (br < 0) {
		return false;
	}

	buf[br] = '\0';             /* null-terminate the string. */
	if (!next_string(&ptr, &str)) {     /* driver version */
		return false;
	}
	if (!next_string(&ptr, &str)) {     /* BIOS version */
		return false;
	}
	if (!next_string(&ptr, &str)) {     /* APM flags */
		return false;
	}

	if (!next_string(&ptr, &str)) {     /* AC line status */
		return false;
	} else if (!int_string(str, &ac_status)) {
		return false;
	}

	if (!next_string(&ptr, &str)) {     /* battery status */
		return false;
	} else if (!int_string(str, &battery_status)) {
		return false;
	}
	if (!next_string(&ptr, &str)) {     /* battery flag */
		return false;
	} else if (!int_string(str, &battery_flag)) {
		return false;
	}
	if (!next_string(&ptr, &str)) {     /* remaining battery life percent */
		return false;
	}
	String sstr = str;
	if (sstr[sstr.length() - 1] == '%') {
		sstr[sstr.length() - 1] = '\0';
	}
	if (!int_string(str, &battery_percent)) {
		return false;
	}

	if (!next_string(&ptr, &str)) {     /* remaining battery life time */
		return false;
	} else if (!int_string(str, &battery_time)) {
		return false;
	}

	if (!next_string(&ptr, &str)) {     /* remaining battery life time units */
		return false;
	} else if (String(str) == "min") {
		battery_time *= 60;
	}

	if (battery_flag == 0xFF) { /* unknown state */
		this->power_state = POWERSTATE_UNKNOWN;
	} else if (battery_flag & (1 << 7)) {       /* no battery */
		this->power_state = POWERSTATE_NO_BATTERY;
	} else if (battery_flag & (1 << 3)) {       /* charging */
		this->power_state = POWERSTATE_CHARGING;
		need_details = true;
	} else if (ac_status == 1) {
		this->power_state = POWERSTATE_CHARGED;        /* on AC, not charging. */
		need_details = true;
	} else {
		this->power_state = POWERSTATE_ON_BATTERY;
		need_details = true;
	}

	this->percent_left = -1;
	this->nsecs_left = -1;
	if (need_details) {
		const int pct = battery_percent;
		const int secs = battery_time;

		if (pct >= 0) {         /* -1 == unknown */
			this->percent_left = (pct > 100) ? 100 : pct; /* clamp between 0%, 100% */
		}
		if (secs >= 0) {        /* -1 == unknown */
			this->nsecs_left = secs;
		}
	}

	return true;
}

/* !!! FIXME: implement d-bus queries to org.freedesktop.UPower. */

bool PowerX11::GetPowerInfo_Linux_sys_class_power_supply(/*PowerState *state, int *seconds, int *percent*/)
{
	const char* base = sys_class_power_supply_path;
	String name;

	DirAccess *dirp = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	dirp->change_dir(base);
	dirp->list_dir_begin();

	if (!dirp) {
		return false;
	}

	this->power_state = POWERSTATE_NO_BATTERY;  /* assume we're just plugged in. */
	this->nsecs_left = -1;
	this->percent_left = -1;

	name = dirp->get_next();

	while (name != "") {
		bool choose = false;
		char str[64];
		PowerState st;
		int secs;
		int pct;

		if ((name == ".") || (name == "..")) {
			name = dirp->get_next();
			continue;  //skip these, of course.
		} else {
			if (!read_power_file(base, name.utf8().get_data(), "type", str, sizeof (str))) {
				name = dirp->get_next();
				continue;  // Don't know _what_ we're looking at. Give up on it.
			} else {
				if (String(str) != "Battery\n") {
					name = dirp->get_next();
					continue;  // we don't care about UPS and such.
				}
			}
		}

		/* some drivers don't offer this, so if it's not explicitly reported assume it's present. */
		if (read_power_file(base, name.utf8().get_data(), "present", str, sizeof (str)) && (String(str) == "0\n")) {
			st = POWERSTATE_NO_BATTERY;
		} else if (!read_power_file(base, name.utf8().get_data(), "status", str, sizeof (str))) {
			st = POWERSTATE_UNKNOWN;  /* uh oh */
		} else if (String(str) == "Charging\n") {
			st = POWERSTATE_CHARGING;
		} else if (String(str) == "Discharging\n") {
			st = POWERSTATE_ON_BATTERY;
		} else if ((String(str) == "Full\n") || (String(str) == "Not charging\n")) {
			st = POWERSTATE_CHARGED;
		} else {
			st = POWERSTATE_UNKNOWN;  /* uh oh */
		}

		if (!read_power_file(base, name.utf8().get_data(), "capacity", str, sizeof (str))) {
			pct = -1;
		} else {
			pct = String(str).to_int();
			pct = (pct > 100) ? 100 : pct; /* clamp between 0%, 100% */
		}

		if (!read_power_file(base, name.utf8().get_data(), "time_to_empty_now", str, sizeof (str))) {
			secs = -1;
		} else {
			secs = String(str).to_int();
			secs = (secs <= 0) ? -1 : secs;  /* 0 == unknown */
		}

		/*
		 * We pick the battery that claims to have the most minutes left.
		 *  (failing a report of minutes, we'll take the highest percent.)
		 */
		if ((secs < 0) && (this->nsecs_left < 0)) {
			if ((pct < 0) && (this->percent_left < 0)) {
				choose = true;  /* at least we know there's a battery. */
			} else if (pct > this->percent_left) {
				choose = true;
			}
		} else if (secs > this->nsecs_left) {
			choose = true;
		}

		if (choose) {
			this->nsecs_left = secs;
			this->percent_left = pct;
			this->power_state = st;
		}

		name = dirp->get_next();
	}

	memdelete(dirp);
	return true;  /* don't look any further*/
}



bool PowerX11::UpdatePowerInfo()
{
	if (GetPowerInfo_Linux_sys_class_power_supply()) {	// try method 1
		return true;
	}
	if (GetPowerInfo_Linux_proc_acpi()) {	// try further
		return true;
	}
	if (GetPowerInfo_Linux_proc_apm()) {	// try even further
		return true;
	}
	return false;
}

PowerX11::PowerX11() : nsecs_left(-1), percent_left(-1), power_state(POWERSTATE_UNKNOWN) {
}

PowerX11::~PowerX11() {
}

PowerState PowerX11::get_power_state() {
	if (UpdatePowerInfo()) {
		return power_state;
	}
	else {
		return POWERSTATE_UNKNOWN;
	}
}

int PowerX11::get_power_seconds_left() {
	if (UpdatePowerInfo()) {
		return nsecs_left;
	}
	else {
		return -1;
	}
}

int PowerX11::get_power_percent_left() {
	if (UpdatePowerInfo()) {
		return percent_left;
	}
	else {
		return -1;
	}
}