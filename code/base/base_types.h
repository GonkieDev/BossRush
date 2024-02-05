/* date = July 8th 2023 0:54 pm */

#ifndef BASE_TYPES_INCLUDE_H
#define BASE_TYPES_INCLUDE_H

#include <stdint.h>
#include <math.h> // currently, only for INIFINITY

#if COMPILER_MSVC
#include <intrin.h>
#endif

//- gsp: base types 
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef i8  b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef void* VoidFunc;

typedef struct DateTime DateTime;
struct DateTime
{
    u8 hours;         // [0, 23]
    u8 minutes;       // [0, 59]
    u8 seconds;       // [0, 59]
    u8 day;           // [0, 30]
    u8 dayOfWeek;     // [0,  6]
    u8 month;         // [0, 11]
    u16 milliseconds; // [0, 999]
    u32 year;    
};


//- gsp: keywords 

#if !BUILD_ROOT
# define function external
# define global   external
#else
# define function static
# define global   static
#endif
#define local_persist static
#define inline_function inline

// NOTE(gsp): used in switch statements to signify that that statement does not break
#define fallthrough

#if COMPILER_MSVC
# define _per_thread __declspec(thread)
#elif COMPILER_CLANG
# define _per_thread __thread
#elif COMPILER_GCC
# define _per_thread __thread
#else
# error "_per_thread must be defined! Something has gone wrong."
#endif

#if !BUILD_EXTERNAL_USAGE
# define per_thread _per_thread
#else
# define per_thread external _per_thread
#endif

#if COMPILER_MSVC
# define shared_export __declspec(dllexport)
#else
# error "shared_export not defined for this compiler!"
#endif

//- gsp: name mangling 
#if LANG_CPP
# define no_name_mangle extern "C"
# define external extern "C"
#else
# define no_name_mangle
# define external extern
#endif

#define export_function no_name_mangle shared_export

///////////////////////////////
//~ gsp: utils 

#define ArrayCount(arr) (sizeof(arr) / sizeof(*(arr)))

#define ProtectCall(call, success)    \
if ((success)) { if (!(call)) { (success) = 0; Assert(0); } }
#define ProtectAssign(assign, badTarget, success)    \
if ((success)) { if ((assign) == (badTarget)) { Assert(0); (success) = 0; } }

#define AbsoluteValueS32(x) (i32)abs((x))
#define AbsoluteValueS64(x) (i64)llabs((x))
#define AbsoluteValueU64(x) (u64)llabs((u64)(x))

//- gsp: Ptrs 
#define AlignPowOf2(x, p) (((x) + (p) - 1)&~((p) - 1))
#define IntFromPtr(p) (u64)((u8*)(p) - (u8*)0)
#define PtrFromInt(n) (void*)((u8*)0 + (n))

#define Swap(type, a, b) do{ type _swapper_ = a; a = b; b = _swapper_; }while(0)

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A,B) A##B
#define Glue(A, B) Glue(A, B)

#define Member(T, m) (((T*)0)->m)
#define SizeOfMember(T, m) sizeof(Member(T, m))
#define OffsetOfMember(T, m) IntFromPtr(&Member(T, m))

//- gsp: Clamp, min, max 
#define Min(a, b) ( ((a)<(b)) ? (a) : (b) )
#define Max(a, b) ( ((a)>(b)) ? (a) : (b) )
#define Min3(a, b, c) (Min(Min(a, b), c))
#define Max3(a, b, c) (Max(Max(a, b), c))
#define Clamp(a, x, b) (((a)>(x))?(a):((b)<(x))?(b):(x))

// gsp: Size utils
#define KB(x) (x << 10)
#define MB(x) (x << 20)
#define GB(x) ((u64)x << 30)
#define TB(x) ((u64)x << 40)

// gsp: Time utils
#define SEC_TO_MILLISECONDS(t) ((t) * 1000)
#define SEC_TO_MICROSECONDS(t) ((t) * 1000000)
#define SEC_TO_NANOSECONDS(t)  ((t) * 1000000000LL)

// gsp: Units utils
#define Thousand(n) ((n)*1000)
#define Million(n)  ((n)*1000000)
#define Billion(n)  ((n)*1000000000LL)

// gsp: (foreach) Loop utils
#define EachElement(arr, it) u64 it = 0; it < ArrayCount(arr); it += 1
#define EachEnumVal(type, it) type it = (type)0; it < type##_COUNT; it = (type)(it+1)
#define EachNonzeroEnumVal(type, it) type it = (type)1; it < type##_COUNT; it = (type)(it+1)

// gsp: defer loop
#define DeferLoop(start, end) for(int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))
#define DeferLoopChecked(begin, end) for(int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))
#define DeferLoopGuarded(guard, start, end) for(int _i_ = ((guard) ? (start) : 0, (guard) ? 0 : 1); _i_ == 0; _i_ += 1, (end))


//- Padding
#define PAD16(ammount) u16 padding16_x##ammount[ammount]
#define PAD32(ammount) u32 padding32_x##ammount[ammount]

//- gsp: Memory functions 

// dst, src, size
#define MemoryCopy memcpy
#define MemoryMove memmove
#define MemorySet  memset

#define MemoryCopyStruct(dst, src) do { Assert(sizeof(*(dst)) == sizeof(*(src))); MemoryCopy((dst), (src), sizeof(*(dst))); } while(0)
#define MemoryCopyArray(dst, src)  do { Assert(sizeof(dst) == sizeof(src)); MemoryCopy((dst), (src), sizeof(src)); }while(0)

#define MemoryZero(ptr, size) MemorySet((ptr), 0, (size))
#define MemoryZeroStruct(ptr) MemoryZero((ptr), sizeof(*(ptr)))
#define MemoryZeroArray(arr)  MemoryZero((arr), sizeof(arr))

#define UnusedVariable(name) (void)name

//- gsp: read_only 
#if OS_WINDOWS
# pragma section(".roglob", read)
# define read_only __declspec(allocate(".roglob"))
#else
# define read_only
#endif

//- gsp: Atomics 
#if OS_WINDOWS
// WE ARE SLY FOXES!!!
# define AtomicIncEval64(ptr) InterlockedIncrement64(ptr)
# define AtomicDecEval64(ptr) InterlockedDecrement64(ptr)
# define AtomicEvalSet64(ptr, val) InterlockedExchange64((ptr), (val))
#else
# error Atomics not implemented for this platform.
#endif

//- gsp: bitmasks 
#define bitmask1  0x01
#define bitmask2  0x03
#define bitmask3  0x07
#define bitmask4  0x0F
#define bitmask5  0x1F
#define bitmask6  0x3F
#define bitmask7  0x7F
#define bitmask8  0xFF
#define bitmask9  0x01FF
#define bitmask10 0x03FF
#define bitmask11 0x07FF
#define bitmask12 0x0FFF
#define bitmask13 0x01FFF
#define bitmask14 0x03FFF

//- gsp: Limits 
read_only global u8 U8Max = 0xFF;
read_only global u8 U8Min = 0;

read_only global u16 U16Max = 0xFFFF;
read_only global u16 U16Min = 0;

read_only global u32 U32Max = 0xFFFFFFFF;
read_only global u32 U32Min = 0;

read_only global u64 U64Max = 0xFFFFFFFFFFFFFFFF;
read_only global u64 U64Min = 0;

read_only global i8 I8Max = 0x7F;
read_only global i8 I8Min = -1 - 0x7F;

read_only global i16 I16Max = 0x7FFF;
read_only global i16 I16Min = -1 - 0x7FFF;

read_only global i32 I32Max = 0x7FFFFFFF;
read_only global i32 I32Min = -1 - 0x7FFFFFFF;

read_only global i64 I64Max = 0x7FFFFFFFFFFFFFFF;
read_only global i64 I64Min = -1 - 0x7FFFFFFFFFFFFFFF;

//- gsp: Constants 
read_only global f32 F32Max = (f32)(3.4028234664e+38);
read_only global f32 F32Min = (f32)(-3.4028234664e+38);
read_only global f32 F32SmallestPositive = (f32)(1.1754943508e-38);
read_only global f32 F32Epsilon = (f32)(5.96046448e-8);

read_only global u32 SignF32 = 0x80000000;
read_only global u32 ExponentF32 = 0x7F800000;
read_only global u32 MantissaF32 = 0x7FFFFF;

read_only global u64 SignF64 = 0x8000000000000000ull;
read_only global u64 ExponentF64 = 0x7FF0000000000000ull;
read_only global u64 MantissaF64 = 0xFFFFFFFFFFFFFull;

//- gsp: linked list helpers 

#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define QueuePush_NZ(f,l,n,next,zchk,zset) (zchk(f)?\
(((f)=(l)=(n)), zset((n)->next)):\
((l)->next=(n),(l)=(n),zset((n)->next)))
#define QueuePushFront_NZ(f,l,n,next,zchk,zset) (zchk(f) ? (((f) = (l) = (n)), zset((n)->next)) :\
((n)->next = (f)), ((f) = (n)))
#define QueuePop_NZ(f,l,next,zset) ((f)==(l)?\
(zset(f),zset(l)):\
((f)=(f)->next))
#define StackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define StackPop_NZ(f,next,zchk) (zchk(f)?0:((f)=(f)->next))

#define DLLInsert_NPZ(f,l,p,n,next,prev,zchk,zset) \
(zchk(f) ? (((f) = (l) = (n)), zset((n)->next), zset((n)->prev)) :\
zchk(p) ? (zset((n)->prev), (n)->next = (f), (zchk(f) ? (0) : ((f)->prev = (n))), (f) = (n)) :\
((zchk((p)->next) ? (0) : (((p)->next->prev) = (n))), (n)->next = (p)->next, (n)->prev = (p), (p)->next = (n),\
((p) == (l) ? (l) = (n) : (0))))
#define DLLPushBack_NPZ(f,l,n,next,prev,zchk,zset) DLLInsert_NPZ(f,l,l,n,next,prev,zchk,zset)
#define DLLRemove_NPZ(f,l,n,next,prev,zchk,zset) (((f)==(n))?\
((f)=(f)->next, (zchk(f) ? (zset(l)) : zset((f)->prev))):\
((l)==(n))?\
((l)=(l)->prev, (zchk(l) ? (zset(f)) : zset((l)->next))):\
((zchk((n)->next) ? (0) : ((n)->next->prev=(n)->prev)),\
(zchk((n)->prev) ? (0) : ((n)->prev->next=(n)->next))))


#define QueuePush(f,l,n)         QueuePush_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePushFront(f,l,n)    QueuePushFront_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePop(f,l)            QueuePop_NZ(f,l,next,SetNull)
#define StackPush(f,n)           StackPush_N(f,n,next)
#define StackPop(f)              StackPop_NZ(f,next,CheckNull)
#define DLLPushBack(f,l,n)       DLLPushBack_NPZ(f,l,n,next,prev,CheckNull,SetNull)
#define DLLPushFront(f,l,n)      DLLPushBack_NPZ(l,f,n,prev,next,CheckNull,SetNull)
#define DLLInsert(f,l,p,n)       DLLInsert_NPZ(f,l,p,n,next,prev,CheckNull,SetNull)
#define DLLRemove(f,l,n)         DLLRemove_NPZ(f,l,n,next,prev,CheckNull,SetNull)


//- gsp: loops 
#define for_in(type, name, array) for (u32 name##Idx = 0; name##Idx < ArrayCount(array); ++name##Idx)


//- gsp: assertions 
#if OS_WINDOWS
#define BreakDebugger() __debugbreak()
#else
#define BreakDebugger() (*(volatile int *)0 = 0)
#endif

#undef Assert
#if BUILD_DEBUG
# define Assert(b) do { if(!(b)) { BreakDebugger(); } } while(0)
#else
# define Assert(b)
#endif

#define AssertMsg(b, msg) Assert((b) && (msg))

#define StaticAssert(b, msg) static_assert(b, msg)
#if 0
#define StaticAssert(c, label) u8 static_assert_##label[(c)?(1):(-1)]
#endif

#define NotImplemented AssertMsg(0, "Not Implemented")
#define InvalidPath AssertMsg(0, "Invalid Path")
#define NotImplementedStatic StaticAssert(0, "Not Implemented")
#define InvalidPathStatic StaticAssert(0, "Invalid Path")

//- gsp: snprintf alias for MSVC < 2015 
#if COMPILER_MSVC && COMPILER_MSVC_YEAR < 2015
# define snprintf _snprintf
#endif

#define UnreferencedParameter(param) ((void)(param))

//////////////////////////
//~ gsp: Helper functions
inline_function f32
F32Inf(void)
{
    union { u32 u; f32 f; } x;
    x.u = 0x7F800000;
    return x.f;
}

inline_function f32
F32NegInf(void)
{
    union { u32 u; f32 f; } x;
    x.u = 0xFF800000;
    return x.f;
}

inline_function b32
F32IsNan(f32 f)
{
    union { u32 u; f32 f; } x;
    x.f = f;
    return ((x.u & ExponentF32) == ExponentF32) && ((x.u & MantissaF32) != 0);
}

inline_function b32
F32IsDenorm(f32 f)
{
    union { u32 u; f32 f; } x;
    x.f = f;
    return ((x.u & ExponentF32) == 0) && ((x.u & MantissaF32) != 0);
}

inline_function f32
AbsoluteValueF32(f32 f)
{
    union { u32 u; f32 f; } x;
    x.f = f;
    x.u = x.u & ~SignF32;
    return x.f;
}

inline_function f64
AbsoluteValueF64(f64 f)
{
    union { u64 u; f64 f; } x;
    x.f = f;
    x.u = x.u & ~SignF64;
    return x.f;
}

inline_function f32
SignFromF32(f32 f)
{
    return f < 0.f ? -1.f : +1.f;
}

inline_function f64
SignFromF64(f64 f)
{
    return f < 0.0 ? -1.0 : +1.0;
}

inline_function i32
U64SafeTruncateToI32(u64 x)
{
    Assert(x <= (u64)I32Max);
    return ((i32)x);
}

inline_function u32
U64SafeTruncateToU32(u64 x)
{
    Assert(x <= (u64)U32Max);
    return ((u32)x);
}


////////////////////////////////
//~ gsp: Base Enums for rfj maths library

typedef enum Side
{
    Side_Invalid = -1,
    Side_Min,
    Side_Max,
    Side_COUNT
}
Side;

typedef enum Axis2
{
    Axis2_Invalid = -1,
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT
}
Axis2;
#define Axis2_Flip(a) ((Axis2)(!(a)))
#define For_Axis2(axis) for (Axis2 (axis) = Axis2_X; (axis) < Axis2_COUNT; ++(axis))

typedef enum Axis3
{
    Axis3_Invalid = -1,
    Axis3_X,
    Axis3_Y,
    Axis3_Z,
    Axis3_COUNT
}
Axis3;

typedef enum Axis4
{
    Axis4_Invalid = -1,
    Axis4_X,
    Axis4_Y,
    Axis4_Z,
    Axis4_W,
    Axis4_COUNT
}
Axis4;

typedef enum Corner
{
    Corner_Invalid = -1,
    Corner_00,
    Corner_01,
    Corner_10,
    Corner_11,
    Corner_COUNT
}
Corner;

typedef enum Dir2
{
    Dir2_Left,
    Dir2_Up,
    Dir2_Right,
    Dir2_Down,
    Dir2_COUNT
}
Dir2;

typedef enum Comparison
{
    Comparison_Null,
    Comparison_EqualTo,
    Comparison_NotEqualTo,
    Comparison_LessThan,
    Comparison_LessThanOrEqualTo,
    Comparison_GreaterThan,
    Comparison_GreaterThanOrEqualTo,
    Comparison_COUNT
}
Comparison;

typedef struct u32LListNode u32LListNode;
struct u32LListNode
{
    u32LListNode *next;
    u32 x;
    u32 __pad;
};

typedef struct u32LList u32LList;
struct u32LList
{
    u32 *first;
    u32  nodeCount;
    u32 __pad;
};


///////////////////////////////////////////////
//~ gsp: For each 

#define EachCorner(name) Corner name = (Corner)0; name < Corner_COUNT; name = (Corner)(name+1)


///////////////////////////////////////////////
//~ gsp: Forward decls

//- Array helpers 
#define U32InArray(arr, target) U32InArray_((arr), sizeof(arr), (target), sizeof(arr))
#define U32InArrayHB(arr, target, highBound) U32InArray_((arr), sizeof(arr), (target), (highBound))

inline_function b32 U32InArray_(u32 *array, u32 arraySize, u32 target, u32 goUpUntil);


///////////////////////////////////////////////
//~ End of file
#endif //BASE_TYPES_INCLUDE_H
