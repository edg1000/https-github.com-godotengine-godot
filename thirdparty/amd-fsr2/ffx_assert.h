// This file is part of the FidelityFX SDK.
//
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "ffx_types.h"
#include "ffx_util.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

#ifdef _DEBUG
#ifdef _WIN32

#ifdef DISABLE_FFX_DEBUG_BREAK
#define FFX_DEBUG_BREAK \
    {                   \
    }
#else
/// Macro to force the debugger to break at this point in the code.
#define FFX_DEBUG_BREAK __debugbreak();
#endif
#else
#define FFX_DEBUG_BREAK \
    {                   \
    }
#endif
#else
// don't allow debug break in release builds.
#define FFX_DEBUG_BREAK
#endif

/// A typedef for the callback function for assert printing.
///
/// This can be used to re-route printing of assert messages from the FFX backend
/// to another destination. For example instead of the default behaviour of printing
/// the assert messages to the debugger's TTY the message can be re-routed to a
/// MessageBox in a GUI application.
///
/// @param [in] message                 The message generated by the assert.
///
typedef void (*FfxAssertCallback)(const char* message);

/// Function to report an assert.
///
/// @param [in] file                    The name of the file as a string.
/// @param [in] line                    The index of the line in the file.
/// @param [in] condition               The boolean condition that was tested.
/// @param [in] msg                     The optional message to print.
///
/// @returns
/// Always returns true.
///
FFX_API bool ffxAssertReport(const char* file, int32_t line, const char* condition, const char* msg);

/// Provides the ability to set a callback for assert messages.
///
/// @param [in] callback                The callback function that will receive assert messages.
///
FFX_API void ffxAssertSetPrintingCallback(FfxAssertCallback callback);

#ifdef _DEBUG
/// Standard assert macro.
#define FFX_ASSERT(condition)                                                      \
    do                                                                             \
    {                                                                              \
        if (!(condition) && ffxAssertReport(__FILE__, __LINE__, #condition, NULL)) \
            FFX_DEBUG_BREAK                                                        \
    } while (0)

/// Assert macro with message.
#define FFX_ASSERT_MESSAGE(condition, msg)                                        \
    do                                                                            \
    {                                                                             \
        if (!(condition) && ffxAssertReport(__FILE__, __LINE__, #condition, msg)) \
            FFX_DEBUG_BREAK                                                       \
    } while (0)

/// Assert macro that always fails.
#define FFX_ASSERT_FAIL(message)                            \
    do                                                      \
    {                                                       \
        ffxAssertReport(__FILE__, __LINE__, NULL, message); \
        FFX_DEBUG_BREAK                                     \
    } while (0)
#else
// asserts disabled
#define FFX_ASSERT(condition)  \
    do                         \
    {                          \
        FFX_UNUSED(condition); \
    } while (0)

#define FFX_ASSERT_MESSAGE(condition, message) \
    do                                         \
    {                                          \
        FFX_UNUSED(condition);                 \
        FFX_UNUSED(message);                   \
    } while (0)

#define FFX_ASSERT_FAIL(message) \
    do                           \
    {                            \
        FFX_UNUSED(message);     \
    } while (0)
#endif  // #if _DEBUG

/// Simple static assert.
#define FFX_STATIC_ASSERT(condition) static_assert(condition, #condition)

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
