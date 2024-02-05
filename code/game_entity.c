/////////////////////////////////////////////////////
//~ Entity Creation
function G_Entity
G_EntityMakeDefault(void)
{
    G_Entity entity = {0};
    entity.generation = g_state->frameCount;
    entity.shade = HMM_V4(1.f, 1.f, 1.f, 1.f);
    entity.rot   = HMM_Q(0.f, 0.f, 0.f, 1.f);
    return entity;
}

function G_Entity *
G_EntityAdd(G_EntityFlag flags)
{
    G_Entity *entity = 0;
    
    // NOTE(gsp): find first dead entity
    for (u32 i = 0; i < g_state->entitiesCount; ++i)
    {
        G_Entity *currEntity = &g_state->entities[i];
        if (G_IsEntityDead(currEntity))
        {
            entity = currEntity;
        }
    }
    
    // NOTE(gsp): add entity if we don't have one already
    if (entity == 0)
    {
        Assert(G_MAX_ENTITIES > g_state->entitiesCount);
        entity = &g_state->entities[g_state->entitiesCount++];
        *entity = G_EntityMakeDefault();
        entity->flags = flags;
    }
    
    return entity;
}

function void      
G_PlayerEntityInit(HMM_Vec2 pos, SpriteAnimTreeDir *animDirTree)
{
    G_Entity *player = G_PlayerEntityGet();
    *player = G_EntityMakeDefault();
    
    player->flags = G_EntityFlag_PlayerControlled
        | G_EntityFlag_Physics
        | G_EntityFlag_RenderSprite3D
        | G_EntityFlag_TreeAnimated;
    
    
    // Animations
    player->currAnim = &animDirTree->anims[0];
    player->animDirTree = animDirTree;
    
    // Drawing
    player->shade = HMM_V4(1.f, 1.f, 1.f, 1.f);
    
    // Attributes
    player->mvSpd = 20.f;
    player->timeToReachTerminalSpd = 0.1f;
    
    // Physics
#if 1
    P2D_Shape shape = 
        P2D_CircleMake(pos, 1.5f);
#else
    HMM_Vec2 halfSize = HMM_V2(1.f, 1.f);
    P2D_Shape shape = P2D_AABBMake(HMM_V2(-halfSize.x, -halfSize.y), halfSize);
#endif
    
    player->rbID = P2D_RbMake(shape, pos, p25d_mat_rock, 0);
}

function void      
G_EnemyEntityInit(HMM_Vec2 pos, SpriteAnimTreeDir *animDirTree)
{
    G_Entity *entity = G_EntityAdd(  G_EntityFlag_Physics
                                   | G_EntityFlag_RenderSprite3D
                                   | G_EntityFlag_TreeAnimated);
    
    entity->currAnim = &animDirTree->anims[0];
    
    entity->shade = HMM_V4(1.f, 0.5f, 0.5f, 1.f);
    entity->mvSpd = 10.f;
    entity->animDirTree = animDirTree;
    
#if 1
    P2D_Shape shape = 
        P2D_CircleMake(pos, 1.5f);
#else
    HMM_Vec2 halfSize = HMM_V2(1.f, 1.f);
    P2D_Shape shape = P2D_AABBMake(HMM_V2(-halfSize.x, -halfSize.y), halfSize);
#endif
    
    entity->rbID = P2D_RbMake(shape, pos, p25d_mat_rock, 0);
}

function void
G_WallEntityAdd(HMM_Vec2 pos, HMM_Vec2 size, f32 height)
{
    G_Entity *entity = G_EntityAdd(G_EntityFlag_Physics|G_EntityFlag_RenderMesh|G_EntityFlag_Static);
    
    f32 sizeX;
    HMM_Vec2 halfSize;
    if (size.x > size.y)
    {
        sizeX = size.x;
        halfSize = HMM_V2(size.x, 0.2f);
    }
    else
    {
        sizeX = size.y;
        halfSize = HMM_V2(0.2f, size.y);
        entity->rot = HMM_QFromAxisAngle_RH(HMM_V3(0.f, 1.f, 0.f), RadFromDegF32(90.f));
    }
    
    P2D_Shape shape = P2D_AABBMake(HMM_Sub(pos, halfSize), HMM_Add(pos, halfSize));
    entity->rbID = P2D_RbMake(shape, pos, p25d_mat_static, 0);
    
    entity->size = HMM_V3(sizeX, height, 1.f);
    entity->mesh = g_state->wall;
}

function void
G_ProjectEntityAdd(SA_Slice sprite, HMM_Vec2 pos, HMM_Vec2 dir, f32 spd, f32 size)
{
    G_Entity *entity = G_EntityAdd(G_EntityFlag_Physics|G_EntityFlag_RenderSprite3D|G_EntityFlag_Projectile);
    
    P2D_Shape shape = P2D_CircleMake(pos, size);
    entity->mvSpd = spd;
    entity->rbID = P2D_RbMake(shape, pos, p25d_mat_rock, P2D_RbMask_Trigger);
    entity->staticSprite = sprite;
}

/////////////////////////////////////////////////////
//~ Basic API
function void
G_UpdateEntities(f32 dt)
{
    for (u32 i = 0; i < g_state->entitiesCount; ++i)
    {
        G_Entity *entity = &g_state->entities[i];
        G_UpdateEntity(entity, dt);
    }
}

function void 
G_RenderEntities(void)
{
    for (u32 i = 0; i < g_state->entitiesCount; ++i)
    {
        G_Entity *entity = &g_state->entities[i];
        G_RenderEntity(entity);
    }
}

inline_function b32
G_IsEntityAlive(G_Entity *entity)
{
    return !!(entity->generation != U64Max);
}

inline_function b32 
G_IsEntityDead(G_Entity *entity)
{
    return !(G_IsEntityAlive(entity));
}

function void
G_KillEntity(G_Entity *entity)
{
    entity->generation = U64Max;
}

function G_Entity *
G_PlayerEntityGet(void)
{
    return &g_state->entities[0];
}

inline_function HMM_Vec3
G_EntityGetPos3D(G_Entity *entity)
{
    P2D_Rb *rb = P2D_RbFromID(entity->rbID);
    return HMM_V3(rb->pos.x, 0.f, rb->pos.y);
}

/////////////////////////////////////////////////////
//~ Internal

function void
G_UpdateEntity(G_Entity *entity, f32 dt)
{
    if (G_IsEntityDead(entity))
        return;
    
    entity->generation++;
    entity->moveDir = V2Zero();
    
    if (G_EntityHasFlags(entity, G_EntityFlag_PlayerControlled))
    {
        Input *input = g_state->input;
        
        if (!V3F32IsZero(input->move))
        {
            Camera *cam = g_state->activeCam;
            
            HMM_Vec3 move3;
            move3   = HMM_Mul(cam->fwd, input->move.z);
            move3   = HMM_Add(HMM_Mul(cam->right, -input->move.x), move3);
            move3.y = 0.f;
            move3   = HMM_Norm(move3);
            
            HMM_Vec2 move = HMM_V2(move3.x, move3.z);
            
            entity->moveDir = move;
            entity->moveDirRelToCam = HMM_V2(input->move.x, input->move.z);
            entity->prevNonZeroMoveDir = entity->moveDir;
            
#if 0
            P2D_RbApplyForce(entity->rbID, HMM_MulV2F(entity->moveDir, 100.f * dt));
#endif
        }
        
        P2D_Rb *rb = P2D_RbFromID(entity->rbID);
        b32 moveDirIsZero = V2F32IsZero(entity->moveDir);
        
        if (entity->timeToReachTerminalSpd != 0)
        {
            entity->timeSpentAccelerating = moveDirIsZero ?
                Max(0.f, entity->timeSpentAccelerating - dt) :
            Min(entity->timeToReachTerminalSpd, entity->timeSpentAccelerating + dt);
            
            rb->vel = HMM_MulV2F(entity->prevNonZeroMoveDir, entity->timeSpentAccelerating / entity->timeToReachTerminalSpd);
        }
        else
        {
            if (!moveDirIsZero)
            {
                rb->vel = entity->moveDir;
            }
            else
            {
                rb->vel = V2Zero();
            }
        }
        
        rb->vel = HMM_MulV2F(rb->vel, entity->mvSpd);
    }
    
    if (G_EntityHasFlags(entity, G_EntityFlag_TreeAnimated))
    {
        HMM_Vec2 moveDir = entity->moveDirRelToCam;
        u32 animIndex = entity->animDirTreeLastIdx;
        if (moveDir.y > 0.f) // Dir2D_UX
        {
            animIndex = (moveDir.x > 0.f) ? Dir2D_UR : ((moveDir.x < 0.f) ? Dir2D_UL : Dir2D_U);
        }
        else if (moveDir.y < 0.f) // Dir2D_DX
        {
            animIndex = (moveDir.x > 0.f) ? Dir2D_DR : ((moveDir.x < 0.f) ? Dir2D_DL : Dir2D_D);
        }
        else // left or right
        {
            animIndex = (moveDir.x > 0.f) ? Dir2D_R : ((moveDir.x < 0.f) ? Dir2D_L : entity->animDirTreeLastIdx);
        }
        
        SpriteAnimTreeDir *animDirTree = entity->animDirTree;
        if (entity->animDirTreeLastIdx != animIndex)
        {
            entity->currAnimTime = 0.f;
        }
        entity->animDirTreeLastIdx = animIndex;
        entity->currAnim = &animDirTree->anims[animIndex];
        entity->currAnimTime += dt;
    }
}

function void
G_RenderEntity(G_Entity *entity)
{
    if (G_EntityHasFlags(entity, G_EntityFlag_RenderSprite3D))
    {
        if (entity->animDirTree)
        {
            SA_Slice slice  = SpriteAnimSliceFromTime(entity->currAnim, entity->currAnimTime);
            HMM_Vec3 pos = G_EntityGetPos3D(entity);
            D_Sprite3D(.pos = pos, .slice = slice, .color = entity->shade);
        }
        else
        {
            SA_Slice slice = entity->staticSprite;
            HMM_Vec3 pos = G_EntityGetPos3D(entity);
            D_Sprite3D(.pos = pos, .slice = slice, .color = entity->shade);
        }
    }
    else if (G_EntityHasFlags(entity, G_EntityFlag_RenderMesh))
    {
        P2D_Rb *rb = P2D_RbFromID(entity->rbID);
        HMM_Vec3 pos = HMM_V3(rb->pos.x, 0.f, rb->pos.y);
        D_Mesh(.mesh = entity->mesh, .size = entity->size, .pos = pos, .rot = entity->rot);
    }
    
#if BUILD_DEBUG
    if (g_state->showHitBoxes && G_EntityHasFlags(entity, G_EntityFlag_Physics))
    {
        P2D_Shape shape = P2D_RbFromID(entity->rbID)->shape;
        HMM_Vec4 color = HMM_V4(0.05f, 1.f, 0.05f, 1.f);
        
        P2D_Manifold manifold = {0};
        for (u32 i = 0; i < g_state->entitiesCount; ++i)
        {
            G_Entity *other = &g_state->entities[i];
            if (other == entity) 
                continue;
            
            P2D_ShapevsShape(shape, P2D_RbFromID(other->rbID)->shape, &manifold);
            if (manifold.count > 0)
            {
                color = HMM_V4(1.f, 0.05f, 0.05f, 1.f);
            }
        }
        
        
        switch (shape.kind)
        {
            case P2D_ShapeKind_Circle: D_DebugCircle(HMM_V3(shape.circle.pos.x, 0.25f, shape.circle.pos.y), shape.circle.radius, color); break;
            case P2D_ShapeKind_AABB:
            {
                P2D_AABB aabb = shape.aabb;
                HMM_Vec2 size2D = HMM_Sub(aabb.max, aabb.min);
                HMM_Vec2 pos2D  = HMM_DivV2F(HMM_Add(aabb.max, aabb.min), 2.f);
                D_Mesh(.mesh  = g_state->plane, 
                       .color = color, 
                       .size  = HMM_V3(size2D.x, 1.f, size2D.y), 
                       .pos   = HMM_V3(pos2D.x, 0.25f, pos2D.y));
                
                D_DebugCircle(HMM_V3(aabb.min.x, 0.3f, aabb.min.y),
                              0.25f, HMM_V4(222.f/255.f, 47.f/255.f, 204.f/255.f, 1.f));
                D_DebugCircle(HMM_V3(aabb.max.x, 0.3f, aabb.max.y),
                              0.25f, HMM_V4(97.f/255.f, 47.f/255.f, 222.f/255.f, 1.f));
            }break;
        }
    }
#endif
}

/////////////////////////////////////////////////////
//~ Flag Helpers
inline_function b32
G_EntityHasFlags(G_Entity *entity, G_EntityFlag flags)
{
    return !!(entity->flags & flags);
}
