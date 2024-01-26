/**************************************************************************/
/*  file_access_extension.h                                               */
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

#ifndef FILE_ACCESS_EXTENSION_H
#define FILE_ACCESS_EXTENSION_H

#include "core/io/file_access.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"

class FileAccessExtension : public FileAccess {
	GDCLASS(FileAccessExtension, FileAccess);

protected:
	static void _bind_methods();

public:
	GDVIRTUAL0RC(bool, _is_open);
	virtual bool is_open() const override;

	GDVIRTUAL0RC(String, _get_path);
	virtual String get_path() const override;
	GDVIRTUAL0RC(String, _get_path_absolute);
	virtual String get_path_absolute() const override;

	GDVIRTUAL1(_seek, uint64_t);
	virtual void seek(uint64_t p_position) override;
	GDVIRTUAL1(_seek_end, int64_t);
	virtual void seek_end(int64_t p_position) override;
	GDVIRTUAL0RC(uint64_t, _get_position);
	virtual uint64_t get_position() const override;
	GDVIRTUAL0RC(uint64_t, _get_length);
	virtual uint64_t get_length() const override;

	GDVIRTUAL0RC(bool, _eof_reached);
	virtual bool eof_reached() const override;

	GDVIRTUAL0RC(uint8_t, _get_8);
	virtual uint8_t get_8() const override;
	GDVIRTUAL0RC(uint16_t, _get_16);
	virtual uint16_t get_16() const override;
	GDVIRTUAL0RC(uint32_t, _get_32);
	virtual uint32_t get_32() const override;
	GDVIRTUAL0RC(uint64_t, _get_64);
	virtual uint64_t get_64() const override;

	GDVIRTUAL0RC(float, _get_float);
	virtual float get_float() const override;
	GDVIRTUAL0RC(double, _get_double);
	virtual double get_double() const override;
	GDVIRTUAL0RC(real_t, _get_real);
	virtual real_t get_real() const override;

	GDVIRTUAL1RC(Variant, _get_var, bool);
	Variant get_var(bool p_allow_objects = false) const;

	virtual uint64_t get_buffer(uint8_t *p_dst, uint64_t p_length) const override;
	GDVIRTUAL1RC(Vector<uint8_t>, _get_buffer, int64_t);
	Vector<uint8_t> get_buffer(int64_t p_length) const;
	GDVIRTUAL0RC(String, _get_line);
	virtual String get_line() const override;
	GDVIRTUAL0RC(String, _get_token);
	virtual String get_token() const override;
	GDVIRTUAL1RC(Vector<String>, _get_csv_line, String);
	virtual Vector<String> get_csv_line(const String &p_delim = ",") const override;
	GDVIRTUAL1RC(String, _get_as_text, bool);
	String get_as_text(bool p_skip_cr = false) const;
	GDVIRTUAL1RC(String, _get_as_utf8_string, bool);
	virtual String get_as_utf8_string(bool p_skip_cr = false) const override;

	FileAccessExtension();
	~FileAccessExtension();
};

#endif // FILE_ACCESS_EXTENSION_H
