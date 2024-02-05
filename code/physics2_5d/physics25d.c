global P2D_State *p2d_state;

function void
P2D_Init(M_Arena *permArena, P2D_State *state, u32 rbsMax)
{
    state->rbsMax = rbsMax;
    state->rbs = M_PushArray(permArena, P2D_Rb, rbsMax);
    
    p2d_state = state;
}

function f32
P2D_Update(f32 dt)
{
    // Consts
    const f32 fps = 120;
    const f32 h = 1.f / fps;
    
    p2d_state->timeAcc += dt;
    p2d_state->timeAcc = Min(p2d_state->timeAcc, h*4); // avoid spiral of death!!
    while(p2d_state->timeAcc > h)
    {
        P2D_FixedUpdate(h);
        p2d_state->timeAcc -= h;
    }
    
    f32 alpha = p2d_state->timeAcc / dt;
    return alpha;
}

function P2D_ManifoldList
P2D_BroadPhase(M_Arena *arena)
{
    P2D_ManifoldList manList = {0};
    
    // O(n^2) broad phase
    for (u32 aIdx = 0; aIdx < p2d_state->countRbs; ++aIdx)
    {
        P2D_Rb *A = &p2d_state->rbs[aIdx];
        for (u32 bIdx = aIdx + 1; bIdx < p2d_state->countRbs; ++bIdx)
        {
            P2D_Rb *B = &p2d_state->rbs[bIdx];
            
            if (A->invMass == 0 && B->invMass == 0) continue;
            
            // Narrow phase
            P2D_Manifold manifold = { .A = A, .B = B };
            P2D_ShapevsShape(A->shape, B->shape, &manifold);
            if (manifold.count > 0)
            {
                P2D_ManifoldNode *node = M_PushStructNoZero(arena, P2D_ManifoldNode);
                MemoryCopyStruct(&node->m, &manifold);
                QueuePush(manList.first, manList.last, node);
            }
        }
    }
    
    return manList;
}

function void
P2D_FixedUpdate(f32 h)
{
    ScratchArenaScope(scratch, 0, 0)
    {
        // Determine overlapping forces
        P2D_ManifoldList manifoldList = P2D_BroadPhase(scratch.arena);
        
        // Apply forces - game input
        HMM_Vec2 g = p2d_state->g;
        
        for (u32 i = 0; i < p2d_state->countRbs; ++i)
        {
            P2D_Rb *rb = &p2d_state->rbs[i];
            if (rb->mass == 0.f) continue;
            
            // Add gravity and apply newton's 2nd law
            rb->vel = HMM_Add(rb->vel, 
                              HMM_Add(HMM_MulV2F(rb->force, rb->invMass), g));
            
            // NOTE(gsp): fake friction
#if 0
            f32 velLenSqr = HMM_LenSqr(rb->vel);
            const f32 maxVel = 5.f;
            if (velLenSqr > HMM_SQUARE(maxVel))
            {
                rb->vel = HMM_MulV2F(rb->vel, maxVel / HMM_SQRTF(velLenSqr));
            }
            else if (rb->force.x == 0.f && rb->force.y == 0.f && (rb->vel.x != 0.f || rb->vel.y != 0.f))
            {
                rb->vel = HMM_Sub(rb->vel, HMM_MulV2F(rb->vel, 0.5f*h));
                
            }
#endif
        }
        
        // solve constraints
        // integrate forces
        for (P2D_ManifoldNode *node = manifoldList.first; node != 0; node = node->next)
        {
            P2D_Manifold manifold = node->m;
            if (manifold.count == 0) continue;
            
            P2D_Rb *A = manifold.A;
            P2D_Rb *B = manifold.B;
            
            if (A->mask & P2D_RbMask_Trigger) continue;
            if (B->mask & P2D_RbMask_Trigger) continue;
            
            P2D_ResolveCollision(A, B, manifold.n);
            P2D_PositionalCorrection(A, B, &manifold);
        }
        
        // integrate positions
        for (u32 i = 0; i < p2d_state->countRbs; ++i)
        {
            P2D_Rb *rb = &p2d_state->rbs[i];
            
            // Add gravity and apply newton's 2nd law
            rb->pos = HMM_Add(rb->pos, HMM_MulV2F(rb->vel, h));
            
            rb->force = HMM_V2(0.f, 0.f);
            rb->vel = HMM_V2(0.f, 0.f);
        }
        
        
    }
    
    //- Update shapes
    for (u32 i = 0; i < p2d_state->countRbs; ++i)
    {
        P2D_Rb *rb = &p2d_state->rbs[i];
        P2D_Shape *shape = &rb->shape;
        switch (shape->kind)
        {
            case P2D_ShapeKind_Circle:
            {
                P2D_Circle *circle = &shape->circle;
                circle->pos = rb->pos;
            }break;
            case P2D_ShapeKind_AABB:
            {
                P2D_AABB *aabb = &shape->aabb;
                HMM_Vec2 halfSize = HMM_MulV2F(HMM_Sub(aabb->max, aabb->min), 0.5f);
                HMM_Vec2 pos2D    = rb->pos;
                aabb->min = HMM_Sub(pos2D, halfSize);
                aabb->max = HMM_Add(pos2D, halfSize);
            }break;
        }
    }
    
}

/////////////////////////////////////////////////////////
//~ Rb creation
function P2D_RbID
P2D_RbMake(P2D_Shape shape, HMM_Vec2 pos, P2D_Material mat, P2D_RbMask mask)
{
    Assert(p2d_state->countRbs < p2d_state->rbsMax);
    if (p2d_state->countRbs >= p2d_state->rbsMax)
    {
        return p2d_rb_id_invalid;
    }
    
    f32 mass = P2D_AreaFromShape(shape) * mat.density;
    f32 invMass = (mass == 0.f) ? 0.f : (1.f/mass);
    
    P2D_Rb rb = {
        .shape   = shape,
        .pos     = pos,
        .invMass = invMass,
        .mass    = mass,
        .restitution = mat.restitution,
        .mask    = mask,
    };
    
    P2D_RbID rbID = p2d_state->countRbs;
    p2d_state->rbs[rbID] = rb;
    ++p2d_state->countRbs;
    
    return rbID;
}

function P2D_Rb *
P2D_RbFromID(P2D_RbID id)
{
    Assert(id != p2d_rb_id_invalid);
    return &p2d_state->rbs[id];
}

/////////////////////////////////////////////////////////
//~ Rb apply forces
function void 
P2D_RbApplyForce(P2D_RbID id, HMM_Vec2 force)
{
    P2D_Rb *rb = P2D_RbFromID(id);
    rb->force = HMM_Add(rb->force, force);
}

/////////////////////////////////////////////////////////
//~ Shape Creation

function P2D_Shape
P2D_CircleMake(HMM_Vec2 pos, f32 radius)
{
    P2D_Shape shape = {
        .kind = P2D_ShapeKind_Circle,
        .circle = {
            .pos = pos,
            .radius = radius,
        },
    };
    return shape;
}

function P2D_Shape
P2D_AABBMake(HMM_Vec2 min, HMM_Vec2 max)
{
    P2D_Shape shape = {
        .kind = P2D_ShapeKind_AABB,
        .aabb = {
            .min = min,
            .max = max,
        },
    };
    return shape;
}

/////////////////////////////////////////////////////////
//~ 
function void 
P2D_ResolveCollision(P2D_Rb *A, P2D_Rb *B, HMM_Vec2 normal)
{
    // Calculate relative velocity 
    HMM_Vec2 rv = HMM_Sub(B->vel, A->vel);
    
    // Calculate relative velocity in terms of the normal direction 
    f32 velAlongNormal = HMM_Dot(rv, normal);
    
    // Do not resolve if velocities are separating 
    if(velAlongNormal > 0)
        return;
    
    // Calculate restitution 
    float e = Min(A->restitution, B->restitution);
    
    // Calculate impulse scalar 
    f32 j = -(1.f + e) * velAlongNormal;
    j /= (A->invMass + B->invMass);
    
    // Apply impulse 
    HMM_Vec2 impulse = HMM_MulV2F(normal, j);
    
    A->vel = HMM_Sub(A->vel, HMM_MulV2F(impulse, A->invMass));
    B->vel = HMM_Add(B->vel, HMM_MulV2F(impulse, B->invMass));
}

function void
P2D_PositionalCorrection(P2D_Rb *A, P2D_Rb *B, P2D_Manifold *man)
{
#if 1
    const f32 percent = 0.0001f; // usually 20% to 80% 
    const f32 slop = 0.0001f;   // usually 0.01 to 0.1
#else
    const f32 percent = 0.2f; // usually 20% to 80% 
    const f32 slop = 0.05f;   // usually 0.01 to 0.1
#endif
    
    f32 coeff = Max(man->depths[0] - slop, 0.f) * A->mass * B->mass * percent;
    HMM_Vec2 correction = HMM_Mul(man->n, coeff);
    
    A->pos = HMM_Sub(A->pos, HMM_Mul(correction, A->invMass));
    B->pos = HMM_Add(B->pos, HMM_Mul(correction, B->invMass));
}

/////////////////////////////////////////////////////////
//~ Manifolds
function void
P2D_CirclevsCircle(P2D_Circle A, P2D_Circle B, P2D_Manifold *man)
{
    HMM_Vec2 d = HMM_Sub(B.pos, A.pos);
    f32 d2 = HMM_LenSqr(d);
    f32 r2 = A.r + B.r;
    r2 *= r2;
    if (d2 < r2)
    {
        f32 l = HMM_SQRTF(d2);
        // NOTE(gsp): when l is 0, just resolve in random chosen
        HMM_Vec2 n = (l != 0) ? HMM_MulV2F(d, 1.f / l) : HMM_V2(0.f, 1.f);
        
        man->count       = 1;
        man->depths[0]   = r2 - l;
        man->contacts[0] = HMM_Sub(B.pos, HMM_MulV2F(n, B.r));
        man->n           = n;
    }
}

function void
P2D_AABBvsAABB(P2D_AABB A, P2D_AABB B, P2D_Manifold *man)
{
    HMM_Vec2 midA = HMM_MulV2F(HMM_Add(A.min, A.max), 0.5f);
    HMM_Vec2 midB = HMM_MulV2F(HMM_Add(B.min, B.max), 0.5f);
    // half extents
    HMM_Vec2 eha  = HMM_AbsV2(HMM_MulV2F(HMM_Sub(A.max, A.min), 0.5f));
    HMM_Vec2 ehb  = HMM_AbsV2(HMM_MulV2F(HMM_Sub(B.max, B.min), 0.5f));
    HMM_Vec2 d    = HMM_Sub(midB, midA);
    
    // SAT
    f32 overlapX = eha.x + ehb.x - AbsoluteValueF32(d.x);
	if (overlapX < 0) return;
	f32 overlapY = eha.y + ehb.y - AbsoluteValueF32(d.y);
	if (overlapY < 0) return;
    
    
    // Manifold calc
    HMM_Vec2 n;
    HMM_Vec2 p;
    f32 depth;
    
    if (overlapX < overlapY) // X is axis of least penetration
    {
        depth = overlapX;
        if (d.x < 0.f)
        {
            LogInfo(0, "X: left");
            n = HMM_V2(-1.f, 0.f);
            p = HMM_Sub(midA, HMM_V2(eha.x, 0.f));
        }
        else
        {
            LogInfo(0, "X: right");
            n = HMM_V2(1.f, 0.f);
            p = HMM_Add(midA, HMM_V2(eha.x, 0.f));
        }
    }
    else // Y is axis of least penetration
    {
        depth = overlapY;
        if (d.y < 0.f)
        {
            LogInfo(0, "Y: down");
            n = HMM_V2(0.f, -1.f);
            p = HMM_Sub(midA, HMM_V2(0.f, eha.y));
        }
        else
        {
            LogInfo(0, "Y: up");
            n = HMM_V2(0.f, 1.f);
            p = HMM_Add(midA, HMM_V2(0.f, eha.y));
        }
    }
    
    man->count = 1;
    man->contacts[0] = p;
    man->depths[0]   = depth;
    man->n = n;
}

function void
P2D_CirclevsAABB(P2D_Circle A, P2D_AABB B, P2D_Manifold *man)
{
    HMM_Vec2 closest = HMM_ClampV2(B.min, A.pos, B.max);
    HMM_Vec2 ab = HMM_Sub(closest, A.pos);
    f32 d2 = HMM_Dot(ab, ab);
    f32 r2 = A.r * A.r;
    if (d2 < r2)
    {
        man->count = 1;
        
        if (d2 != 0) // Center of circle NOT inside AABB
        {
            f32 d = HMM_SQRTF(d2);
            man->n = HMM_Norm(ab);
            man->depths[0] = A.r - d;
            man->contacts[0] = HMM_Add(A.pos, HMM_MulV2F(man->n, d));
        }
        else // Center of circle inside AABB
        {
            HMM_Vec2 mid  = HMM_Mul(HMM_Add(B.max, B.min), 0.5f);
            HMM_Vec2 ehb  = HMM_Mul(HMM_Sub(B.max, B.min), 0.5f);
            HMM_Vec2 d    = HMM_Sub(A.pos, mid);
            HMM_Vec2 absD = HMM_AbsV2(d);
            
            f32 overlapX = ehb.x - absD.x;
            f32 overlapY = ehb.y - absD.y;
            
            f32 depth;
            HMM_Vec2 n;
            if (overlapX < overlapY)
            {
                depth = overlapX;
                n = HMM_V2(d.x < 0.f ? 1.f : -1.f, 0.f);
            }
            else
            {
                depth = overlapY;
                n = HMM_V2(0.f, d.y < 0.f ? 1.f : -1.f);
            }
            
            man->n = n;
            man->depths[0] = depth;
            man->contacts[0] = HMM_Sub(A.pos, HMM_MulV2F(n, depth));
        }
    }
}

function void
P2D_ShapevsShape(P2D_Shape A, P2D_Shape B, P2D_Manifold *m)
{
    switch(A.kind)
    {
        
        //-
        case P2D_ShapeKind_Circle:
        {
            switch(B.kind)
            {
                case P2D_ShapeKind_Circle: 
                P2D_CirclevsCircle(A.circle, B.circle, m); break;
                case P2D_ShapeKind_AABB:
                {
                    P2D_CirclevsAABB(A.circle, B.aabb, m);
                }break;
            }
        }break;
        
        //-
        case P2D_ShapeKind_AABB:
        {
            switch(B.kind)
            {
                // NOTE(gsp): @intersection_flipped
                case P2D_ShapeKind_Circle:
                {
                    P2D_CirclevsAABB(B.circle, A.aabb, m);
                    m->n = HMM_V2(-m->n.x, -m->n.y);
                }break;
                case P2D_ShapeKind_AABB:   
                P2D_AABBvsAABB(A.aabb, B.aabb, m); break;
            }
        }break;
        
        default: AssertMsg(0, "Invalid shape kind."); break;
    }
}

/////////////////////////////////////////////////////////
//~ Intersection detection

function b32
P2D_CircleCircleIntersection(P2D_Circle a, P2D_Circle b)
{
    f32 distSq = HMM_LenSqr(HMM_Sub(a.pos, b.pos));
    f32 radiusSq = (a.r+b.r)*(a.r+b.r);
    return (distSq < radiusSq);
}

function b32
P2D_AABBAABBIntersection(P2D_AABB a, P2D_AABB b)
{
    if((a.max.x < b.min.x) || (a.min.x > b.max.x)) return 0;
    if((a.max.y < b.min.y) || (a.min.y > b.max.y)) return 0;
    return 1;
}

function b32
P2D_CircleAABBIntersection(P2D_Circle circle, P2D_AABB aabb)
{
    HMM_Vec2 aabbHalfSize = HMM_DivV2F(HMM_Sub(aabb.max, aabb.min), 2.f);
    HMM_Vec2 aabbCenter   = HMM_Add(aabb.min, aabbHalfSize);
    
    HMM_Vec2 D = HMM_Sub(circle.pos, aabbCenter);
    
    HMM_Vec2 closest = aabbCenter;
    closest.x += Clamp(-aabbHalfSize.x, D.x, aabbHalfSize.x);
    closest.y += Clamp(-aabbHalfSize.y, D.y, aabbHalfSize.y);
    
    D = HMM_Sub(closest, circle.pos);
    b32 result = HMM_LenSqr(D) < HMM_SQUARE(circle.r);
    return result;
}

function b32
P2D_ShapeIntersection(P2D_Shape a, P2D_Shape b)
{
    b32 result = 0;
    switch(a.kind)
    {
        
        //-
        case P2D_ShapeKind_Circle:
        {
            switch(b.kind)
            {
                case P2D_ShapeKind_Circle: result = P2D_CircleCircleIntersection(a.circle, b.circle); break;
                case P2D_ShapeKind_AABB:   result = P2D_CircleAABBIntersection(a.circle, b.aabb);     break;
            }
        }break;
        
        //-
        case P2D_ShapeKind_AABB:
        {
            switch(b.kind)
            {
                // NOTE(gsp): @intersection_flipped
                case P2D_ShapeKind_Circle: result = P2D_CircleAABBIntersection(b.circle, a.aabb); break;
                case P2D_ShapeKind_AABB:   result = P2D_AABBAABBIntersection(a.aabb, b.aabb);     break;
            }
        }break;
        
        default: AssertMsg(0, "Invalid shape kind."); break;
    }
    return result;
}

/////////////////////////////////////////////////////////
//~ Minkowski sum
function P2D_Circle
P2D_MinkowskiAdd2Circle(P2D_Circle me, P2D_Circle other)
{
    P2D_Circle result = {
        .pos = other.pos,
        .radius = other.radius + me.radius,
    };
    return result;
}

function P2D_AABB
P2D_MinkowskiAdd2AABB(P2D_AABB me, P2D_AABB other)
{
    HMM_Vec2 meHalfSize = P2D_AABBHalfSize(me);
    P2D_AABB result = {
        .min = HMM_Sub(other.min, meHalfSize),
        .max = HMM_Add(other.max, meHalfSize),
    };
    return result;
}

/////////////////////////////////////////////////////////
//~ Helpers

function HMM_Vec2
P2D_AABBHalfSize(P2D_AABB aabb)
{
    HMM_Vec2 result = HMM_DivV2F(HMM_Sub(aabb.max, aabb.min), 2.f);
    return result;
}

inline_function f32
P2D_AreaFromShape(P2D_Shape shape)
{
    f32 result = 0.f;
    switch(shape.kind)
    {
        case P2D_ShapeKind_Circle:
        {
            result = HMM_SQUARE(shape.circle.r) * PiF32;
        }break;
        case P2D_ShapeKind_AABB:
        {
            HMM_Vec2 size = HMM_Sub(shape.aabb.max, shape.aabb.min);
            result = size.x * size.y;
        }break;
    }
    return result;
}

