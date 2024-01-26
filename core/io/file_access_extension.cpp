/**************************************************************************/
/*  file_access_extension.cpp                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "file_access_extension.h"

#include "core/object/object.h"

Error FileAccessExtension::open_internal(const String &p_path, int p_mode_flags) {
	Error err = OK;
	GDVIRTUAL_CALL(_open_internal, p_path, p_mode_flags, err);
	return err;
}

uint64_t FileAccessExtension::_get_modified_time(const String &p_file) {
	uint64_t time = 0;
	GDVIRTUAL_CALL(__get_modified_time, p_file, time);
	return time;
}

bool FileAccessExtension::is_open() const {
	bool is_open = false;
	GDVIRTUAL_CALL(_is_open, is_open);
	return is_open;
}

String FileAccessExtension::get_path() const {
	String path = "";
	GDVIRTUAL_CALL(_get_path, path);
	return path;
}

String FileAccessExtension::get_path_absolute() const {
	String path_absolute = "";
	GDVIRTUAL_CALL(_get_path_absolute, path_absolute);
	return path_absolute;
}

void FileAccessExtension::seek(uint64_t p_position) {
	GDVIRTUAL_CALL(_seek, p_position);
}

void FileAccessExtension::seek_end(int64_t p_position) {
	GDVIRTUAL_CALL(_seek_end, p_position);
}

uint64_t FileAccessExtension::get_position() const {
	uint64_t position = 0;
	GDVIRTUAL_CALL(_get_position, position);
	return position;
}

uint64_t FileAccessExtension::get_length() const {
	uint64_t length = 0;
	GDVIRTUAL_CALL(_get_length, length);
	return length;
}

bool FileAccessExtension::eof_reached() const {
	bool eof_reached = false;
	GDVIRTUAL_CALL(_eof_reached, eof_reached);
	return eof_reached;
}

uint8_t FileAccessExtension::get_8() const {
	uint8_t val = 0;
	GDVIRTUAL_CALL(_get_8, val);
	return val;
}

uint16_t FileAccessExtension::get_16() const {
	uint16_t val = 0;
	GDVIRTUAL_CALL(_get_16, val);
	return val;
}

uint32_t FileAccessExtension::get_32() const {
	uint32_t val = 0;
	GDVIRTUAL_CALL(_get_32, val);
	return val;
}

uint64_t FileAccessExtension::get_64() const {
	uint64_t val = 0;
	GDVIRTUAL_CALL(_get_64, val);
	return val;
}

float FileAccessExtension::get_float() const {
	float val = 0;
	GDVIRTUAL_CALL(_get_float, val);
	return val;
}

double FileAccessExtension::get_double() const {
	double val = 0;
	GDVIRTUAL_CALL(_get_double, val);
	return val;
}

real_t FileAccessExtension::get_real() const {
	real_t val = 0;
	GDVIRTUAL_CALL(_get_real, val);
	return val;
}

Variant FileAccessExtension::get_var(bool p_allow_objects) const {
	Variant var;
	GDVIRTUAL_CALL(_get_var, p_allow_objects, var);
	return var;
}

uint64_t FileAccessExtension::get_buffer(uint8_t *p_dst, uint64_t p_length) const {
	ERR_FAIL_COND_V(!p_dst && p_length > 0, -1);

	Vector<uint8_t> buffer = get_buffer(p_length);

	int64_t i = 0;
	for (i = 0; i < buffer.size(); i++) {
		p_dst[i] = buffer.get(i);
	}

	return i;
}

Vector<uint8_t> FileAccessExtension::get_buffer(int64_t p_length) const {
	Vector<uint8_t> buffer;
	GDVIRTUAL_CALL(_get_buffer, p_length, buffer);
	return buffer;
}

String FileAccessExtension::get_line() const {
	String line;
	GDVIRTUAL_CALL(_get_line, line);
	return line;
}

String FileAccessExtension::get_token() const {
	String token;
	GDVIRTUAL_CALL(_get_token, token);
	return token;
}

Vector<String> FileAccessExtension::get_csv_line(const String &p_delim) const {
	Vector<String> csv_line;
	GDVIRTUAL_CALL(_get_csv_line, p_delim, csv_line);
	return csv_line;
}

String FileAccessExtension::get_as_text(bool p_skip_cr) const {
	String val_as_text;
	GDVIRTUAL_CALL(_get_as_text, p_skip_cr, val_as_text);
	return val_as_text;
}

String FileAccessExtension::get_as_utf8_string(bool p_skip_cr) const {
	String val_as_utf8_string;
	GDVIRTUAL_CALL(_get_as_utf8_string, p_skip_cr, val_as_utf8_string);
	return val_as_utf8_string;
}

void FileAccessExtension::set_big_endian(bool p_big_endian) {
	GDVIRTUAL_CALL(_set_big_endian, p_big_endian);
}

Error FileAccessExtension::get_error() const {
	Error err = OK;
	GDVIRTUAL_CALL(_get_error, err);
	return err;
}

void FileAccessExtension::flush() {
	GDVIRTUAL_CALL(_flush);
}

void FileAccessExtension::store_8(uint8_t p_dest) {
	GDVIRTUAL_CALL(_store_8, p_dest);
}

void FileAccessExtension::store_16(uint16_t p_dest) {
	GDVIRTUAL_CALL(_store_16, p_dest);
}

void FileAccessExtension::store_32(uint32_t p_dest) {
	GDVIRTUAL_CALL(_store_32, p_dest);
}

void FileAccessExtension::store_64(uint64_t p_dest) {
	GDVIRTUAL_CALL(_store_64, p_dest);
}

void FileAccessExtension::store_float(float p_dest) {
	GDVIRTUAL_CALL(_store_float, p_dest);
}

void FileAccessExtension::store_double(double p_dest) {
	GDVIRTUAL_CALL(_store_double, p_dest);
}

void FileAccessExtension::store_real(real_t p_dest) {
	GDVIRTUAL_CALL(_store_real, p_dest);
}

void FileAccessExtension::store_string(const String &p_string) {
	GDVIRTUAL_CALL(_store_string, p_string);
}

void FileAccessExtension::store_line(const String &p_line) {
	GDVIRTUAL_CALL(_store_line, p_line);
}

void FileAccessExtension::store_csv_line(const Vector<String> &p_values, const String &p_delim) {
	GDVIRTUAL_CALL(_store_csv_line, p_values, p_delim);
}

void FileAccessExtension::store_pascal_string(const String &p_string) {
	GDVIRTUAL_CALL(_store_pascal_string, p_string);
}

String FileAccessExtension::get_pascal_string() {
	String string;
	GDVIRTUAL_CALL(_get_pascal_string, string);
	return string;
}

void FileAccessExtension::store_buffer(const uint8_t *p_src, uint64_t p_length) {
	Vector<uint8_t> buffer;
	uint64_t i = 0;
	for (i = 0; i < p_length; i++) {
		buffer.insert(i, p_src[i]);
	}
	store_buffer(buffer);
}

void FileAccessExtension::store_buffer(const Vector<uint8_t> &p_buffer) {
	GDVIRTUAL_CALL(_store_buffer, p_buffer);
}

void FileAccessExtension::store_var(const Variant &p_var, bool p_full_objects) {
	GDVIRTUAL_CALL(_store_var, p_var, p_full_objects);
}

void FileAccessExtension::close() {
	GDVIRTUAL_CALL(_close);
}

bool FileAccessExtension::file_exists(const String &p_name) {
	bool file_exists;
	GDVIRTUAL_CALL(_file_exists, p_name, file_exists);
	return file_exists;
}

Error FileAccessExtension::reopen(const String &p_path, int p_mode_flags) {
	Error err = OK;
	GDVIRTUAL_CALL(_reopen, p_path, p_mode_flags, err);
	return err;
}

void FileAccessExtension::_bind_methods() {
	GDVIRTUAL_BIND(_open_internal, "path", "mode_flags");
	GDVIRTUAL_BIND(__get_modified_time, "file");

	GDVIRTUAL_BIND(_is_open);

	GDVIRTUAL_BIND(_get_path);
	GDVIRTUAL_BIND(_get_path_absolute);

	GDVIRTUAL_BIND(_seek, "position");
	GDVIRTUAL_BIND(_seek_end, "position");
	GDVIRTUAL_BIND(_get_position);
	GDVIRTUAL_BIND(_get_length);

	GDVIRTUAL_BIND(_eof_reached);

	GDVIRTUAL_BIND(_get_8);
	GDVIRTUAL_BIND(_get_16);
	GDVIRTUAL_BIND(_get_32);
	GDVIRTUAL_BIND(_get_64);

	GDVIRTUAL_BIND(_get_float);
	GDVIRTUAL_BIND(_get_double);
	GDVIRTUAL_BIND(_get_real);

	GDVIRTUAL_BIND(_get_var, "allow_objects");

	GDVIRTUAL_BIND(_get_buffer, "length");
	GDVIRTUAL_BIND(_get_line);
	GDVIRTUAL_BIND(_get_token);
	GDVIRTUAL_BIND(_get_csv_line, "delim");
	GDVIRTUAL_BIND(_get_as_text, "skip_cr");
	GDVIRTUAL_BIND(_get_as_utf8_string, "skip_cr");

	GDVIRTUAL_BIND(_set_big_endian, "big_endian");

	GDVIRTUAL_BIND(_get_error);

	GDVIRTUAL_BIND(_flush);
	GDVIRTUAL_BIND(_store_8, "dest");
	GDVIRTUAL_BIND(_store_16, "dest");
	GDVIRTUAL_BIND(_store_32, "dest");
	GDVIRTUAL_BIND(_store_64, "dest");

	GDVIRTUAL_BIND(_store_float, "dest");
	GDVIRTUAL_BIND(_store_double, "dest");
	GDVIRTUAL_BIND(_store_real, "dest");

	GDVIRTUAL_BIND(_store_string, "string");
	GDVIRTUAL_BIND(_store_line, "line");
	GDVIRTUAL_BIND(_store_csv_line, "values", "delim");

	GDVIRTUAL_BIND(_store_pascal_string, "string");
	GDVIRTUAL_BIND(_get_pascal_string);

	GDVIRTUAL_BIND(_store_buffer, "buffer");

	GDVIRTUAL_BIND(_store_var, "var", "full_objects");

	GDVIRTUAL_BIND(_close);

	GDVIRTUAL_BIND(_file_exists, "name");

	GDVIRTUAL_BIND(_reopen, "path", "mode_flags");
}

FileAccessExtension::FileAccessExtension() {
}

FileAccessExtension::~FileAccessExtension() {
}
