/*************************************************************************/
/*  file_access_buffered_fa.h                                            */
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
#ifndef FILE_ACCESS_BUFFERED_FA_H
#define FILE_ACCESS_BUFFERED_FA_H

#include "core/io/file_access_buffered.h"

template <class T>
class FileAccessBufferedFA : public FileAccessBuffered {

	T f;

	int read_data_block(int p_offset, int p_size, uint8_t *p_dest = 0) const {

		ERR_FAIL_COND_V(!f.is_open(), -1);

		((T *)&f)->seek(p_offset);

		if (p_dest) {

			f.get_buffer(p_dest, p_size);
			return p_size;

		} else {

			if (read_cache.buffer.size() == 0) {
                read_cache.buffer.resize(cache_size);
                read_cache.size = 0;
            };

            read_cache.offset = p_offset;

			// on dvector
			//DVector<uint8_t>::Write write = cache.buffer.write();
			//f.get_buffer(write.ptr(), p_size);

			// on vector
			f.get_buffer(read_cache.buffer.ptr(), p_size);

			return p_size;
		};
	};

	int write_data_block(int p_offset, int p_size, uint8_t *p_src = 0) {
        
        ERR_FAIL_COND_V(!f.is_open(), -1);

        ((T *)&f)->seek(p_offset);

        f.store_buffer(p_src, p_size);
        return p_size;
    };

	static FileAccess *create() {

		return memnew(FileAccessBufferedFA<T>());
	};

protected:
	virtual void _set_access_type(AccessType p_access) {
		f._set_access_type(p_access);
		FileAccessBuffered::_set_access_type(p_access);
	};

public:

	bool file_exists(const String &p_name) {

		return f.file_exists(p_name);
	};

	Error _open(const String &p_path, int p_mode_flags) {

		close();

		Error ret = f._open(p_path, p_mode_flags);
		if (ret != OK)
			return ret;
		//ERR_FAIL_COND_V( ret != OK, ret );

		file.size = f.get_len();
		file.offset = 0;
		file.open = true;
		file.name = p_path;
		file.access_flags = p_mode_flags;

		read_cache.buffer.resize(0);
		write_cache.buffer.resize(0);
		read_cache.offset = write_cache.offset = 0;

		return set_error(OK);
	};

	void close() {

		_write_cache_commit();

		f.close();

		file.offset = 0;
		file.size = 0;
		file.open = false;
		file.name = "";

		read_cache.buffer.resize(0);
		write_cache.buffer.resize(0);
		read_cache.offset = write_cache.offset = 0;
		set_error(OK);
	};

	//	static void make_default() {

	//FileAccess::create_func = FileAccessBufferedFA<T>::create;
	//	};

	virtual uint64_t _get_modified_time(const String &p_file) {

		return f._get_modified_time(p_file);
	}

	FileAccessBufferedFA(){

	};

	~FileAccessBufferedFA() {
		close();
	};
};

#endif // FILE_ACCESS_BUFFERED_FA_H
