global M_Arena   *d_cmd_list_arena = 0;
global R_CmdList *d_cmd_list       = 0;

/////////////////////////////////////////////////////
//~
inline_function void
D_SetPtrs(M_Arena *arena, R_CmdList *cmdList)
{
    d_cmd_list_arena = arena;
    d_cmd_list       = cmdList;
}

/////////////////////////////////////////////////////
//~ mesh
inline_function void
D_Mesh_(D_MeshParams params)
{
    HMM_Mat4 model = HMM_Scale(params.size);
    model = HMM_Mul(HMM_QToM4(params.rot), model);
    model = HMM_Mul(HMM_Translate(params.pos), model);
    
    R_Cmd rcmd = {
        .kind  = R_CmdKind_Mesh,
        .color = params.color,
        .tex   = params.tex,
        .modelMat = model,
        .mesh  = params.mesh,
    };
    R_CmdPush(d_cmd_list_arena, d_cmd_list, rcmd);
}

/////////////////////////////////////////////////////
//~ Sprite3D
inline_function void 
D_Sprite3D_(Camera *cam, D_Sprite3DParams params)
{
    HMM_Vec3 pos = params.pos;
    HMM_Quat rot;
    {
        HMM_Vec3 fwd = cam->fwd;
        f32 pitch = asinf(fwd.y);
        f32 yaw = atan2f(fwd.x, fwd.z);
        
        rot = HMM_Q(0.f, 0.f, 0.f, 1.f);
        rot = HMM_MulQ(HMM_QFromAxisAngle_RH(HMM_V3(1.f, 0.f, 0.f), -pitch), rot);
        rot = HMM_MulQ(HMM_QFromAxisAngle_RH(HMM_V3(0.f, 1.f, 0.f), yaw), rot);
    }
    
    HMM_Mat4 model = HMM_Scale(HMM_V3(params.size.x, params.size.y, 1.f));
    
    model = HMM_Mul(HMM_QToM4(rot), model);
    model = HMM_Mul(HMM_Translate(pos), model);
    
    HMM_Vec2 imageSize = Vec2FromVec(R_SizeFromTexture2D(params.slice.tex));
    Rng2F32 uv = Rng2F32FromRng2(params.slice.region);
    uv.min = HMM_DivV2(uv.min, imageSize);
    uv.max = HMM_DivV2(uv.max, imageSize);
    
    // Find distance to camera
    R_CmdSprite3D rcmd = {
        .color     = params.color,
        .tex       = params.slice.tex,
        .uv        = uv,
        .modelMat  = model,
    };
    R_CmdSprite3DPush(d_cmd_list_arena, d_cmd_list, rcmd);
}

/////////////////////////////////////////////////////
//~ text

function HMM_Vec2
D_DrawText_(FP_Handle font, f32 fontSize, HMM_Vec2 startPos, HMM_Vec4 color, String8 text)
{
    HMM_Vec2 result = {0};
    
    HMM_Vec2 pos = startPos;
    ScratchArenaScope(scratch, &d_cmd_list_arena, 1)
    {
        FC_GlyphRun glyphRun = FC_GlyphRunFromString(scratch.arena, font, fontSize, text, V2S32(0, 0));
        for (u32 i = 0; i < text.size; ++i)
        {
            FC_GlyphInfo *glyphInfo = glyphRun.infos[i];
            Rng2F32 charRect = R2F32(pos, HMM_Add(pos, glyphInfo->dim));
            
            R_TexHandle tex = glyphInfo->texture;
            Rng2F32 uv = Rng2F32FromRng2(glyphInfo->uv);
            {
                Vec2S32 texSizeS32 = R_SizeFromTexture2D(tex);
                HMM_Vec2 texSize = HMM_V2((f32)texSizeS32.x, (f32)texSizeS32.y);
                uv.p0 = HMM_DivV2(uv.p0, texSize);
                uv.p1 = HMM_DivV2(uv.p1, texSize);
            }
            
            R_CmdPush(d_cmd_list_arena, d_cmd_list, (R_Cmd){
                          .kind  = R_CmdKind_UI_Rect2D,
                          .rect  = charRect,
                          .color = color,
                          .tex   = tex,
                          .uv    = uv,
                      });
            
            pos.x += glyphInfo->advanceWidth + glyphRun.kerningAdjustments[i];
        }
    }
    
    result.x = pos.x - startPos.x;
    result.y = fontSize;
    return result;
}

function HMM_Vec2
D_DrawTextFmt(FP_Handle font, f32 fontSize, HMM_Vec2 pos, HMM_Vec4 color, char *fmt, ...)
{
    HMM_Vec2 result = {0};
    ScratchArenaScope(scratch, &d_cmd_list_arena, 1)
    {
        va_list args;
        va_start(args, fmt);
        String8 formatted = PushStr8FmtVAList(scratch.arena, fmt, args);
        va_end(args);
        result = D_DrawText_(font, fontSize, pos, color, formatted);
    }
    return result;
}

/////////////////////////////////////////////////////
//~ @D_Debug
#if BUILD_DEBUG

function void
D_DebugArrow_(HMM_Vec3 pos, HMM_Vec3 dir, HMM_Vec4 color)
{
    HMM_Vec3 normDir = HMM_Norm(dir);
    f32 len = HMM_Len(dir);
    
    HMM_Mat4 model = HMM_Scale(HMM_V3(len, 1.f, 1.f));
    
    f32 pitch = asinf(normDir.y);
    f32 yaw = atan2f(-normDir.z, normDir.x);
    
    HMM_Mat4 rot;
    rot = HMM_Rotate_RH(pitch, HMM_V3(0.f, 0.f, 1.f));
    rot = HMM_Mul(HMM_Rotate_RH(yaw, HMM_V3(0.f, 1.f, 0.f)), rot);
    model = HMM_Mul(rot, model);
    
    model = HMM_Mul(HMM_Translate(pos), model);
    
    R_CmdPush(d_cmd_list_arena, d_cmd_list, (R_Cmd){
                  .kind = R_CmdKind_Mesh,
                  .mesh = g_state->arrow,
                  .modelMat = model,
                  .color = color,
              });
}

function void
D_DebugCircle_(HMM_Vec3 pos, f32 radius, HMM_Vec4 color)
{
    HMM_Mat4 model = HMM_Scale(HMM_V3(radius, 1.f, radius));
    model = HMM_Mul(HMM_Translate(HMM_Add(pos, HMM_V3(0.f, 0.25f, 0.f))), model);
    
    R_CmdPush(d_cmd_list_arena, d_cmd_list, (R_Cmd){
                  .kind = R_CmdKind_Mesh,
                  .mesh = g_state->filledCircle,
                  .modelMat = model,
                  .color = color,
              });
}

function void
D_DebugSphere_(HMM_Vec3 pos, f32 radius, HMM_Vec4 color)
{
    HMM_Mat4 model = HMM_Scale(HMM_V3(radius, radius, radius));
    model = HMM_Mul(HMM_Translate(pos), model);
    
    R_Cmd rcmd = {
        .kind  = R_CmdKind_Mesh,
        .color = color,
        .modelMat = model,
        .mesh = g_state->uvsphere,
    };
    R_CmdPush(d_cmd_list_arena, d_cmd_list, rcmd);
}

#endif // #if BUILD_DEBUG