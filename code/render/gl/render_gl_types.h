#ifndef RENDER_GL_TYPES_INCLUDE_H
#define RENDER_GL_TYPES_INCLUDE_H

#define GL_LOG_FILE "gl_log"

/////////////////////////////////////////////////////
//~ @gl_base_types
typedef struct { union { u32 n; u32 name; }; } GL_SSBO;
typedef struct { union { u32 n; u32 name; }; } GL_UBO;
typedef struct { union { u32 n; u32 name; }; } GL_SP;   // shader program
typedef struct { union { u32 n; u32 name; }; } GL_EBO;
typedef struct { union { u32 n; u32 name; }; } GL_VAO;

/////////////////////////////////////////////////////
//~

typedef struct R_GL_ProjViewport R_GL_ProjViewport;
struct R_GL_ProjViewport
{
    HMM_Mat4 proj;
    HMM_Vec2 viewport;
    HMM_Vec2 __pad;
};

typedef struct R_GL_GlobalUBO R_GL_GlobalUBO;
struct R_GL_GlobalUBO
{
    HMM_Mat4 cam;
    // NOTE: leave proj and viewport in this order and next to each other
    // so that when window changes size we can upload both of them
    union
    {
        R_GL_ProjViewport projViewport;
        struct
        {
            HMM_Mat4 proj;
            HMM_Vec2 viewport;
            HMM_Vec2 __pad;
        };
    };
};

typedef struct R_GL_EzmMeshUBO R_GL_EzmMeshUBO;
struct R_GL_EzmMeshUBO
{
    HMM_Mat4 model;
};

////////////////////////////////////////
//~ gsp: Bound State
typedef struct R_GL_BoundState R_GL_BoundState;
struct R_GL_BoundState
{
    GL_VAO vao;
    GL_UBO ubo;
    GL_SP  sp;
};

////////////////////////////////////////
//~ gsp: Window equip
typedef struct R_GL_WindowEquip R_GL_WindowEquip;
struct R_GL_WindowEquip
{
    OS_Handle os_window;
    Vec2S32 clientDims;
};

////////////////////////////////////////
//~
typedef struct R_GL_Texture R_GL_Texture;
struct R_GL_Texture
{
    Vec2S32 size;
    R_Texture2DFormat fmt;
    R_Texture2DSampling sampling;
    u64 handle;   // u64 bit handle for bindless textures
    u32 id;       // gl 'name'
    u32 texIndex; // in gl_state array
    u32 levels;   // mipmap levels
    PAD32(1);
};

////////////////////////////////////////
//~ Mesh
typedef struct R_GL_EzmMesh R_GL_EzmMesh;
struct R_GL_EzmMesh
{
    GL_SSBO vertexBuffer;
    GL_SSBO indexBuffer;
    u32     indicesCount;
};

////////////////////////////////////////
//~ UI Vertex
typedef struct R_GL_UIVertex R_GL_UIVertex;
struct R_GL_UIVertex
{
    Rng2F32 rect;
    Rng2F32 uv;
    HMM_Vec4 color;
    u32 texId;
    PAD32(3);
};

////////////////////////////////////////
//~ Sprite3D Vertex
typedef struct R_GL_Sprite3DVertex R_GL_Sprite3DVertex;
struct R_GL_Sprite3DVertex
{
    Rng2F32  uv;
    HMM_Vec4 color;
    u32      texId;
    PAD32(3);
};

////////////////////////////////////////
//~ Sprite3D Vertex
typedef struct R_GL_Sprite3DInstanceData R_GL_Sprite3DInstanceData;
struct R_GL_Sprite3DInstanceData
{
    HMM_Mat4 modelMat;
};

/////////////////////////////////////////////////////
//~ @r_gl_state
#if COMPILER_MSVC
#pragma warning(disable: 4820)
#endif

typedef struct R_GL_State R_GL_State;
struct R_GL_State 
{
    //- Global
    GL_VAO nullVAO;
    GL_UBO globalUBO;
    
    R_GL_Texture *defaultTex;
    
    //- UI
    GL_SP uiShaderProgram;
    GL_SSBO uissbo;
    
    //- Sprite3D
    GL_SP   sprite3DShaderProgram;
    GL_SSBO sprite3Dvertexssbo;
    GL_SSBO sprite3DinstanceSSBO;
    
    //- Ezm
    GL_SP ezmShaderProgram;
    u32 ezmShaderProgramModelLoc;
    u32 ezmShaderProgramAlbedoLoc;
    u32 ezmShaderProgramShadeColorLoc;
    
    u32 ezmMeshesCount;
    R_GL_EzmMesh ezmMeshes[R_EZM_MAX_MESHES];
    
    //- Textures
    u32 texturesCount;
    R_GL_Texture textures[R_MAX_TEXTURES];
    GL_SSBO texssbo;
    
    //- Bound state
    R_GL_BoundState bs;
    
    //- Queried values
    // TODO(gsp): @gl_query ssbo max size
    // TODO(gsp): @gl_query max textures
    i32 gl_maxTextureSize;
    
    //-
    M_Arena *arena;
    R_GL_WindowEquip eqp;
    
#if BUILD_DEBUG
    i32 maxLabelLen;
#endif
};

#if COMPILER_MSVC
#pragma warning(default: 4820)
#endif

////////////////////////////////////////
//~ END OF FILE
#endif //RENDER_GL_TYPES_INCLUDE_H
