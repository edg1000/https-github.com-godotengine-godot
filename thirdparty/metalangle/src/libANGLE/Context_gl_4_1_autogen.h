// GENERATED FILE - DO NOT EDIT.
// Generated by generate_entry_points.py using data from gl.xml.
//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Context_gl_4_1_autogen.h: Creates a macro for interfaces in Context.

#ifndef ANGLE_CONTEXT_GL_4_1_AUTOGEN_H_
#define ANGLE_CONTEXT_GL_4_1_AUTOGEN_H_

#define ANGLE_GL_4_1_CONTEXT_API                                                                   \
    void depthRangeArrayv(GLuint first, GLsizei count, const GLdouble *v);                         \
    void depthRangeIndexed(GLuint index, GLdouble n, GLdouble f);                                  \
    void getDoublei_v(GLenum target, GLuint index, GLdouble *data);                                \
    void getFloati_v(GLenum target, GLuint index, GLfloat *data);                                  \
    void getVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params);                         \
    void programUniform1d(ShaderProgramID programPacked, GLint location, GLdouble v0);             \
    void programUniform1dv(ShaderProgramID programPacked, GLint location, GLsizei count,           \
                           const GLdouble *value);                                                 \
    void programUniform2d(ShaderProgramID programPacked, GLint location, GLdouble v0,              \
                          GLdouble v1);                                                            \
    void programUniform2dv(ShaderProgramID programPacked, GLint location, GLsizei count,           \
                           const GLdouble *value);                                                 \
    void programUniform3d(ShaderProgramID programPacked, GLint location, GLdouble v0, GLdouble v1, \
                          GLdouble v2);                                                            \
    void programUniform3dv(ShaderProgramID programPacked, GLint location, GLsizei count,           \
                           const GLdouble *value);                                                 \
    void programUniform4d(ShaderProgramID programPacked, GLint location, GLdouble v0, GLdouble v1, \
                          GLdouble v2, GLdouble v3);                                               \
    void programUniform4dv(ShaderProgramID programPacked, GLint location, GLsizei count,           \
                           const GLdouble *value);                                                 \
    void programUniformMatrix2dv(ShaderProgramID programPacked, GLint location, GLsizei count,     \
                                 GLboolean transpose, const GLdouble *value);                      \
    void programUniformMatrix2x3dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void programUniformMatrix2x4dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void programUniformMatrix3dv(ShaderProgramID programPacked, GLint location, GLsizei count,     \
                                 GLboolean transpose, const GLdouble *value);                      \
    void programUniformMatrix3x2dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void programUniformMatrix3x4dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void programUniformMatrix4dv(ShaderProgramID programPacked, GLint location, GLsizei count,     \
                                 GLboolean transpose, const GLdouble *value);                      \
    void programUniformMatrix4x2dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void programUniformMatrix4x3dv(ShaderProgramID programPacked, GLint location, GLsizei count,   \
                                   GLboolean transpose, const GLdouble *value);                    \
    void scissorArrayv(GLuint first, GLsizei count, const GLint *v);                               \
    void scissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);    \
    void scissorIndexedv(GLuint index, const GLint *v);                                            \
    void vertexAttribL1d(GLuint index, GLdouble x);                                                \
    void vertexAttribL1dv(GLuint index, const GLdouble *v);                                        \
    void vertexAttribL2d(GLuint index, GLdouble x, GLdouble y);                                    \
    void vertexAttribL2dv(GLuint index, const GLdouble *v);                                        \
    void vertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);                        \
    void vertexAttribL3dv(GLuint index, const GLdouble *v);                                        \
    void vertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);            \
    void vertexAttribL4dv(GLuint index, const GLdouble *v);                                        \
    void vertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride,               \
                              const void *pointer);                                                \
    void viewportArrayv(GLuint first, GLsizei count, const GLfloat *v);                            \
    void viewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);               \
    void viewportIndexedfv(GLuint index, const GLfloat *v);

#endif  // ANGLE_CONTEXT_API_4_1_AUTOGEN_H_
