//
// Copyright (C) 2016 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Google Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest.h>

#include "TestFixture.h"

namespace glslangtest {
namespace {

using PreprocessingTest = GlslangTest<::testing::TestWithParam<std::string>>;

TEST_P(PreprocessingTest, FromFile)
{
    loadFilePreprocessAndCheck(GlobalTestSettings.testRoot, GetParam());
}

// clang-format off
INSTANTIATE_TEST_CASE_P(
    Glsl, PreprocessingTest,
    ::testing::ValuesIn(std::vector<std::string>({
        "preprocessor.bad_arg.vert",
        "preprocessor.cpp_style_line_directive.vert",
        "preprocessor.cpp_style___FILE__.vert",
        "preprocessor.edge_cases.vert",
        "preprocessor.errors.vert",
        "preprocessor.extensions.vert",
        "preprocessor.function_macro.vert",
        "preprocessor.include.enabled.vert",
        "preprocessor.include.disabled.vert",
        "preprocessor.line.vert",
        "preprocessor.line.frag",
        "preprocessor.pragma.vert",
        "preprocessor.simple.vert",
        "preprocessor.success_if_parse_would_fail.vert",
        "preprocessor.defined.vert",
        "preprocessor.many.endif.vert",
        "preprocessor.eof_missing.vert",
    })),
    FileNameAsCustomTestSuffix
);
// clang-format on

}  // anonymous namespace
}  // namespace glslangtest
