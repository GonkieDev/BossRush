#ifndef RENDER_GL_INCLUDE_H
#define RENDER_GL_INCLUDE_H

// DSA:
// glCreateTextures: 4.5
// glTextureStorage2D: 4.5
// glTextureSubImage2D: 4.5

#define R_GL_MAJOR_VER 4
#define R_GL_MINOR_VER 5

//- @GL_Extensions
global read_only String8 r_gl_extensions[] = {
    S8LitComp("GL_ARB_bindless_texture"),
    S8LitComp("GL_ARB_shader_storage_buffer_object"),
    S8LitComp("GL_ARB_direct_state_access")
};

////////////////////////////////////////
//~ [h] includes
#include <gl/gl.h>

#include "render_gl_loader.h"
#include "render_gl_types.h"

//- gl generated shaders include
#if COMPILER_MSVC
# pragma warning( push )
# pragma warning( disable : 4057 )
#endif

#include "builtin_shaders/render_gl_generated_shaders.h"

#if COMPILER_MSVC
# pragma warning( pop )
#endif

////////////////////////////////////////
//~ @GL_functions
function b32 R_GL_InitResources(void);

//- @Shader_functions
inline_function u32 R_GL_ShaderCompile(u8 **source, GLenum shaderType);
inline_function GL_SP R_GL_SP(u8 **vertSource, u8 **fragSource);

//- @GL_EZM_Functions
inline_function R_GL_EzmMesh *R_GL_EzmMeshFromMeshKey(R_MeshKey meshKey);
inline_function R_MeshKey R_MeshKeyFromGLEzmMesh(R_GL_EzmMesh *glEzmMesh);

//- @GL_Textures
inline_function R_GL_Texture *R_GL_TextureFromTexHandle(R_TexHandle handle);
inline_function R_TexHandle R_GL_TexHandleFromTexture(R_GL_Texture *tex);
inline_function u32 R_GL_Sampling(R_Texture2DSampling sampling);
inline_function u32 R_GL_DataFormat(R_Texture2DFormat fmt);
inline_function u32 R_GL_SizedFormat(R_Texture2DFormat fmt);

////////////////////////////////////////
//~ @R_GL_BoundState
inline_function void R_GL_BindUBO(GL_UBO ubo);
inline_function void R_GL_BindSP(GL_SP sp);
inline_function void R_GL_BindVAO(GL_VAO vao);
inline_function void R_GL_UnbindVAO(GL_VAO vao);

////////////////////////////////////////
//~ @GL_Misc
inline_function R_GL_UIVertex R_GL_UIVertexFromCmd(R_Cmd *cmd, u32 corner);
inline_function R_GL_Sprite3DVertex R_GL_Sprite3DVertexFromCmd(R_CmdSprite3D *cmd, u32 corner);

////////////////////////////////////////
//~ @GL_DebugFunctions
#if !defined(R_GL_ENABLE_IGNORE_WARNS)
# define R_GL_ENABLE_IGNORE_WARNS 0 
#endif

#if BUILD_DEBUG 

# if R_GL_ENABLE_IGNORE_WARNS
global u32 R_GL_IgnoreErrors[0] = {};
# endif

function void APIENTRY
R_GL_DebugOutputCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
                         GLsizei length, const char *message, const void *userParam);

// NOTE(gsp): I think you need to upload data to a buffer for it to become a valid gl 'name'/id
function void R_GL_LabelObject(GLenum type, u32 id, String8 str);
#define R_GL_NameObject(type, id, c_literal_str) R_GL_LabelObject(type, id, S8Lit(c_literal_str))

#define R_GL_NameTexture(texHandle, c_literal_str) R_GL_NameObject(GL_TEXTURE, R_GL_TextureFromTexHandle(texHandle)->id, c_literal_str)

#else // #if BUILD_DEBUG

#define R_GL_NameObject(type, id, c_str)
#define R_GL_NameTexture(texHandle, c_literal_str)

#endif // #if BUILD_DEBUG

////////////////////////////////////////
//~ @per_os GL Platform functions
function void *R_GL_GetGLFuncAddress(char *name);

function b32 R_GL_OS_Init(OS_Handle osWindowHandle);
function void R_GL_OS_FinishFrame(OS_Handle osHandle);

////////////////////////////////////////
//~ END OF FILE
#endif //RENDER_GL_INCLUDE_H
