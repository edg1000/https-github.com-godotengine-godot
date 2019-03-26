/*************************************************************************/
/*  dir_access_unix.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef DIR_ACCESS_UNIX_H
#define DIR_ACCESS_UNIX_H

#if defined(UNIX_ENABLED) || defined(LIBC_FILEIO_ENABLED)

#include "core/os/dir_access.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

class DirAccessUnix : public DirAccess {

	DIR *dir_stream;

	static DirAccess *create_fs();

	String current_dir;
	bool _cisdir;
	bool _cishidden;

protected:
	virtual String fix_unicode_name(const char *p_name) const { return String::utf8(p_name); }

public:
	virtual Error list_dir_begin(); ///< This starts dir listing
	virtual String get_next();
	virtual bool current_is_dir() const;
	virtual bool current_is_hidden() const;

	virtual void list_dir_end(); ///<

	virtual int get_drive_count();
	virtual String get_drive(int p_drive);

	virtual Error change_dir(String p_dir); ///< can be relative or absolute, return false on success
	virtual String get_current_dir(); ///< return current dir location
	virtual Error make_dir(String p_dir);

	virtual bool file_exists(String p_file);
	virtual bool dir_exists(String p_dir);

	virtual uint64_t get_modified_time(String p_file);

	virtual Error rename(String p_path, String p_new_path);
	virtual Error remove(String p_path);

	virtual int64_t get_space_left();

	virtual String get_filesystem_type() const;

	DirAccessUnix();
	~DirAccessUnix();
};

#endif //UNIX ENABLED
#endif
