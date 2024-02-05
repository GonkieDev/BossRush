#ifndef GAME_ENTITY_INCLUDE_H
#define GAME_ENTITY_INCLUDE_H

// TODO(gsp): change to entity ID instead of pointer and make getter function

typedef u64 G_EntityFlag;
enum
{
    G_EntityFlag_None,
    
    G_EntityFlag_PlayerControlled = (1 << 0),
    G_EntityFlag_Physics          = (1 << 2),
    G_EntityFlag_Static           = (1 << 3),
    G_EntityFlag_RenderSprite3D   = (1 << 4),
    G_EntityFlag_TreeAnimated     = (1 << 5),
    G_EntityFlag_RenderMesh       = (1 << 6),
    G_EntityFlag_Projectile       = (1 << 7),
    
    G_EntityFlag_COUNT
};

#if COMPILER_MSVC
#pragma warning(disable: 4820)
#endif

typedef struct G_Entity G_Entity;
struct G_Entity
{
    G_EntityFlag flags;
    u64 generation;
    
    // Animation stuff
    SpriteAnimTreeDir *animDirTree;
    SpriteAnim *currAnim;
    f32 currAnimTime;
    u32 animDirTreeLastIdx;
    SA_Slice staticSprite;
    
    // Attributes
    f32 mvSpd;
    f32 timeToReachTerminalSpd;
    f32 timeSpentAccelerating;
    
    //
    HMM_Vec2 moveDir;
    HMM_Vec2 prevNonZeroMoveDir;
    // NOTE(gsp): never non zero and used for animation
    HMM_Vec2 moveDirRelToCam; 
    
    // Physics stuff
    P2D_RbID rbID;
    
    // Mesh stuff
    R_MeshKey mesh;
    HMM_Quat rot;
    
    // Drawing stuff
    HMM_Vec3 size;
    HMM_Vec4 shade;
    
};

#if COMPILER_MSVC
#pragma warning(default: 4820)
#endif


StaticAssert(sizeof(G_Entity) % 16 == 0, "");


/////////////////////////////////////////////////////
//~ Entity Creation
function G_Entity  G_EntityMakeDefault(void);
function G_Entity *G_EntityAdd(G_EntityFlag flags);

function void G_PlayerEntityInit(HMM_Vec2 pos, SpriteAnimTreeDir *animDirTree);
function void G_EnemyEntityInit(HMM_Vec2 pos, SpriteAnimTreeDir *animDirTree);
function void G_WallEntityAdd(HMM_Vec2 pos, HMM_Vec2 size, f32 height);
function void G_ProjectEntityAdd(SA_Slice sprite, HMM_Vec2 pos, HMM_Vec2 dir, f32 spd, f32 size);

/////////////////////////////////////////////////////
//~ Basic API
function        void      G_UpdateEntities(f32 dt);
function        void      G_RenderEntities(void);

inline_function b32       G_IsEntityAlive(G_Entity *entity);
inline_function b32       G_IsEntityDead(G_Entity *entity);
function        void      G_KillEntity(G_Entity *entity);

function        G_Entity *G_PlayerEntityGet(void);
inline_function HMM_Vec3  G_EntityGetPos3D(G_Entity *entity);

/////////////////////////////////////////////////////
//~ Internal
function void G_UpdateEntity(G_Entity *entity, f32 dt);
function void G_RenderEntity(G_Entity *entity);

/////////////////////////////////////////////////////
//~ Flag Helpers
inline_function b32 G_EntityHasFlags(G_Entity *entity, G_EntityFlag flags);

/////////////////////////////////////////////////////
//~ END OF FILE
#endif //GAME_ENTITY_INCLUDE_H
