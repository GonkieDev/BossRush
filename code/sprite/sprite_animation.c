function SpriteAnim
SpriteAnimFromParams(M_Arena *arena, SA_State *sa_state, SpriteAnimParams *params)
{
    SpriteAnim anim  = {0};
    anim.duration    = params->duration;
    anim.slicesCount = params->filepathsCount;
    anim.slices      = M_PushArray(arena, SA_Slice, anim.slicesCount);
    
    for (u32 i=0; i < anim.slicesCount; ++i)
    {
        anim.slices[i] = SA_SpriteFromFile(sa_state, params->filepaths[i]);
    }
    
    return anim;
}

inline_function SA_Slice
SpriteAnimSliceFromTime(SpriteAnim *anim, f32 time)
{
    while (time > anim->duration)
    {
        time -= anim->duration;
    }
    f32 percent = time / anim->duration;
    u32 sliceIdx = (u32)(percent * (f32)anim->slicesCount);
    return anim->slices[sliceIdx];
}

inline_function SpriteAnimTreeDir 
SpriteAnimTreeDirMakeFromParams(M_Arena *arena, SA_State *sa_state, SpriteAnimParams params[Dir2D_COUNT])
{
    SpriteAnimTreeDir tree = {0};
    for (u32 i = 0; i < Dir2D_COUNT; ++i)
    {
        tree.anims[i] = SpriteAnimFromParams(arena, sa_state, &params[i]);
    }
    return tree;
}