#ifndef DRAW_CORE_INCLUDE_H
#define DRAW_CORE_INCLUDE_H

typedef struct D_MeshParams D_MeshParams;
struct D_MeshParams
{
    HMM_Vec4 color;
    HMM_Quat rot;
    HMM_Vec3 pos;
    HMM_Vec3 size;
    R_TexHandle tex;
    R_MeshKey   mesh;
    PAD32(2);
};

typedef struct D_Sprite3DParams D_Sprite3DParams;
struct D_Sprite3DParams
{
    HMM_Vec4 color;
    HMM_Vec3 pos;
    HMM_Vec2 size;
    PAD32(1);
    SA_Slice slice;
};

/////////////////////////////////////////////////////
//~
inline_function void D_SetPtrs(M_Arena *arena, R_CmdList *cmdList);

/////////////////////////////////////////////////////
//~ mesh
#define D_Mesh(...) D_Mesh_((D_MeshParams){    \
.size.x = 1.f, .size.y = 1.f, .size.z = 1.f,    \
.rot = HMM_Q(0.f, 0.f, 0.f, 1.f), \
.color = HMM_V4(1.f, 1.f, 1.f, 1.f),            \
##__VA_ARGS__ })
inline_function void D_Mesh_(D_MeshParams params);

/////////////////////////////////////////////////////
//~ Sprite3D
#define D_Sprite3D(...) D_Sprite3D_(&g_state->mainCam, (D_Sprite3DParams){                              \
.size.x = 5.f, .size.y = 5.f,                   \
.color = HMM_V4(1.f, 1.f, 1.f, 1.f),            \
##__VA_ARGS__ })

inline_function void D_Sprite3D_(Camera *cam, D_Sprite3DParams params);


/////////////////////////////////////////////////////
//~ text

// NOTE(gsp): return: .x: line width, .y:  line height
function HMM_Vec2 D_DrawText_(FP_Handle font, f32 fontSize, HMM_Vec2 pos, HMM_Vec4 color, String8 text);
function HMM_Vec2 D_DrawTextFmt(FP_Handle font, f32 fontSize, HMM_Vec2 pos, HMM_Vec4 color, char *fmt, ...);

#define D_DrawTextQ(pos, fmt, ...) \
D_DrawTextFmt(g_state->uiFont, g_state->uiFontSize, pos, HMM_V4(1.f, 1.f, 1.f, 1.f), fmt, ##__VA_ARGS__)
#define D_DrawText(font, fontSize, pos, color, fmt, ...) \
D_DrawTextFmt(font, fontSize, pos, color, fmt, ##__VA_ARGS__)

/////////////////////////////////////////////////////
//~ @D_Debug
#if BUILD_DEBUG
//# define D_DebugArrow(pos, dir) D_Mesh(.mesh = g_state->arrow, .pos = (pos), .size = (dir) )
#define D_DebugArrow(pos, dir, color) D_DebugArrow_(pos, dir, color)
#define D_DebugCircle(pos, radius, color) D_DebugCircle_(pos, radius, color)
#define D_DebugSphere(pos, radius, color) D_DebugSphere_(pos, radius, color)

#else // #if BUILD_DEBUG
#define D_DebugArrow(pos, dir, color)
#define D_DebugCircle(pos, radius, color)
#endif // #if BUILD_DEBUG

#if BUILD_DEBUG

function void D_DebugArrow_(HMM_Vec3 pos, HMM_Vec3 dir, HMM_Vec4 color);
function void D_DebugCircle_(HMM_Vec3 pos, f32 radius, HMM_Vec4 color);
function void D_DebugSphere_(HMM_Vec3 pos, f32 radius, HMM_Vec4 color);

#endif // #if BUILD_DEBUG


/////////////////////////////////////////////////////
//~ END OF FILE
#endif //DRAW_CORE_INCLUDE_H
