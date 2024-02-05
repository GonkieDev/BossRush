/* date = July 8th 2023 8:43 pm */

// Original file taken from Ryan Fleury: https://github.com/ryanfleury
// Modified by gsp

#ifndef BASE_MATH_INCLUDE_H
#define BASE_MATH_INCLUDE_H

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


///////////////////////////////////////////////////////////////////////////////////////////
//~ Ryan's math librarya



////////////////////////////////
//~ rjf: Macros

//- rjf: floating point operations

#define FloorF32(f)        floorf(f)
#define CeilF32(f)         ceilf(f)
#define RoundF32(f)        roundf(f)
#define ModF32(x, y)       fmodf(x, y)
#define DegFromRadF32(v)   ((180.f/PiF32) * (v))
#define RadFromDegF32(v)   ((PiF32/180.f) * (v))
#define RadsF32(v)         RadFromDegF32(v)
#define TurnsFromDegF32(v) ((v) / 360.f)
#define TurnsFromRadF32(v) ((v) / (2*PiF32))
#define DegFromTurnsF32(v) ((v) * 360.f)
#define RadFromTurnsF32(v) ((v) * (2*PiF32))
#define SquareRootF32(x)   sqrtf(x)
#define SinF32(v)          sinf(RadFromTurnsF32(v))
#define CosF32(v)          cosf(RadFromTurnsF32(v))
#define TanF32(v)          tanf(RadFromTurnsF32(v))
#define Sin2F32(v)         PowF32(Sin(v), 2)
#define Cos2F32(v)         PowF32(Cos(v), 2)
#define PowF32(b, exp)     powf(b, exp)
#define Log10F32(v)        log10f(v)
#define LogEF32(v)         logf(v)

#define FloorF64(f)        floor(f)
#define CeilF64(f)         ceil(f)
#define RoundF64(f)        round(f)
#define ModF64(x, y)       fmod(x, y)
#define DegFromRadF64(v)   ((180.0/PiF64) * (v))
#define RadFromDegF64(v)   ((PiF64/180.0) * (v))
#define TurnsFromDegF64(v) ((v) / 360.0)
#define TurnsFromRadF64(v) ((v) / (2*PiF64))
#define DegFromTurnsF64(v) ((v) * 360.0)
#define RadFromTurnsF64(v) ((v) * (2*PiF64))
#define SquareRootF64(x)   sqrt(x)
#define SinF64(v)          sin(RadFromTurnsF64(v))
#define CosF64(v)          cos(RadFromTurnsF64(v))
#define TanF64(v)          tan(RadFromTurnsF64(v))
#define Sin2F64(v)         PowF64(Sin(v), 2)
#define Cos2F64(v)         PowF64(Cos(v), 2)
#define PowF64(b, exp)     pow(b, exp)
#define Log10F64(v)        log10(v)
#define LogEF64(v)         log(v)

//- rjf: omitted base type aliases

#define Floor(f)           FloorF32(f)
#define Ceil(f)            CeilF32(f)
#define Mod(x, y)          ModF32(x, y)
#define DegFromRad(v)      DegFromRadF32(v)
#define RadFromDeg(v)      RadFromDegF32(v)
#define TurnsFromDeg(v)    TurnsFromDegF32(v)
#define TurnsFromRad(v)    TurnsFromRadF32(v)
#define DegFromTurns(v)    ((v) * 360.0)
#define RadFromTurns(v)    ((v) * (2*PiF64))
#define SquareRoot(x)      SquareRootF32(x)
#define Sin(v)             SinF32(v)
#define Cos(v)             CosF32(v)
#define Tan(v)             TanF32(v)
#define Sin2(v)            Sin2F32(v)
#define Cos2(v)            Cos2F32(v)
#define Pow(b, exp)        PowF32(b, exp)
#define Log10(v)           Log10F32(v)
#define LogE(v)            LogEF32(v)

////////////////////////////////
//~ rjf: Constants

read_only global F32 InfinityF32 = INFINITY;

read_only global F32 PiF32 = 3.1415926535897f;
read_only global F32 OneOverSquareRootOfTwoPiF32 = 0.3989422804f;
read_only global F32 EulersNumberF32 = 2.7182818284590452353602874713527f;

read_only global F64 PiF64 = 3.1415926535897;
read_only global F64 OneOverSquareRootOfTwoPiF64 = 0.3989422804;
read_only global F64 EulersNumberF64 = 2.7182818284590452353602874713527;

////////////////////////////////
//~ rjf: Vector Types

//- rjf: 2-vectors
typedef union Vec2F64 Vec2F64;
union Vec2F64
{
 struct
 {
  F64 x;
  F64 y;
 };
 F64 elements[2];
 F64 v[2];
};

typedef union Vec2S32 Vec2S32;
union Vec2S32
{
 struct
 {
  S32 x;
  S32 y;
 };
 S32 elements[2];
 S32 v[2];
};

typedef union Vec2S64 Vec2S64;
union Vec2S64
{
 struct
 {
  S64 x;
  S64 y;
 };
 S64 elements[2];
 S64 v[2];
};

//- rjf: 3-vectors
typedef union Vec3F64 Vec3F64;
union Vec3F64
{
 struct
 {
  F64 x;
  F64 y;
  F64 z;
 };
 
 struct
 {
  F64 r;
  F64 g;
  F64 b;
 };
 
 struct
 {
  F64 red;
  F64 green;
  F64 blue;
 };
 
 struct
 {
  Vec2F64 xy;
  F64 _z1;
 };
 
 struct
 {
  F64 _x1;
  Vec2F64 yz;
 };
 
 F64 elements[3];
 F64 v[3];
};

typedef union Vec3S32 Vec3S32;
union Vec3S32
{
 struct
 {
  S32 x;
  S32 y;
  S32 z;
 };
 
 struct
 {
  S32 r;
  S32 g;
  S32 b;
 };
 
 S32 elements[3];
 S32 v[3];
};

typedef union Vec3S64 Vec3S64;
union Vec3S64
{
 struct
 {
  S64 x;
  S64 y;
  S64 z;
 };
 
 struct
 {
  S64 r;
  S64 g;
  S64 b;
 };
 
 S64 elements[3];
 S64 v[3];
};

//-
typedef union Vec4F64 Vec4F64;
union Vec4F64
{
 struct
 {
  F64 x;
  F64 y;
  F64 z;
  F64 w;
 };
 
 struct
 {
  Vec2F64 xy;
  Vec2F64 zw;
 };
 
 struct
 {
  Vec3F64 xyz;
  F64 _w1;
 };
 
 struct
 {
  F64 _x1;
  Vec3F64 yzw;
 };
 
 struct
 {
  Vec3F64 rgb;
  F64 _w2;
 };
 
 struct
 {
  Vec3F64 gba;
  F64 _w3;
 };
 
 struct
 {
  F64 r;
  F64 g;
  F64 b;
  F64 a;
 };
 
 struct
 {
  F64 red;
  F64 green;
  F64 blue;
  F64 alpha;
 };
 
 struct
 {
  F64 left;
  F64 up;
  F64 right;
  F64 down;
 };
 
 F64 elements[4];
 F64 v[4];
 struct
 {
  F64 padding_[2];
  F64 dim[2];
 };
};

typedef union Vec4S32 Vec4S32;
union Vec4S32
{
 struct
 {
  S32 x;
  S32 y;
  S32 z;
  S32 w;
 };
 
 struct
 {
  Vec2S32 xy;
  Vec2S32 zw;
 };
 
 struct
 {
  Vec3S32 xyz;
  S32 _w1;
 };
 
 struct
 {
  S32 _x1;
  Vec3S32 yzw;
 };
 
 struct
 {
  Vec3S32 rgb;
  S32 _w2;
 };
 
 struct
 {
  Vec3S32 gba;
  S32 _w3;
 };
 
 struct
 {
  S32 r;
  S32 g;
  S32 b;
  S32 a;
 };
 
 struct
 {
  S32 red;
  S32 green;
  S32 blue;
  S32 alpha;
 };
 
 struct
 {
  S32 left;
  S32 up;
  S32 right;
  S32 down;
 };
 
 struct
 {
  S32 padding_[2];
  S32 dim[2];
 };
 
 S32 elements[4];
 S32 v[4];
};

typedef union Vec4S64 Vec4S64;
union Vec4S64
{
 struct
 {
  S64 x;
  S64 y;
  S64 z;
  S64 w;
 };
 
 struct
 {
  Vec2S64 xy;
  Vec2S64 zw;
 };
 
 struct
 {
  Vec3S64 xyz;
  S64 _w1;
 };
 
 struct
 {
  S64 _x1;
  Vec3S64 yzw;
 };
 
 struct
 {
  Vec3S64 rgb;
  S64 _w2;
 };
 
 struct
 {
  Vec3S64 gba;
  S64 _w3;
 };
 
 struct
 {
  S64 r;
  S64 g;
  S64 b;
  S64 a;
 };
 
 struct
 {
  S64 red;
  S64 green;
  S64 blue;
  S64 alpha;
 };
 
 struct
 {
  S64 left;
  S64 up;
  S64 right;
  S64 down;
 };
 
 struct
 {
  S64 padding_[2];
  S64 dim[2];
 };
 
 S64 elements[4];
 S64 v[4];
};

#define Vec2FromVec(v) HMM_V2((f32)(v).x, (f32)(v).y)
#define Vec2F64FromVec(v) V2F64((f64)(v).x, (f64)(v).y)
#define Vec2S32FromVec(v) V2S32((i32)(v).x, (i32)(v).y)
#define Vec2S64FromVec(v) V2S64((i64)(v).x, (i64)(v).y)
#define Vec3FromVec(v) HMM_V3((f32)(v).x, (f32)(v).y, (f32)(v).z)
#define Vec3F64FromVec(v) V3F64((f64)(v).x, (f64)(v).y, (f64)(v).z)
#define Vec3S32FromVec(v) V3S32((i32)(v).x, (i32)(v).y, (i32)(v).z)
#define Vec3S64FromVec(v) V3S64((i64)(v).x, (i64)(v).y, (i64)(v).z)
#define Vec4FromVec(v) HMM_V4((f32)(v).x, (f32)(v).y, (f32)(v).z, (f32)(v).w)
#define Vec4F64FromVec(v) V4F64((f64)(v).x, (f64)(v).y, (f64)(v).z, (f64)(v).w)
#define Vec4S32FromVec(v) V4S32((i32)(v).x, (i32)(v).y, (i32)(v).z, (i32)(v).w)
#define Vec4S64FromVec(v) V4S64((i64)(v).x, (i64)(v).y, (i64)(v).z, (i64)(v).w)

// NOTE(gsp): change
#define V2Zero() HMM_V2(0.f, 0.f)
#define V3Zero() HMM_V3(0.f, 0.f, 0.f)

////////////////////////////////
//~ rjf: Matrix Types

typedef struct Mat3x3F64 Mat3x3F64;
struct Mat3x3F64
{
 F64 elements[3][3];
};

typedef struct Mat4x4F64 Mat4x4F64;
struct Mat4x4F64
{
 F64 elements[4][4];
};

////////////////////////////////
//~ rjf: Interval Types

//- rjf: 1D intervals

typedef union Rng1F32 Rng1F32;
union Rng1F32
{
 struct
 {
  F32 min;
  F32 max;
 };
 F32 v[2];
};

typedef union Rng1F64 Rng1F64;
union Rng1F64
{
 struct
 {
  F64 min;
  F64 max;
 };
 F64 v[2];
};

typedef union Rng1U32 Rng1U32;
union Rng1U32
{
 struct
 {
  U32 min;
  U32 max;
 };
 U32 v[2];
};

typedef union Rng1U64 Rng1U64;
union Rng1U64
{
 struct
 {
  U64 min;
  U64 max;
 };
 U64 v[2];
};

typedef union Rng1S32 Rng1S32;
union Rng1S32
{
 struct
 {
  S32 min;
  S32 max;
 };
 S32 v[2];
};

typedef union Rng1S64 Rng1S64;
union Rng1S64
{
 struct
 {
  S64 min;
  S64 max;
 };
 S64 v[2];
};

//- rjf: 2D intervals

typedef union Rng2F32 Rng2F32;
union Rng2F32
{
 struct
 {
  HMM_Vec2 min;
  HMM_Vec2 max;
 };
 struct
 {
  HMM_Vec2 p0;
  HMM_Vec2 p1;
 };
 struct
 {
  F32 x0;
  F32 y0;
  F32 x1;
  F32 y1;
 };
 HMM_Vec2 v[2];
};

typedef union Rng2F64 Rng2F64;
union Rng2F64
{
 struct
 {
  Vec2F64 min;
  Vec2F64 max;
 };
 struct
 {
  Vec2F64 p0;
  Vec2F64 p1;
 };
 struct
 {
  F64 x0;
  F64 y0;
  F64 x1;
  F64 y1;
 };
 Vec2F64 v[2];
};

typedef union Rng2S32 Rng2S32;
union Rng2S32
{
 struct
 {
  Vec2S32 min;
  Vec2S32 max;
 };
 struct
 {
  Vec2S32 p0;
  Vec2S32 p1;
 };
 struct
 {
  S32 x0;
  S32 y0;
  S32 x1;
  S32 y1;
 };
 Vec2S32 v[2];
};

typedef union Rng2S64 Rng2S64;
union Rng2S64
{
 struct
 {
  Vec2S64 min;
  Vec2S64 max;
 };
 struct
 {
  Vec2S64 p0;
  Vec2S64 p1;
 };
 struct
 {
  S64 x0;
  S64 y0;
  S64 x1;
  S64 y1;
 };
 Vec2S64 v[2];
};

//- rjf: 3D intervals

typedef union Rng3F32 Rng3F32;
union Rng3F32
{
 struct
 {
  HMM_Vec3 min;
  HMM_Vec3 max;
 };
 struct
 {
  HMM_Vec3 p0;
  HMM_Vec3 p1;
 };
 struct
 {
  F32 x0;
  F32 y0;
  F32 z0;
  F32 x1;
  F32 y1;
  F32 z1;
 };
 HMM_Vec3 v[2];
};

//- rjf: 1D interval list types

typedef struct Rng1U64Node Rng1U64Node;
struct Rng1U64Node
{
 Rng1U64Node *next;
 Rng1U64 v;
};

typedef struct Rng1U64List Rng1U64List;
struct Rng1U64List
{
 Rng1U64Node *first;
 Rng1U64Node *last;
 U64 count;
 U64 total_count;
};

#define Rng2F32FromRng2(r) R2F32(HMM_V2((f32)r.p0.x, (f32)r.p0.y), HMM_V2((f32)r.p1.x, (f32)r.p1.y))
#define Rng2S32FromRng2(r) R2S32(Vec2S32FromVec(r.p0), Vec2S32FromVec(r.p1))

////////////////////////////////
//~ rjf: Scalar Ops

function F32 Mix1F32(F32 a, F32 b, F32 t);

////////////////////////////////
//~ rjf: Vector Ops

function Vec2F64 V2F64(F64 x, F64 y);
function Vec2F64 Add2F64(Vec2F64 a, Vec2F64 b);
function Vec2F64 Sub2F64(Vec2F64 a, Vec2F64 b);
function Vec2F64 Mul2F64(Vec2F64 a, Vec2F64 b);
function Vec2F64 Div2F64(Vec2F64 a, Vec2F64 b);
function Vec2F64 Scale2F64(Vec2F64 a, F64 scale);
function F64 Dot2F64(Vec2F64 a, Vec2F64 b);
function F64 LengthSquared2F64(Vec2F64 v);
function F64 Length2F64(Vec2F64 v);
function Vec2F64 Normalize2F64(Vec2F64 v);
function Vec2F64 Mix2F64(Vec2F64 a, Vec2F64 b, F64 t);

function Vec2S32 V2S32(S32 x, S32 y);
function Vec2S32 Add2S32(Vec2S32 a, Vec2S32 b);
function Vec2S32 Sub2S32(Vec2S32 a, Vec2S32 b);
//gsp: addition
function Vec2S32 Scale2S32(Vec2S32 a, U32 scale);

function Vec2S64 V2S64(S64 x, S64 y);
function Vec2S64 Add2S64(Vec2S64 a, Vec2S64 b);
function Vec2S64 Sub2S64(Vec2S64 a, Vec2S64 b);

function b32     V2F32IsZero(HMM_Vec2 v);
function b32     V3F32IsZero(HMM_Vec3 v);
function Vec3F64 V3F64(F64 x, F64 y, F64 z);
function Vec3F64 Add3F64(Vec3F64 a, Vec3F64 b);
function Vec3F64 Sub3F64(Vec3F64 a, Vec3F64 b);
function Vec3F64 Mul3F64(Vec3F64 a, Vec3F64 b);
function Vec3F64 Div3F64(Vec3F64 a, Vec3F64 b);
function Vec3F64 Scale3F64(Vec3F64 a, F64 scale);
function F64 Dot3F64(Vec3F64 a, Vec3F64 b);
function F64 LengthSquared3F64(Vec3F64 v);
function F64 Length3F64(Vec3F64 v);
function Vec3F64 Normalize3F64(Vec3F64 v);
function Vec3F64 Mix3F64(Vec3F64 a, Vec3F64 b, F64 t);
function Vec3F64 Cross3F64(Vec3F64 a, Vec3F64 b);
function Vec3F64 Transform3F64(Vec3F64 v, Mat3x3F64 m);

function Vec3S32 V3S32(S32 x, S32 y, S32 z);

function Vec3S64 V3S64(S64 x, S64 y, S64 z);

function Vec4F64 V4F64(F64 x, F64 y, F64 z, F64 w);
function Vec4F64 Add4F64(Vec4F64 a, Vec4F64 b);
function Vec4F64 Sub4F64(Vec4F64 a, Vec4F64 b);
function Vec4F64 Mul4F64(Vec4F64 a, Vec4F64 b);
function Vec4F64 Div4F64(Vec4F64 a, Vec4F64 b);
function Vec4F64 Scale4F64(Vec4F64 a, F64 scale);
function F64 Dot4F64(Vec4F64 a, Vec4F64 b);
function F64 LengthSquared4F64(Vec4F64 v);
function F64 Length4F64(Vec4F64 v);
function Vec4F64 Normalize4F64(Vec4F64 v);
function Vec4F64 Mix4F64(Vec4F64 a, Vec4F64 b, F64 t);
function Vec4F64 Transform4F64(Vec4F64 v, Mat4x4F64 m);

function Vec4S32 V4S32(S32 x, S32 y, S32 z, S32 w);

function Vec4S64 V4S64(S64 x, S64 y, S64 z, S64 w);

////////////////////////////////
//~ rjf: Range Ops

// gsp: change
#define R2IsNil(r) ((r.p0.x == 0) && (r.p0.y == 0) && (r.p1.x == 0) && (r.p1.y == 0))

function Rng1F32 R1F32(F32 min, F32 max);
function Rng1F32 Shift1F32(Rng1F32 r, F32 v);
function Rng1F32 Pad1F32(Rng1F32 r, F32 x);
function F32 Center1F32(Rng1F32 r);
function B32 Contains1F32(Rng1F32 r, F32 v);
function F32 Dim1F32(Rng1F32 r);
function Rng1F32 Union1F32(Rng1F32 a, Rng1F32 b);
function Rng1F32 Intersection1F32(Rng1F32 a, Rng1F32 b);

function Rng1F64 R1F64(F64 min, F64 max);
function Rng1F64 Shift1F64(Rng1F64 r, F64 v);
function Rng1F64 Pad1F64(Rng1F64 r, F64 x);
function F64 Center1F64(Rng1F64 r);
function B32 Contains1F64(Rng1F64 r, F64 v);
function F64 Dim1F64(Rng1F64 r);
function Rng1F64 Union1F64(Rng1F64 a, Rng1F64 b);
function Rng1F64 Intersection1F64(Rng1F64 a, Rng1F64 b);

function Rng1S32 R1S32(S32 min, S32 max);
function Rng1S32 Shift1S32(Rng1S32 r, S32 v);
function Rng1S32 Pad1S32(Rng1S32 r, S32 x);
function S32 Center1S32(Rng1S32 r);
function B32 Contains1S32(Rng1S32 r, S32 v);
function S32 Dim1S32(Rng1S32 r);
function Rng1S32 Union1S32(Rng1S32 a, Rng1S32 b);
function Rng1S32 Intersection1S32(Rng1S32 a, Rng1S32 b);

function Rng1S64 R1S64(S64 min, S64 max);
function Rng1S64 Shift1S64(Rng1S64 r, S64 v);
function Rng1S64 Pad1S64(Rng1S64 r, S64 x);
function S64 Center1S64(Rng1S64 r);
function B32 Contains1S64(Rng1S64 r, S64 v);
function S64 Dim1S64(Rng1S64 r);
function Rng1S64 Union1S64(Rng1S64 a, Rng1S64 b);
function Rng1S64 Intersection1S64(Rng1S64 a, Rng1S64 b);

function Rng1U32 R1U32(U32 min, U32 max);
function Rng1U32 Shift1U32(Rng1U32 r, U32 v);
function Rng1U32 Pad1U32(Rng1U32 r, U32 x);
function U32 Center1U32(Rng1U32 r);
function B32 Contains1U32(Rng1U32 r, U32 v);
function U32 Dim1U32(Rng1U32 r);
function Rng1U32 Union1U32(Rng1U32 a, Rng1U32 b);
function Rng1U32 Intersection1U32(Rng1U32 a, Rng1U32 b);

function Rng1U64 R1U64(U64 min, U64 max);
function Rng1U64 Shift1U64(Rng1U64 r, U64 v);
function Rng1U64 Pad1U64(Rng1U64 r, U64 x);
function U64 Center1U64(Rng1U64 r);
function B32 Contains1U64(Rng1U64 r, U64 v);
function U64 Dim1U64(Rng1U64 r);
function Rng1U64 Union1U64(Rng1U64 a, Rng1U64 b);
function Rng1U64 Intersection1U64(Rng1U64 a, Rng1U64 b);

function Rng2F32  R2F32(HMM_Vec2 min, HMM_Vec2 max);
function Rng2F32  Shift2F32(Rng2F32 r, HMM_Vec2 v);
function Rng2F32  Pad2F32(Rng2F32 r, F32 x);
function HMM_Vec2 Center2F32(Rng2F32 r);
function B32      Contains2F32(Rng2F32 r, HMM_Vec2 v);
function HMM_Vec2 Dim2F32(Rng2F32 r);
function Rng2F32  Union2F32(Rng2F32 a, Rng2F32 b);
function Rng2F32  Intersection2F32(Rng2F32 a, Rng2F32 b);

function Rng2F64 R2F64(Vec2F64 min, Vec2F64 max);
function Rng2F64 Shift2F64(Rng2F64 r, Vec2F64 v);
function Rng2F64 Pad2F64(Rng2F64 r, F64 x);
function Vec2F64 Center2F64(Rng2F64 r);
function B32 Contains2F64(Rng2F64 r, Vec2F64 v);
function Vec2F64 Dim2F64(Rng2F64 r);
function Rng2F64 Union2F64(Rng2F64 a, Rng2F64 b);
function Rng2F64 Intersection2F64(Rng2F64 a, Rng2F64 b);

function Rng2S32 R2S32(Vec2S32 min, Vec2S32 max);
function Rng2S32 Shift2S32(Rng2F32 r, Vec2S32 v);
function Rng2S32 Pad2S32(Rng2S32 r, S32 x);
function Vec2S32 Center2S32(Rng2S32 r);
function B32 Contains2S32(Rng2S32 r, Vec2S32 v);
function Vec2S32 Dim2S32(Rng2S32 r);
function Rng2S32 Union2S32(Rng2S32 a, Rng2S32 b);
function Rng2S32 Intersection2S32(Rng2S32 a, Rng2S32 b);

function Rng2S64 R2S64(Vec2S64 min, Vec2S64 max);
function Rng2S64 Shift2S64(Rng2F32 r, Vec2S64 v);
function Rng2S64 Pad2S64(Rng2S64 r, S64 x);
function Vec2S64 Center2S64(Rng2S64 r);
function B32 Contains2S64(Rng2S64 r, Vec2S64 v);
function Vec2S64 Dim2S64(Rng2S64 r);
function Rng2S64 Union2S64(Rng2S64 a, Rng2S64 b);
function Rng2S64 Intersection2S64(Rng2S64 a, Rng2S64 b);

function Rng3F32 R3F32(HMM_Vec3 min, HMM_Vec3 max);
function Rng3F32 Shift3F32(Rng3F32 r, HMM_Vec3 v);
function Rng3F32 Pad3F32(Rng3F32 r, F32 x);
function HMM_Vec3 Center3F32(Rng3F32 r);
function B32 Contains3F32(Rng3F32 r, HMM_Vec3 v);
function HMM_Vec3 Dim3F32(Rng3F32 r);
function Rng3F32 Union3F32(Rng3F32 a, Rng3F32 b);
function Rng3F32 Intersection3F32(Rng3F32 a, Rng3F32 b);

////////////////////////////////
//~ rjf: Lists

function void Rng1U64ListPushNode(Rng1U64List *list, Rng1U64Node *node);
function void Rng1U64ListPushNodeFront(Rng1U64List *list, Rng1U64Node *node);
//function void Rng1U64ListPush(M_Arena *arena, Rng1U64List *list, Rng1U64 rng);
//function void Rng1U64ListPushFront(M_Arena *arena, Rng1U64List *list, Rng1U64 rng);

////////////////////////////////
//~ rjf: Miscellaneous Ops

function HMM_Vec3 HSVFromRGB(HMM_Vec3 rgb);
function HMM_Vec3 RGBFromHSV(HMM_Vec3 hsv);
function HMM_Vec4 HMM_Vec4FromHexRGBA(U32 hex);
function F32 MillisecondsFromMicroseconds(U64 microseconds);
function U64 MicrosecondsFromMilliseconds(F32 milliseconds);
function Vec2S64 SideVertexFromCorner(Corner corner);

////////////////////////////////////////////////////////////////////////////////////////////
//~ gsp: Stuff I added
function Vec2S32 SideVertexFromCorner32(Corner corner);


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


#endif //BASE_MATH_H
