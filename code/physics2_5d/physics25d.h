#ifndef PHYSICS25D_INCLUDE_H
#define PHYSICS25D_INCLUDE_H

// Physics 2.5D
// TODO(gsp): fix player being able to push other non-static entities into static entities

//- Shapes

typedef u64 P2D_ShapeKind;
enum
{
    P2D_ShapeKind_Circle,
    P2D_ShapeKind_AABB,
    
    P2D_ShapeKind_COUNT
};

typedef struct P2D_Circle P2D_Circle;
struct P2D_Circle
{
    HMM_Vec2 pos;
    union
    {
        f32 radius;
        f32 r;
    };
    f32 __pad;
};

typedef struct P2D_AABB P2D_AABB;
struct P2D_AABB
{
    HMM_Vec2 min;
    HMM_Vec2 max;
};

//- Generic Shape
typedef struct P2D_Shape P2D_Shape;
struct P2D_Shape
{
    P2D_ShapeKind kind;
    u64 __pad;
    union
    {
        P2D_Circle circle;
        P2D_AABB   aabb;
    };
};

typedef struct P2D_ShapeRoundedAABB P2D_ShapeRoundedAABB;
struct P2D_ShapeRoundedAABB
{
    HMM_Vec2 min;
    HMM_Vec2 max;
    f32      r;   // radius at each corner
};

/////////////////////////////////////////////////////////
//~ Materials
typedef struct P2D_Material P2D_Material;
struct P2D_Material
{
    f32 density;
    f32 restitution;
};

//read_only global P2D_Material p25d_mat_rock   = {.density=0.6f, .restitution=0.1f};
read_only global P2D_Material p25d_mat_rock   = {.density=5.f, .restitution=0.1f};
//read_only global P2D_Material p25d_mat_wood   = {.density=0.3f, .restitution=0.2f};
read_only global P2D_Material p25d_mat_wood   = {.density=5.f, .restitution=0.2f};
read_only global P2D_Material p25d_mat_bouncy = {.density=0.3f, .restitution=0.8f};
read_only global P2D_Material p25d_mat_super  = {.density=0.3f, .restitution=0.95f};
read_only global P2D_Material p25d_mat_static = {.density=0.0f, .restitution=0.4f};

/////////////////////////////////////////////////////////
//~ 
typedef u32 P2D_RbID;

// TODO(gsp): trigger callback
#define P2D_TriggerCollideCallback(name) void name(struct P2D_Manifold *manifold)

typedef u32 P2D_RbMask;
enum
{
    P2D_RbMask_Null    = 0,
    P2D_RbMask_Trigger = (1 << 0),
};

typedef struct P2D_Rb P2D_Rb;
struct P2D_Rb
{
    P2D_Shape shape;
    HMM_Vec2 pos;
    HMM_Vec2 vel;
    HMM_Vec2 force;
    b32      gravityScale;
    f32      invMass;
    f32      mass;
    f32      restitution;
    P2D_RbMask mask;
    PAD32(1);
};

/////////////////////////////////////////////////////////
//~ Manifold/Hit
typedef struct P2D_Manifold P2D_Manifold;
struct P2D_Manifold
{
    P2D_Rb *A;
    P2D_Rb *B;
    
    u32 count;
    f32 depths[2];
    HMM_Vec2 contacts[2]; // contact points
    
    // NOTE(gsp): always points from A -> B
    HMM_Vec2 n;
    u32 __pad;
};

typedef struct P2D_ManifoldNode P2D_ManifoldNode;
struct P2D_ManifoldNode
{
    P2D_ManifoldNode *next;
    P2D_Manifold m;
};

typedef struct P2D_ManifoldList P2D_ManifoldList;
struct P2D_ManifoldList
{
    P2D_ManifoldNode *first;
    P2D_ManifoldNode *last;
    u32 count;
    u32 __pad;
};

/////////////////////////////////////////////////////////
//~ Physics state
read_only global P2D_RbID p2d_rb_id_invalid = (P2D_RbID)(0xFFFFFFFF);

typedef struct P2D_State P2D_State;
struct P2D_State
{
    f32 timeAcc;
    f32 __pad;
    
    HMM_Vec2 g;
    
    P2D_Rb *rbs;
    u32 rbsMax;
    u32 countRbs;
};

/////////////////////////////////////////////////////////
//~ General API
function void P2D_Init(M_Arena *permArena, P2D_State *state, u32 rbsMax);

// NOTE(gsp): return: alpha - use it to linear interpolate renderer
//            prevPos * alpha + prevPos * (1.0f - alpha)
function f32  P2D_Update(f32 dt);
function void P2D_FixedUpdate(f32 h);

/////////////////////////////////////////////////////////
//~ Rb
function P2D_RbID P2D_RbMake(P2D_Shape shape, HMM_Vec2 pos, P2D_Material mat, P2D_RbMask mask);
function P2D_Rb  *P2D_RbFromID(P2D_RbID id);

/////////////////////////////////////////////////////////
//~ Rb apply forces
function void P2D_RbApplyForce(P2D_RbID id, HMM_Vec2 force);

/////////////////////////////////////////////////////////
//~ Shape Creation
function P2D_Shape P2D_CircleMake(HMM_Vec2 pos, f32 radius);
function P2D_Shape P2D_AABBMake(HMM_Vec2 min, HMM_Vec2 max);

/////////////////////////////////////////////////////////
//~ Collision resolve
// NOTE(gsp): make sure to 0 out man->count
function void P2D_ResolveCollision(P2D_Rb *A, P2D_Rb *B, HMM_Vec2 normal);
function void P2D_PositionalCorrection(P2D_Rb *A, P2D_Rb *B, P2D_Manifold *man);

/////////////////////////////////////////////////////////
//~ Manifolds

// NOTE(gsp): make sure to 0 out man->count

function void P2D_CirclevsCircle(P2D_Circle A, P2D_Circle B, P2D_Manifold *man);
function void P2D_AABBvsAABB(P2D_AABB A, P2D_AABB B, P2D_Manifold *man);
function void P2D_CirclevsAABB(P2D_Circle A, P2D_AABB B, P2D_Manifold *man);

// Generic
function void P2D_ShapevsShape(P2D_Shape A, P2D_Shape B, P2D_Manifold *man);

/////////////////////////////////////////////////////////
//~ Intersection detection

// Same type
function b32 P2D_CircleCircleIntersection(P2D_Circle a, P2D_Circle b);
function b32 P2D_AABBAABBIntersection(P2D_AABB a, P2D_AABB b);

// Dif types
function b32 P2D_CircleAABBIntersection(P2D_Circle circle, P2D_AABB aabb);

// General
function b32 P2D_ShapeIntersection(P2D_Shape a, P2D_Shape b);

/////////////////////////////////////////////////////////
//~ Minkowski sum
function P2D_Circle P2D_MinkowskiAdd2Circle(P2D_Circle me, P2D_Circle other);
function P2D_AABB   P2D_MinkowskiAdd2AABB(P2D_AABB me, P2D_AABB other);

/////////////////////////////////////////////////////////
//~ Helpers
function HMM_Vec2 P2D_AABBHalfSize(P2D_AABB aabb);
inline_function f32 P2D_AreaFromShape(P2D_Shape shape);

/////////////////////////////////////////////////////////
//~ END OF FILE
#endif //PHYSICS25D_INCLUDE_H
