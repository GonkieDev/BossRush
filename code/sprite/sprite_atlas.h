#ifndef SPRITE_ATLAS_INCLUDE_H
#define SPRITE_ATLAS_INCLUDE_H

typedef struct SA_Atlas SA_Atlas;
struct SA_Atlas
{
    M_Atlas     *allocator;
    R_TexHandle  tex;
    SA_Atlas    *next;
};

typedef struct SA_AtlasList SA_AtlasList;
struct SA_AtlasList
{
    SA_Atlas *first;
    u64       count;
};

typedef struct SA_State SA_State; 
struct SA_State
{
    SA_AtlasList atlasList;
    Vec2S32      atlasSize;
    // NOTE(gsp): this arena is given as parameter and cached here, not allocated by SA
    M_Arena     *arena;
};

typedef struct SA_Slice SA_Slice;
struct SA_Slice
{
    Rng2S32     region;
    R_TexHandle tex;
};

/////////////////////////////////////////////////////////
//~ gsp: API

function SA_State *SA_Init(M_Arena *arena, Vec2S32 atlasSize);
function SA_Slice  SA_SpriteFromFile(SA_State *sa_state, String8 filepath);

/////////////////////////////////////////////////////////
//~
function SA_Atlas *SA_AtlasInit(SA_State *sa_state);
function SA_Slice  SA_AtlasAlloc(SA_State *sa_state, Vec2S32 size);


/////////////////////////////////////////////////////////
//~ END OF FILE
#endif //SPRITE_ATLAS_INCLUDE_H
