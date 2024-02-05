#ifndef SPRITE_ANIMATION_INCLUDE_H
#define SPRITE_ANIMATION_INCLUDE_H

/////////////////////////////////////////////////////////
//~ structs

typedef struct SpriteAnim SpriteAnim;
struct SpriteAnim
{
    SA_Slice *slices;
    u32 slicesCount;
    f32 duration;
};

typedef struct SpriteAnimParams SpriteAnimParams;
struct SpriteAnimParams
{
    String8 *filepaths;
    u32 filepathsCount;
    f32 duration;
};

enum
{
    Dir2D_D,
    Dir2D_DL,
    Dir2D_L,
    Dir2D_UL,
    Dir2D_U,
    Dir2D_UR,
    Dir2D_R,
    Dir2D_DR,
    Dir2D_M,
    
    Dir2D_COUNT
};

#if 0
read_only global HMM_Vec2 dir2d_vec_arr[Dir2D_Count] = {
    [Dir2D_D]  = HMM_V2( 0.f, -1.f),
    [Dir2D_DL] = HMM_V2(-1.f, -1.f),
    [Dir2D_L]  = HMM_V2(-1.f,  0.f),
    [Dir2D_UL] = HMM_V2(-1.f,  1.f),
    [Dir2D_U]  = HMM_V2( 0.f,  1.f),
    [Dir2D_UR] = HMM_V2( 1.f,  1.f),
    [Dir2D_R]  = HMM_V2( 1.f,  0.f),
    [Dir2D_DR] = HMM_V2(-1.f,  1.f),
    [Dir2D_M]  = HMM_V2( 0.f,  0.f),
}; 
#endif

typedef struct SpriteAnimTreeDir SpriteAnimTreeDir; 
struct SpriteAnimTreeDir
{
    SpriteAnim anims[Dir2D_COUNT];
};

/////////////////////////////////////////////////////////
//~ API

function SpriteAnim SpriteAnimFromParams(M_Arena *arena, SA_State *sa_state, SpriteAnimParams *params);

inline_function SA_Slice SpriteAnimSliceFromTime(SpriteAnim *anim, f32 time);

inline_function SpriteAnimTreeDir 
SpriteAnimTreeDirMakeFromParams(M_Arena *arena, SA_State *sa_state, SpriteAnimParams params[Dir2D_COUNT]);

/////////////////////////////////////////////////////////
//~ Animation Params Data

#define SAP(name, duration_, ...) \
read_only global String8 name##_fp[] = { __VA_ARGS__ };                  \
read_only global SpriteAnimParams name = {                               \
.filepathsCount = ArrayCount(name##_fp),                                \
.duration = duration_,                                                  \
.filepaths = name##_fp,                                                 \
}

#define SAP_DIR_ARRAY(baseName) {         \
[Dir2D_D]  = baseName##_d,                       \
[Dir2D_DL] = baseName##_dl,                      \
[Dir2D_L]  = baseName##_l,                       \
[Dir2D_UL] = baseName##_ul,                      \
[Dir2D_U]  = baseName##_u,                       \
[Dir2D_UR] = baseName##_ur,                      \
[Dir2D_R]  = baseName##_r,                       \
[Dir2D_DR] = baseName##_dr,                      \
}

//- mage walks
#define SAP_MAGE "assets/sprites/mage/"
#define SAP_MAGE_ANIM_SPD 0.5f

SAP(mage_walk_d,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageD0.png"),
    S8LitComp(SAP_MAGE "mageD1.png"),
    S8LitComp(SAP_MAGE "mageD0.png"),
    S8LitComp(SAP_MAGE "mageD2.png"),
    );

SAP(mage_walk_dl,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageDL0.png"),
    S8LitComp(SAP_MAGE "mageDL1.png"),
    S8LitComp(SAP_MAGE "mageDL0.png"),
    S8LitComp(SAP_MAGE "mageDL2.png"),
    );

SAP(mage_walk_l,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageL0.png"),
    S8LitComp(SAP_MAGE "mageL1.png"),
    S8LitComp(SAP_MAGE "mageL0.png"),
    S8LitComp(SAP_MAGE "mageL2.png"),
    );

SAP(mage_walk_ul,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageUL0.png"),
    S8LitComp(SAP_MAGE "mageUL1.png"),
    S8LitComp(SAP_MAGE "mageUL0.png"),
    S8LitComp(SAP_MAGE "mageUL2.png"),
    );

SAP(mage_walk_u,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageU0.png"),
    S8LitComp(SAP_MAGE "mageU1.png"),
    S8LitComp(SAP_MAGE "mageU0.png"),
    S8LitComp(SAP_MAGE "mageU2.png"),
    );

SAP(mage_walk_ur,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageUR0.png"),
    S8LitComp(SAP_MAGE "mageUR1.png"),
    S8LitComp(SAP_MAGE "mageUR0.png"),
    S8LitComp(SAP_MAGE "mageUR2.png"),
    );

SAP(mage_walk_r,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageR0.png"),
    S8LitComp(SAP_MAGE "mageR1.png"),
    S8LitComp(SAP_MAGE "mageR0.png"),
    S8LitComp(SAP_MAGE "mageR2.png"),
    );

SAP(mage_walk_dr,
    SAP_MAGE_ANIM_SPD,
    S8LitComp(SAP_MAGE "mageDR0.png"),
    S8LitComp(SAP_MAGE "mageDR1.png"),
    S8LitComp(SAP_MAGE "mageDR0.png"),
    S8LitComp(SAP_MAGE "mageDR2.png"),
    );




/////////////////////////////////////////////////////////
//~ END OF FILE
#endif //SPRITE_ANIMATION_INCLUDE_H
