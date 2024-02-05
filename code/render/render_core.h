/* date = July 18th 2023 1:58 pm */

#ifndef RENDER_CORE_INCLUDE_H
#define RENDER_CORE_INCLUDE_H

////////////////////////////////////////
//~ gsp: max defines
#define R_MAX_TEXTURES   10

#define R_EZM_MAX_MESHES 500

#define R_MAX_UI_RECTS    5000
#define R_MAX_UI_VERTICES R_MAX_UI_RECTS*6

#define R_MAX_SPRITE3D_RECTS    5000
#define R_MAX_SPRITE3D_VERTICES R_MAX_SPRITE3D_RECTS*6

////////////////////////////////////////
//~ gsp: Backend abstraction

//- gsp: general api 
function b32  R_Init(OS_Handle osWindowHandle);
function void R_Shutdown(void);

//- gsp: frame & render
#define R_Frame(fov, cmdListPtr) DeferLoopChecked(R_FrameBegin(fov), R_FrameEnd(cmdListPtr))
function b32  R_FrameBegin(f32 fov);
function void R_FrameEnd(R_CmdList *cmdList);

//- gsp: textures 
function R_TexHandle R_Texture2DAlloc(Vec2S32 size, R_Texture2DFormat fmt, R_Texture2DSampling sampling, void *initialData);
function void        R_Texture2DRelease(R_TexHandle texture);
function void        R_Texture2DFillRegion(R_TexHandle texture, Rng2S32 region, void *data);
function Vec2S32     R_SizeFromTexture2D(R_TexHandle texture);
function R_Texture2DFormat R_FormatFromTexture2D(R_TexHandle texture);
function b32         R_TexHandleIsValid(R_TexHandle texHandle);

//- gsp: meshes 
function R_MeshKey R_EzmMeshAlloc(EzmModel *model, EzmMesh *ezmMesh);
inline_function b32 R_MeshKeyIsValid(R_MeshKey meshKey);

//- gsp: R_Cmd helpers
function void R_CmdPush(M_Arena *arena, R_CmdList *list, R_Cmd cmd);
function void R_CmdSprite3DPush(M_Arena *arena, R_CmdList *list, R_CmdSprite3D cmd);

//- Camera Matrix
function void R_CamMatrixUpdate(HMM_Mat4 *camMatrix);


////////////////////////////////////////
//~ End of file
#endif //RENDER_CORE_INCLUDE_H
