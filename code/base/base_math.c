// Original file taken from Ryan Fleury: https://github.com/ryanfleury

#ifndef BASE_MATH_INCLUDE_C
#define BASE_MATH_INCLUDE_C

// gsp: #defines to be compatible with my code base
// NOTE(gsp): make sure the #undef s match at the bottom
#define S8 i8
#define S16 i16
#define S32 i32
#define S64 i64
#define U8 u8
#define U16 u16
#define U32 u32
#define U64 u64
#define B8 b8
#define B16 b16
#define B32 b32
#define B64 b64
#define F32 f32
#define F64 f64

////////////////////////////////
//~ rjf: Scalar Ops

function F32
Mix1F32(F32 a, F32 b, F32 t)
{
    F32 c = a*(1-t) + b*t;
    return c;
}

////////////////////////////////
//~ rjf: Vector Ops

function Vec2F64
V2F64(F64 x, F64 y)
{
    Vec2F64 result = { x, y };
    return result;
}
function Vec2F64 Add2F64(Vec2F64 a, Vec2F64 b) { return V2F64(a.x+b.x, a.y+b.y); }
function Vec2F64 Sub2F64(Vec2F64 a, Vec2F64 b) { return V2F64(a.x-b.x, a.y-b.y); }
function Vec2F64 Mul2F64(Vec2F64 a, Vec2F64 b) { return V2F64(a.x*b.x, a.y*b.y); }
function Vec2F64 Div2F64(Vec2F64 a, Vec2F64 b) { return V2F64(a.x/b.x, a.y/b.y); }
function Vec2F64 Scale2F64(Vec2F64 a, F64 scale) { return V2F64(a.x*scale, a.y*scale); }
function F64 Dot2F64(Vec2F64 a, Vec2F64 b) { return (a.x*b.x + a.y*b.y); }
function F64 LengthSquared2F64(Vec2F64 v) { return Dot2F64(v, v); }
function F64 Length2F64(Vec2F64 v) { return SquareRootF64(LengthSquared2F64(v)); }
function Vec2F64 Normalize2F64(Vec2F64 v) { return Scale2F64(v, 1.f/Length2F64(v)); }
function Vec2F64 Mix2F64(Vec2F64 a, Vec2F64 b, F64 t) { return V2F64(a.x*(1-t) + b.x*t, a.y*(1-t) + b.y*t); }

function Vec2S32
V2S32(S32 x, S32 y)
{
    Vec2S32 result = { x, y };
    return result;
}

function Vec2S32 Add2S32(Vec2S32 a, Vec2S32 b) { return V2S32(a.x+b.x, a.y+b.y); }
function Vec2S32 Sub2S32(Vec2S32 a, Vec2S32 b) { return V2S32(a.x-b.x, a.y-b.y); }
function Vec2S32 Scale2S32(Vec2S32 a, U32 scale) { return V2S32((S32)(a.x*scale), (S32)(a.y*scale)); }

function Vec2S64
V2S64(S64 x, S64 y)
{
    Vec2S64 v;
    v.x = x;
    v.y = y;
    return v;
}

function Vec2S64 Add2S64(Vec2S64 a, Vec2S64 b) { return V2S64(a.x+b.x, a.y+b.y); }
function Vec2S64 Sub2S64(Vec2S64 a, Vec2S64 b) { return V2S64(a.x-b.x, a.y-b.y); }

function b32 V2F32IsZero(HMM_Vec2 v) { return ((v.x == 0.f) && (v.y == 0.f));}
function b32 V3F32IsZero(HMM_Vec3 v) { return ((v.x == 0.f) && (v.y == 0.f) && (v.z == 0.f));}

function Vec3F64
V3F64(F64 x, F64 y, F64 z)
{
    Vec3F64 result = { x, y, z };
    return result;
}
function Vec3F64 Add3F64(Vec3F64 a, Vec3F64 b) { return V3F64(a.x+b.x, a.y+b.y, a.z+b.z); }
function Vec3F64 Sub3F64(Vec3F64 a, Vec3F64 b) { return V3F64(a.x-b.x, a.y-b.y, a.z-b.z); }
function Vec3F64 Mul3F64(Vec3F64 a, Vec3F64 b) { return V3F64(a.x*b.x, a.y*b.y, a.z*b.z); }
function Vec3F64 Div3F64(Vec3F64 a, Vec3F64 b) { return V3F64(a.x/b.x, a.y/b.y, a.z/b.z); }
function Vec3F64 Scale3F64(Vec3F64 a, F64 scale) { return V3F64(a.x*scale, a.y*scale, a.z*scale); }
function F64 Dot3F64(Vec3F64 a, Vec3F64 b) { return (a.x*b.x + a.y*b.y + a.z*b.z); }
function F64 LengthSquared3F64(Vec3F64 v) { return Dot3F64(v, v); }
function F64 Length3F64(Vec3F64 v) { return SquareRootF64(LengthSquared3F64(v)); }
function Vec3F64 Normalize3F64(Vec3F64 v) { return Scale3F64(v, 1.f/Length3F64(v)); }
function Vec3F64 Mix3F64(Vec3F64 a, Vec3F64 b, F64 t) { return V3F64(a.x*(1-t) + b.x*t, a.y*(1-t) + b.y*t, a.z*(1-t) + b.z*t); }
function Vec3F64 Cross3F64(Vec3F64 a, Vec3F64 b) { return V3F64(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
function Vec3F64
Transform3F64(Vec3F64 v, Mat3x3F64 m)
{
    Vec3F64 result = { 0 };
    for (int i = 0; i < 3; i += 1)
    {
        result.elements[i] = (v.elements[0] * m.elements[0][i] +
                              v.elements[1] * m.elements[1][i] +
                              v.elements[2] * m.elements[2][i]);
    }
    return result;
}

function Vec3S32
V3S32(S32 x, S32 y, S32 z)
{
    Vec3S32 result = {x, y, z};
    return result;
}

function Vec3S64
V3S64(S64 x, S64 y, S64 z)
{
    Vec3S64 result = {x, y, z};
    return result;
}

function Vec4F64
V4F64(F64 x, F64 y, F64 z, F64 w)
{
    Vec4F64 result = { x, y, z, w };
    return result;
}
function Vec4F64 Add4F64(Vec4F64 a, Vec4F64 b) { return V4F64(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w); }
function Vec4F64 Sub4F64(Vec4F64 a, Vec4F64 b) { return V4F64(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w); }
function Vec4F64 Mul4F64(Vec4F64 a, Vec4F64 b) { return V4F64(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
function Vec4F64 Div4F64(Vec4F64 a, Vec4F64 b) { return V4F64(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w); }
function Vec4F64 Scale4F64(Vec4F64 a, F64 scale) { return V4F64(a.x*scale, a.y*scale, a.z*scale, a.w*scale); }
function F64 Dot4F64(Vec4F64 a, Vec4F64 b) { return (a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w); }
function F64 LengthSquared4F64(Vec4F64 v) { return Dot4F64(v, v); }
function F64 Length4F64(Vec4F64 v) { return SquareRootF64(LengthSquared4F64(v)); }
function Vec4F64 Normalize4F64(Vec4F64 v) { return Scale4F64(v, 1.f/Length4F64(v)); }
function Vec4F64 Mix4F64(Vec4F64 a, Vec4F64 b, F64 t) { return V4F64(a.x*(1-t) + b.x*t, a.y*(1-t) + b.y*t, a.z*(1-t) + b.z*t, a.w*(1-t) + b.w*t); }
function Vec4F64
Transform4F64(Vec4F64 v, Mat4x4F64 m)
{
    Vec4F64 result = {0};
    for(int i = 0; i < 4; i += 1)
    {
        result.elements[i] = (v.elements[0]*m.elements[0][i] +
                              v.elements[1]*m.elements[1][i] +
                              v.elements[2]*m.elements[2][i] +
                              v.elements[3]*m.elements[3][i]);
    }
    return result;
}

function Vec4S32
V4S32(S32 x, S32 y, S32 z, S32 w)
{
    Vec4S32 result = { x, y, z, w };
    return result;
}

function Vec4S64
V4S64(S64 x, S64 y, S64 z, S64 w)
{
    Vec4S64 result = { x, y, z, w };
    return result;
}

////////////////////////////////
//~ rjf: Range Ops

function Rng1F32 R1F32(F32 min, F32 max)
{
    Rng1F32 result = { min, max };
    if(result.max < result.min)
    {
        Swap(F32, result.min, result.max);
    }
    return result;
}
function Rng1F32 Pad1F32(Rng1F32 r, F32 x) { return R1F32(r.min-x, r.max+x); }
function F32 Center1F32(Rng1F32 r) { return (r.min + r.max)/2; }
function B32 Contains1F32(Rng1F32 r, F32 v) { return r.min <= v && v < r.max; }
function F32 Dim1F32(Rng1F32 r) { return AbsoluteValueF32(r.max - r.min); }
function Rng1F32 Union1F32(Rng1F32 a, Rng1F32 b) { return R1F32(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1F32 Intersection1F32(Rng1F32 a, Rng1F32 b) { return R1F32(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng1F64 R1F64(F64 min, F64 max)
{
    Rng1F64 result = { min, max };
    if(result.max < result.min)
    {
        Swap(F64, result.min, result.max);
    }
    return result;
}
function Rng1F64 Pad1F64(Rng1F64 r, F64 x) { return R1F64(r.min-x, r.max+x); }
function F64 Center1F64(Rng1F64 r) { return (r.min + r.max)/2; }
function B32 Contains1F64(Rng1F64 r, F64 v) { return r.min <= v && v < r.max; }
function F64 Dim1F64(Rng1F64 r) { return AbsoluteValueF64(r.max - r.min); }
function Rng1F64 Union1F64(Rng1F64 a, Rng1F64 b) { return R1F64(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1F64 Intersection1F64(Rng1F64 a, Rng1F64 b) { return R1F64(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng1S32 R1S32(S32 min, S32 max)
{
    Rng1S32 result = { min, max };
    if(result.max < result.min)
    {
        Swap(S32, result.min, result.max);
    }
    return result;
}
function Rng1S32 Pad1S32(Rng1S32 r, S32 x) { return R1S32(r.min-x, r.max+x); }
function S32 Center1S32(Rng1S32 r) { return (r.min + r.max)/2; }
function B32 Contains1S32(Rng1S32 r, S32 v) { return r.min <= v && v < r.max; }
function S32 Dim1S32(Rng1S32 r) { return AbsoluteValueS32(r.max - r.min); }
function Rng1S32 Union1S32(Rng1S32 a, Rng1S32 b) { return R1S32(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1S32 Intersection1S32(Rng1S32 a, Rng1S32 b) { return R1S32(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng1S64 R1S64(S64 min, S64 max)
{
    Rng1S64 result = { min, max };
    if(result.max < result.min)
    {
        Swap(S64, result.min, result.max);
    }
    return result;
}
function Rng1S64 Pad1S64(Rng1S64 r, S64 x) { return R1S64(r.min-x, r.max+x); }
function S64 Center1S64(Rng1S64 r) { return (r.min + r.max)/2; }
function B32 Contains1S64(Rng1S64 r, S64 v) { return r.min <= v && v < r.max; }
function S64 Dim1S64(Rng1S64 r) { return AbsoluteValueS64(r.max - r.min); }
function Rng1S64 Union1S64(Rng1S64 a, Rng1S64 b) { return R1S64(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1S64 Intersection1S64(Rng1S64 a, Rng1S64 b) { return R1S64(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng1U32 R1U32(U32 min, U32 max)
{
    Rng1U32 result = { min, max };
    if(result.max < result.min)
    {
        Swap(U32, result.min, result.max);
    }
    return result;
}
function Rng1U32 Pad1U32(Rng1U32 r, U32 x) { return R1U32(r.min-x, r.max+x); }
function U32 Center1U32(Rng1U32 r) { return (r.min + r.max)/2; }
function B32 Contains1U32(Rng1U32 r, U32 v) { return r.min <= v && v < r.max; }
function U32 Dim1U32(Rng1U32 r) { return (r.max - r.min); }
function Rng1U32 Union1U32(Rng1U32 a, Rng1U32 b) { return R1U32(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1U32 Intersection1U32(Rng1U32 a, Rng1U32 b) { return R1U32(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng1U64 R1U64(U64 min, U64 max)
{
    Rng1U64 result = { min, max };
    if(result.max < result.min)
    {
        Swap(U64, result.min, result.max);
    }
    return result;
}
function Rng1U64 Pad1U64(Rng1U64 r, U64 x) { return R1U64(r.min-x, r.max+x); }
function U64 Center1U64(Rng1U64 r) { return (r.min + r.max)/2; }
function B32 Contains1U64(Rng1U64 r, U64 v) { return r.min <= v && v < r.max; }
function U64 Dim1U64(Rng1U64 r) { return AbsoluteValueU64(r.max - r.min); }
function Rng1U64 Union1U64(Rng1U64 a, Rng1U64 b) { return R1U64(Min(a.min, b.min), Max(a.max, b.max)); }
function Rng1U64 Intersection1U64(Rng1U64 a, Rng1U64 b) { return R1U64(Max(a.min, b.min), Min(a.max, b.max)); }

function Rng2F32
R2F32(HMM_Vec2 min, HMM_Vec2 max)
{
    Rng2F32 result = { min, max };
    return result;
}
function Rng2F32 Shift2F32(Rng2F32 r, HMM_Vec2 v) { r.x0 += v.x; r.y0 += v.y; r.x1 += v.x; r.y1 += v.y; return r; }
function Rng2F32 Pad2F32(Rng2F32 r, F32 x) { return R2F32(HMM_Sub(r.min, HMM_V2(x, x)), HMM_Add(r.max, HMM_V2(x, x))); }
function HMM_Vec2 Center2F32(Rng2F32 r) { return HMM_V2((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2); }
function B32 Contains2F32(Rng2F32 r, HMM_Vec2 v) { return (r.min.x <= v.x && v.x <= r.max.x) && (r.min.y <= v.y && v.y <= r.max.y); }
function HMM_Vec2 Dim2F32(Rng2F32 r) { return HMM_V2(AbsoluteValueF32(r.max.x - r.min.x), AbsoluteValueF32(r.max.y - r.min.y)); }
function Rng2F32 Union2F32(Rng2F32 a, Rng2F32 b)
{
    return R2F32(HMM_V2(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)),
                 HMM_V2(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y)));
}
function Rng2F32 Intersection2F32(Rng2F32 a, Rng2F32 b)
{
    return R2F32(HMM_V2(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)),
                 HMM_V2(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y)));
}

function Rng2F64
R2F64(Vec2F64 min, Vec2F64 max)
{
    Rng2F64 result = { min, max };
    return result;
}
function Rng2F64 Shift2F64(Rng2F64 r, Vec2F64 v) { r.x0 += v.x; r.y0 += v.y; r.x1 += v.x; r.y1 += v.y; return r; }
function Rng2F64 Pad2F64(Rng2F64 r, F64 x) { return R2F64(Sub2F64(r.min, V2F64(x, x)), Add2F64(r.max, V2F64(x, x))); }
function Vec2F64 Center2F64(Rng2F64 r) { return V2F64((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2); }
function B32 Contains2F64(Rng2F64 r, Vec2F64 v) { return (r.min.x <= v.x && v.x <= r.max.x) && (r.min.y <= v.y && v.y <= r.max.y); }
function Vec2F64 Dim2F64(Rng2F64 r) { return V2F64(AbsoluteValueF64(r.max.x - r.min.x), AbsoluteValueF64(r.max.y - r.min.y)); }
function Rng2F64 Union2F64(Rng2F64 a, Rng2F64 b)
{
    return R2F64(V2F64(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)),
                 V2F64(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y)));
}
function Rng2F64 Intersection2F64(Rng2F64 a, Rng2F64 b)
{
    return R2F64(V2F64(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)),
                 V2F64(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y)));
}

function Rng2S32
R2S32(Vec2S32 min, Vec2S32 max)
{
    Rng2S32 result = { min, max };
    return result;
}
function Rng2S32 Pad2S32(Rng2S32 r, S32 x) { return R2S32(Sub2S32(r.min, V2S32(x, x)), Add2S32(r.max, V2S32(x, x))); }
function Vec2S32 Center2S32(Rng2S32 r) { return V2S32((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2); }
function B32 Contains2S32(Rng2S32 r, Vec2S32 v) { return (r.min.x <= v.x && v.x < r.max.x) && (r.min.y <= v.y && v.y < r.max.y); }
function Vec2S32 Dim2S32(Rng2S32 r) { return V2S32(AbsoluteValueS32(r.max.x - r.min.x), AbsoluteValueS32(r.max.y - r.min.y)); }
function Rng2S32 Union2S32(Rng2S32 a, Rng2S32 b)
{
    return R2S32(V2S32(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)),
                 V2S32(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y)));
}
function Rng2S32 Intersection2S32(Rng2S32 a, Rng2S32 b)
{
    return R2S32(V2S32(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)),
                 V2S32(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y)));
}

function Rng2S64
R2S64(Vec2S64 min, Vec2S64 max)
{
    Rng2S64 result = { min, max };
    return result;
}
function Rng2S64 Pad2S64(Rng2S64 r, S64 x) { return R2S64(Sub2S64(r.min, V2S64(x, x)), Add2S64(r.max, V2S64(x, x))); }
function Vec2S64 Center2S64(Rng2S64 r) { return V2S64((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2); }
function B32 Contains2S64(Rng2S64 r, Vec2S64 v) { return (r.min.x <= v.x && v.x < r.max.x) && (r.min.y <= v.y && v.y < r.max.y); }
function Vec2S64 Dim2S64(Rng2S64 r) { return V2S64(AbsoluteValueS64(r.max.x - r.min.x), AbsoluteValueS64(r.max.y - r.min.y)); }
function Rng2S64 Union2S64(Rng2S64 a, Rng2S64 b)
{
    return R2S64(V2S64(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)),
                 V2S64(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y)));
}
function Rng2S64 Intersection2S64(Rng2S64 a, Rng2S64 b)
{
    return R2S64(V2S64(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)),
                 V2S64(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y)));
}

function Rng3F32
R3F32(HMM_Vec3 min, HMM_Vec3 max)
{
    Rng3F32 result = { min, max };
    return result;
}
function Rng3F32 Shift3F32(Rng3F32 r, HMM_Vec3 v) { r.x0 += v.x; r.y0 += v.y; r.z0 += v.z; r.x1 += v.x; r.y1 += v.y; r.z1 += v.z; return r; }
function Rng3F32 Pad3F32(Rng3F32 r, F32 x) { return R3F32(HMM_Sub(r.min, HMM_V3(x, x, x)), HMM_Add(r.max, HMM_V3(x, x, x))); }
function HMM_Vec3 Center3F32(Rng3F32 r) { return HMM_V3((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2, (r.min.z + r.max.z)/2); }
function B32 Contains3F32(Rng3F32 r, HMM_Vec3 v) { return (r.min.x <= v.x && v.x <= r.max.x) && (r.min.y <= v.y && v.y <= r.max.y) && (r.min.z <= v.z && v.z <= r.max.z); }
function HMM_Vec3 Dim3F32(Rng3F32 r) { return HMM_V3(AbsoluteValueF32(r.max.x - r.min.x), AbsoluteValueF32(r.max.y - r.min.y), AbsoluteValueF32(r.max.z - r.min.z)); }
function Rng3F32 Union3F32(Rng3F32 a, Rng3F32 b)
{
    return R3F32(HMM_V3(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y), Min(a.min.z, b.min.z)),
                 HMM_V3(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y), Max(a.max.z, b.max.z)));
}
function Rng3F32 Intersection3F32(Rng3F32 a, Rng3F32 b)
{
    return R3F32(HMM_V3(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y), Max(a.min.z, b.min.z)),
                 HMM_V3(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y), Min(a.max.z, b.max.z)));
}

////////////////////////////////
//~ rjf: Lists

function void
Rng1U64ListPushNode(Rng1U64List *list, Rng1U64Node *node)
{
    QueuePush(list->first, list->last, node);
    list->count += 1;
    list->total_count += Dim1U64(node->v);
}

function void
Rng1U64ListPushNodeFront(Rng1U64List *list, Rng1U64Node *node)
{
    QueuePushFront(list->first, list->last, node);
    list->count += 1;
    list->total_count += Dim1U64(node->v);
}

#if 0
function void
Rng1U64ListPush(Arena *arena, Rng1U64List *list, Rng1U64 rng)
{
    Rng1U64Node *n = PushArray(arena, Rng1U64Node, 1);
    n->v = rng;
    Rng1U64ListPushNode(list, n);
}

function void
Rng1U64ListPushFront(Arena *arena, Rng1U64List *list, Rng1U64 rng)
{
    Rng1U64Node *n = PushArray(arena, Rng1U64Node, 1);
    n->v = rng;
    Rng1U64ListPushNodeFront(list, n);
}
#endif

////////////////////////////////
//~ rjf: Miscellaneous Ops

function HMM_Vec3
HSVFromRGB(HMM_Vec3 rgb)
{
    F32 r = rgb.r;
    F32 g = rgb.g;
    F32 b = rgb.b;
    F32 k = 0.f;
    if(g < b)
    {
        F32 swap = b;
        b= g;
        g = swap;
        k = -1.f;
    }
    if(r < g)
    {
        F32 swap = r;
        r = g;
        g = swap;
        k = -2.f / 6.f - k;
    }
    F32 chroma = r - (g < b ? g : b);
    HMM_Vec3 result;
    result.x = AbsoluteValueF32(k + (g - b) / (6.f * chroma + 1e-20f));
    result.y = chroma / (r + 1e-20f);
    result.z = r;
    return result;
}

function HMM_Vec3
RGBFromHSV(HMM_Vec3 hsv)
{
    HMM_Vec3 rgb = {0};
    
    if(hsv.y == 0.0f)
    {
        rgb.r = rgb.g = rgb.b = hsv.z;
    }
    else
    {
        F32 h = hsv.x;
        F32 s = hsv.y;
        F32 v = hsv.z;
        
        if(h >= 1.f)
        {
            h -= 10 * 1e-6f;
        }
        
        if(s >= 1.f)
        {
            s -= 10 * 1e-6f;
        }
        
        if(v >= 1.f)
        {
            v -= 10 * 1e-6f;
        }
        
        h = Mod(h, 1.f) / (60.f/360.f);
        int i = (int)h;
        F32 f = h - (F32)i;
        F32 p = v * (1.0f - s);
        F32 q = v * (1.0f - s * f);
        F32 t = v * (1.0f - s * (1.0f - f));
        
        switch (i)
        {
            case 0: { rgb.r = v; rgb.g = t; rgb.b = p; break; }
            case 1: { rgb.r = q; rgb.g = v; rgb.b = p; break; }
            case 2: { rgb.r = p; rgb.g = v; rgb.b = t; break; }
            case 3: { rgb.r = p; rgb.g = q; rgb.b = v; break; }
            case 4: { rgb.r = t; rgb.g = p; rgb.b = v; break; }
            case 5: { default: rgb.r = v; rgb.g = p; rgb.b = q; break; }
        }
    }
    
    return rgb;
}

function HMM_Vec4
HMM_Vec4FromHexRGBA(U32 hex)
{
    HMM_Vec4 result =
    {
        (F32)((hex & 0xff000000) >> 24) / 255.f,
        (F32)((hex & 0x00ff0000) >> 16) / 255.f,
        (F32)((hex & 0x0000ff00) >>  8) / 255.f,
        (F32)((hex & 0x000000ff) >>  0) / 255.f,
    };
    return result;
}

function F32
MillisecondsFromMicroseconds(U64 microseconds)
{
    F32 milliseconds = (F32)((F64)microseconds / 1000.0);
    return milliseconds;
}

function U64
MicrosecondsFromMilliseconds(F32 milliseconds)
{
    U64 microseconds = (U64)((F64)milliseconds * 1000.0);
    return microseconds;
}

function Vec2S64
SideVertexFromCorner(Corner corner)
{
    Vec2S64 result = {0};
    switch(corner)
    {
        default: break;
        case Corner_00:{result = V2S64(0, 0);}break;
        case Corner_01:{result = V2S64(0, 1);}break;
        case Corner_10:{result = V2S64(1, 0);}break;
        case Corner_11:{result = V2S64(1, 1);}break;
    }
    return result;
}





////////////////////////////////////////////////////////////////////////////////////////////
//~ gsp: Stuff I added

// gsp: added
function Vec2S32
SideVertexFromCorner32(Corner corner)
{
    Vec2S32 result = {0};
    switch(corner)
    {
        default: break;
        case Corner_00:{result = V2S32(0, 0);}break;
        case Corner_01:{result = V2S32(0, 1);}break;
        case Corner_10:{result = V2S32(1, 0);}break;
        case Corner_11:{result = V2S32(1, 1);}break;
    }
    return result;
}

//~ Cleanup defines
#undef S8
#undef S16
#undef S32
#undef S64
#undef U8
#undef U16
#undef U32
#undef U64
#undef B8
#undef B16
#undef B32
#undef B64
#undef F32
#undef F64


#endif // #ifndef BASE_MATH_INCLUDE_C