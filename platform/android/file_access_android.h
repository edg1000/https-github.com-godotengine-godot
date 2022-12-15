/*************************************************************************/
/*  file_access_android.h                                                */
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

#ifndef FILE_ACCESS_ANDROID_H
#define FILE_ACCESS_ANDROID_H

#include "core/os/file_access.h"
#include <android/asset_manager.h>
#include <android/log.h>
#include <stdio.h>

class FileAccessAndroid : public FileAccess {
	mutable AAsset *asset = nullptr;
	mutable uint64_t len = 0;
	mutable uint64_t pos = 0;
	mutable bool eof = false;
	String absolute_path;
	String path_src;

protected:
	virtual Error _open(const String &p_path, int p_mode_flags); // open a file
	virtual void _close(); // close a file
	virtual void _seek(uint64_t p_position); // seek to a given position
	virtual void _seek_end(int64_t p_position = 0); // seek from the end of file
	virtual uint64_t _get_position() const; // get position in the file
	virtual bool _eof_reached() const; // reading passed EOF
	virtual uint8_t _get_8() const; // get a byte
	virtual uint64_t _get_buffer(uint8_t *p_dst, uint64_t p_length) const;

public:
	static AAssetManager *asset_manager;

	virtual bool is_open() const; // true when file is open

	/// returns the path for the current open file
	virtual String get_path() const;

	/// returns the absolute path for the current open file
	virtual String get_path_absolute() const;

	virtual uint64_t get_len() const; // get size of the file
	virtual Error get_error() const; // get last error

	virtual void flush();
	virtual void store_8(uint8_t p_dest); // store a byte

	virtual bool file_exists(const String &p_path); // return true if a file exists

	virtual uint64_t _get_modified_time(const String &p_file) { return 0; }
	virtual uint32_t _get_unix_permissions(const String &p_file) { return 0; }
	virtual Error _set_unix_permissions(const String &p_file, uint32_t p_permissions) { return FAILED; }

	~FileAccessAndroid();
};

#endif // FILE_ACCESS_ANDROID_H
