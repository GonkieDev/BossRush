/////////////////////////////////////////////////////////
//~ gsp: API
function SA_State *
SA_Init(M_Arena *arena, Vec2S32 atlasSize)
{
    SA_State *state = M_PushStruct(arena, SA_State);
    state->arena = arena;
    state->atlasSize = atlasSize;
    SA_AtlasInit(state);
    return state;
}

function SA_Slice
SA_SpriteFromFile(SA_State *sa_state, String8 filepath)
{
    SA_Slice slice = {0};
    ScratchArenaScope(scratch, 0, 0)
    {
        A_Image image = A_ImageFromFile(scratch.arena, 4, filepath);
        Assert(image.channels == 4);
        slice = SA_AtlasAlloc(sa_state, image.size);
        R_Texture2DFillRegion(slice.tex, slice.region, image.data);
    }
    return slice;
}

/////////////////////////////////////////////////////////
//~
function SA_Atlas *
SA_AtlasInit(SA_State *sa_state)
{
    SA_Atlas atlas = {0};
    
    atlas.allocator = M_AtlasInit(sa_state->arena, sa_state->atlasSize);
    if (!atlas.allocator)
        return 0;
    
    atlas.tex = R_Texture2DAlloc(sa_state->atlasSize,  R_Texture2DFormat_R8_G8_B8_A8, 
                                 R_Texture2DSampling_Nearest, 0);
    
#if R_BACKEND_GL
    R_GL_NameTexture(atlas.tex, "(Tex) Sprite Atlas");
#endif
    
    
    if (!R_TexHandleIsValid(atlas.tex))
        return 0;
    
    // All good - allocate
    SA_Atlas *result = M_PushStruct(sa_state->arena, SA_Atlas);
    Assert(result);
    if (!result)
    {
        R_Texture2DRelease(atlas.tex);
    }
    
    MemoryCopyStruct(result, &atlas);
    
    // Add to list
    {
        SA_AtlasList *list = &sa_state->atlasList;
        StackPush(list->first, result);
        ++list->count;
    }
    
    return result;
}

function SA_Slice
SA_AtlasAlloc(SA_State *sa_state, Vec2S32 size)
{
    SA_Slice result = {0};
    
    SA_Atlas *atlas = sa_state->atlasList.first;
    for (; atlas != 0; atlas = atlas->next)
    {
        M_Atlas *allocator = atlas->allocator;
        Rng2S32 region = M_AtlasRegionAlloc(sa_state->arena, allocator, size);
        if (!R2IsNil(region))
        {
            result.region = region;
            result.tex = atlas->tex;
            break;
        }
    }
    
    if (!atlas)
    {
        atlas = SA_AtlasInit(sa_state);
        if (atlas)
        {
            Rng2S32 region = M_AtlasRegionAlloc(sa_state->arena, atlas->allocator, size);
            result.region = region;
            result.tex = atlas->tex;
        }
    }
    
    return result;
}
