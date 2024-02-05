#ifndef RENDER_GL_LOADER_INCLUDE_H
#define RENDER_GL_LOADER_INCLUDE_H

//~ WGL defines
//- WGL Context defines 
// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB       0x0001

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

//~ OpenGL defines
#ifndef __khrplatform_h_
typedef          float         khronos_float_t;
typedef signed   char          khronos_int8_t;
typedef unsigned char          khronos_uint8_t;
typedef signed   short int     khronos_int16_t;
typedef unsigned short int     khronos_uint16_t;
#ifdef _WIN64
typedef signed   long long int khronos_intptr_t;
typedef signed   long long int khronos_ssize_t;
#else
typedef signed   long  int     khronos_intptr_t;
typedef signed   long  int     khronos_ssize_t;
#endif

#if defined(_MSC_VER) && !defined(__clang__)
typedef signed   __int64       khronos_int64_t;
typedef unsigned __int64       khronos_uint64_t;
#elif (defined(__clang__) || defined(__GNUC__)) && (__cplusplus < 201100)
#include <stdint.h>
typedef          int64_t       khronos_int64_t;
typedef          uint64_t      khronos_uint64_t;
#else
typedef signed   long long     khronos_int64_t;
typedef unsigned long long     khronos_uint64_t;
#endif
#endif  // __khrplatform_h_

typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;

typedef void GLvoid;
typedef unsigned int GLenum;

typedef khronos_float_t GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef khronos_uint8_t GLubyte;

typedef u64 GLuint64;

typedef char GLchar;

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
#endif
#ifndef APIENTRY
# define APIENTRY
#endif
#ifndef APIENTRYP
# define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
# define GLAPI extern
#endif

//~
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_TRIANGLES                      0x0004
#define GL_ONE                            1
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408
#define GL_POLYGON_MODE                   0x0B40
#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_STENCIL_TEST                   0x0B90
#define GL_VIEWPORT                       0x0BA2
#define GL_BLEND                          0x0BE2
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_FILL                           0x1B02
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_LINK_STATUS                    0x8B82
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002 // glew.h
#define GL_CONTEXT_FLAGS 0x821E // glew.h
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_SOURCE_API 0x8246

#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B

#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_RENDERBUFFER                   0x8D41
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_MAX_LABEL_LENGTH 0x82E8

#define GL_PROGRAM 0x82E2
#define GL_BUFFER 0x82E0
#define GL_SHADER 0x82E1
#define GL_QUERY 0x82E3
#define GL_PROGRAM_PIPELINE 0x82E4
#define GL_TRANSFORM_FEEDBACK 0x8E22
#define GL_SAMPLER 0x82E6
#define GL_FRAMEBUFFER 0x8D40

#define GL_MIRRORED_REPEAT 0x8370
#define GL_CLAMP_TO_EDGE   0x812F


#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2

#define GL_NUM_EXTENSIONS   0x821D
#define GL_MAX_TEXTURE_SIZE 0x0D33

#define GL_RGBA8        0x8058
#define GL_SRGB8        0x8C41
#define GL_SRGB_ALPHA   0x8C42
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_RGBA32UI     0x8D70
#define GL_RGB32UI      0x8D71
#define GL_RGBA16UI     0x8D76
#define GL_RGB16UI      0x8D77
#define GL_RGBA8UI      0x8D7C
#define GL_RGB8UI       0x8D7D
#define GL_RGBA32I      0x8D82
#define GL_RGB32I       0x8D83
#define GL_RGBA16I      0x8D88
#define GL_RGB16I       0x8D89
#define GL_RGBA8I       0x8D8E
#define GL_RGB8I        0x8D8F
#define GL_RGB8         0x8051
#define GL_UNSIGNED_INT_8_8_8_8 0x8035

#define GL_UNSIGNED_INT64_ARB 0x140F

enum
{
    E_GL_SizedFormat_RGBA8 = GL_RGBA8,
    E_GL_SizedFormat_RGB8  = GL_RGB8,
};

/* ------------------------- GL_ARB_buffer_storage ------------------------- */
#define GL_MAP_READ_BIT                     0x0001
#define GL_MAP_WRITE_BIT                    0x0002
#define GL_MAP_PERSISTENT_BIT               0x00000040
#define GL_MAP_COHERENT_BIT                 0x00000080
#define GL_DYNAMIC_STORAGE_BIT              0x0100
#define GL_CLIENT_STORAGE_BIT               0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE         0x821F
#define GL_BUFFER_STORAGE_FLAGS             0x8220

#define GL_SHADER_STORAGE_BUFFER 0x90D2

////////////////////////////////////////
//~ 
function void *R_GL_GetGLFuncAddress(char *name);

typedef void (APIENTRY *DEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

typedef const GLubyte *GLGETSTRINGI(GLenum name, GLuint index);
static GLGETSTRINGI *glGetStringi;

#include "gl_func_loader_generated.h"

////////////////////////////////////////
//~
#endif //RENDER_GL_LOADER_INCLUDE_H
