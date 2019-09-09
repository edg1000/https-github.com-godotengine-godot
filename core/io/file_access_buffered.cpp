/*************************************************************************/
/*  file_access_buffered.cpp                                             */
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
#include "file_access_buffered.h"

#include <string.h>

#include "error_macros.h"

Error FileAccessBuffered::set_error(Error p_error) const {

	return (last_error = p_error);
};

void FileAccessBuffered::set_cache_size(int p_size) {

	cache_size = p_size;
};

int FileAccessBuffered::get_cache_size() {

	return cache_size;
};

int FileAccessBuffered::cache_data_left() const {

	if (file.offset >= file.size) {
		return 0;
	};

	if (read_cache.offset == -1 || file.offset < read_cache.offset || file.offset >= read_cache.offset + read_cache.buffer.size()) {

		return read_data_block(file.offset, cache_size);

	} else {

		return read_cache.buffer.size() - (file.offset - read_cache.offset);
	};

	return 0;
};

void FileAccessBuffered::seek(size_t p_position) {

	_write_cache_commit();
	file.offset = p_position;
};

void FileAccessBuffered::seek_end(int64_t p_position) {

	_write_cache_commit();
	file.offset = file.size + p_position;
};

size_t FileAccessBuffered::get_pos() const {

	return file.offset;
};

size_t FileAccessBuffered::get_len() const {

	return file.size;
};

bool FileAccessBuffered::eof_reached() const {

	return file.offset > file.size;
};

uint8_t FileAccessBuffered::get_8() const {

	ERR_FAIL_COND_V(!file.open, 0);

	uint8_t byte = 0;
	if (cache_data_left() >= 1) {

		byte = read_cache.buffer[file.offset - read_cache.offset];
	};

	++file.offset;

	return byte;
};

int FileAccessBuffered::get_buffer(uint8_t *p_dest, int p_elements) const {

	ERR_FAIL_COND_V(!file.open, -1);

	if (p_elements > cache_size) {

		int total_read = 0;

		if (!(read_cache.offset == -1 || file.offset < read_cache.offset || file.offset >= read_cache.offset + read_cache.buffer.size())) {

			int size = (read_cache.buffer.size() - (file.offset - read_cache.offset));
			size = size - (size % 4);
			//DVector<uint8_t>::Read read = cache.buffer.read();
			//memcpy(p_dest, read.ptr() + (file.offset - cache.offset), size);
			memcpy(p_dest, read_cache.buffer.ptr() + (file.offset - read_cache.offset), size);
			p_dest += size;
			p_elements -= size;
			file.offset += size;
			total_read += size;
		};

		int err = read_data_block(file.offset, p_elements, p_dest);
		if (err >= 0) {
			total_read += err;
			file.offset += err;
		};

		return total_read;
	};

	int to_read = p_elements;
	int total_read = 0;
	while (to_read > 0) {

		int left = cache_data_left();
		if (left == 0) {
			if (to_read > 0) {
				file.offset += to_read;
			};
			return total_read;
		};
		if (left < 0) {
			return left;
		};

		int r = MIN(left, to_read);
		//DVector<uint8_t>::Read read = cache.buffer.read();
		//memcpy(p_dest+total_read, &read.ptr()[file.offset - cache.offset], r);
		memcpy(p_dest + total_read, read_cache.buffer.ptr() + (file.offset - read_cache.offset), r);

		file.offset += r;
		total_read += r;
		to_read -= r;
	};

	return p_elements;
};

void FileAccessBuffered::store_8(uint8_t p_dest) {

	store_buffer(&p_dest, 1);
};

void FileAccessBuffered::_write_cache_commit() {

	if (write_cache.buffer.size()) {
		write_data_block(write_cache.offset, write_cache.size, write_cache.buffer.ptr());

		write_cache.offset += write_cache.size;
		write_cache.size = 0;
	}
};

void FileAccessBuffered::store_buffer(const uint8_t *p_src, int p_length) {

	//_check_flush();
	if (write_cache.buffer.size() == 0) {
		write_cache.buffer.resize(cache_size);
		write_cache.size = 0;
	};

	int to_write = p_length;

	while (to_write > 0) {

		int space = cache_size - write_cache.size;
		if (space <= 0) {

			_write_cache_commit();
			continue;
		};

		int write = MIN(to_write, space);
		memcpy((void *)&write_cache.buffer[write_cache.size], (void *)&p_src[p_length - to_write], write);
		write_cache.size += write;

		to_write -= write;
	};

	if ((read_cache.offset < write_cache.offset && read_cache.offset + read_cache.size > write_cache.offset) || (read_cache.offset > write_cache.offset && write_cache.offset + write_cache.size > read_cache.offset)) {
		if (read_cache.buffer.size() == 0) {
			read_cache.buffer.resize(cache_size);
			read_cache.size = 0;
		};
		read_data_block(read_cache.offset, read_cache.size, read_cache.buffer.ptr());
	}

	file.offset += p_length;

	return;
};

bool FileAccessBuffered::is_open() const {

	return file.open;
};

Error FileAccessBuffered::get_error() const {

	return last_error;
};

FileAccessBuffered::FileAccessBuffered() {

	set_cache_size(DEFAULT_CACHE_SIZE);
};

FileAccessBuffered::~FileAccessBuffered() {
}
