/* date = July 18th 2023 2:29 pm */

#ifndef RENDER_TYPES_INCLUDE_H
#define RENDER_TYPES_INCLUDE_H

//////////////////////////////////////////
//~ gsp: Textures 

typedef enum R_Texture2DSampling
{
    R_Texture2DSampling_Billinear,
    R_Texture2DSampling_Nearest,
    
    R_Texture2DSampling_COUNT
} R_Texture2DSampling;

typedef enum R_Texture2DFormat
{
    R_Texture2DFormat_Null,
    R_Texture2DFormat_R8,
    R_Texture2DFormat_R8_G8_B8,
    R_Texture2DFormat_R8_G8_B8_A8,
    R_Texture2DFormat_R32_G32_B32_A32,
    R_Texture2DFormat_COUNT
} R_Texture2DFormat;

// NOTE(gsp): when changing this don't forget to change
// functions for R_XX_Texture2D 
typedef struct R_TexHandle R_TexHandle;
struct R_TexHandle
{
    u64 a;
};

//////////////////////////////////////////
//~
typedef struct R_MeshKey R_MeshKey;
struct R_MeshKey
{
    u64 a;
};

//////////////////////////////////////////
//~ gsp: Render commands 

typedef enum R_CmdKind
{
    R_CmdKind_Null,
    
    R_CmdKind_Mesh,
    R_CmdKind_UI_Rect2D,
    
    R_CmdKind_COUNT
} R_CmdKind;

typedef struct R_Cmd R_Cmd;
struct R_Cmd
{
    R_CmdKind kind;
    u32 __pad;
    union
    {
        R_TexHandle texture;
        R_TexHandle tex;
    };
    
    HMM_Mat4 modelMat;
    HMM_Vec4 color;
    
    // R_CmdKind_Mesh
    union
    {
        R_MeshKey meshKey;
        R_MeshKey mesh;
    };
    
    // R_CmdKind_UI_Rect2D
    Rng2F32 rect;
    Rng2F32 uv;
    
    PAD32(2);
    
#if BUILD_DEBUG
    f32 circle;
    PAD32(3);
#endif
};

// NOTE(gsp): there is an independent R_CmdSprite3D since they have to be drawn
// back to front due to opacity
typedef struct R_CmdSprite3D R_CmdSprite3D;
struct R_CmdSprite3D
{
    HMM_Mat4    modelMat;
    HMM_Vec4    color;
    Rng2F32     uv;
    R_TexHandle tex;
    PAD32(2);
};

typedef struct R_CmdNode R_CmdNode;
struct R_CmdNode
{
    R_Cmd *cmd;
    R_CmdNode *next;
};

typedef struct R_CmdNodeSprite3D R_CmdNodeSprite3D;
struct R_CmdNodeSprite3D
{
    R_CmdNodeSprite3D *next;
    R_CmdNodeSprite3D *prev;
    R_CmdSprite3D     *cmd;
};

typedef struct R_CmdList R_CmdList;
struct R_CmdList
{
    R_CmdNode *first;
    R_CmdNode *last;
    u64 count;
    
    // NOTE(gsp): sorted back to front by R_CmdSprite3DPush
    R_CmdNodeSprite3D *sprite3DFirst;
    R_CmdNodeSprite3D *sprite3DLast;
    u64 sprite3DCount;
};

////////////////////////////////////////
//~ End of file
#endif //RENDER_TYPES_INCLUDE_H
