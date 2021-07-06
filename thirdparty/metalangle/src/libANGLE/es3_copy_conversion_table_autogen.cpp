// GENERATED FILE - DO NOT EDIT.
// Generated by gen_copy_conversion_table.py using data from es3_copy_conversion_formats.json.
//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// format_map:
//   Determining the sized internal format from a (format,type) pair.
//   Also check es3 format combinations for validity.

#include "angle_gl.h"
#include "common/debug.h"

namespace gl
{

bool ValidES3CopyConversion(GLenum textureFormat, GLenum framebufferFormat)
{
    switch (textureFormat)
    {
        case GL_ALPHA:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_BGRA_EXT:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                    return true;
                default:
                    break;
            }
            break;

        case GL_LUMINANCE:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RED:
                case GL_RG:
                case GL_RGB:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_LUMINANCE_ALPHA:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RED:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RED:
                case GL_RG:
                case GL_RGB:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RED_INTEGER:
            switch (framebufferFormat)
            {
                case GL_RED_INTEGER:
                case GL_RGBA_INTEGER:
                case GL_RGB_INTEGER:
                case GL_RG_INTEGER:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RG:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RG:
                case GL_RGB:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RGB:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RGB:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RGBA:
            switch (framebufferFormat)
            {
                case GL_BGRA_EXT:
                case GL_RGBA:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RGBA_INTEGER:
            switch (framebufferFormat)
            {
                case GL_RGBA_INTEGER:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RGB_INTEGER:
            switch (framebufferFormat)
            {
                case GL_RGBA_INTEGER:
                case GL_RGB_INTEGER:
                    return true;
                default:
                    break;
            }
            break;

        case GL_RG_INTEGER:
            switch (framebufferFormat)
            {
                case GL_RGBA_INTEGER:
                case GL_RGB_INTEGER:
                case GL_RG_INTEGER:
                    return true;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return false;
}

}  // namespace gl
