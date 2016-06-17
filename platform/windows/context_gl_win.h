/*************************************************************************/
/*  context_gl_win.h                                                     */
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
#if defined(OPENGL_ENABLED) || defined(LEGACYGL_ENABLED) || defined(GLES2_ENABLED)
//
// C++ Interface: context_gl_x11
//
// Description:
//
//
// Author: Juan Linietsky <reduzio@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CONTEXT_GL_WIN_H
#define CONTEXT_GL_WIN_H


#include "os/os.h"
#include "drivers/gl_context/context_gl.h"
#include "error_list.h"

#include <windows.h>

typedef bool (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int interval);

class ContextGL_Win : public ContextGL {

	HDC hDC;
	HGLRC hRC;
	unsigned int pixel_format;
	HWND hWnd;
	bool opengl_3_context;
	bool use_vsync;


	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
public:


	virtual void release_current();

	virtual void make_current();

	virtual int get_window_width();
	virtual int get_window_height();
	virtual void swap_buffers();

	virtual Error initialize();

	virtual void set_use_vsync(bool p_use);
	virtual bool is_using_vsync() const;

	ContextGL_Win(HWND hwnd,bool p_opengl_3_context);
	~ContextGL_Win();

};

#endif
#endif
