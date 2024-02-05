/* date = July 8th 2023 9:14 pm */

// This file deals with Strings, Paths, String formatting, etc.
// Based of various codebases such as Ryan Fleury's and Casey Muratori's, as well as some cool stuff I saw in John Blow's Jai programming language

#ifndef BASE_STRING_INCLUDE_H
#define BASE_STRING_INCLUDE_H

#include <stdarg.h>
#include "stb_sprintf.h"

//////////////////////
//~ gsp: string operation types

// NOTE(gsp): size of strings does not include the null char
// but they are stored with a null character at the end

typedef struct String8 String8;
struct String8
{
    u64 size;
    u8 *str;
};

typedef struct String16 String16;
struct String16
{
    u64 size;
    u16 *str;
};

typedef struct String32 String32;
struct String32
{
    u64 size;
    u32 *str;
};

//- gsp: string lists 
typedef struct String8Node String8Node;
struct String8Node
{
    String8Node *next;
    String8 string;
};

typedef struct String8List String8List;
struct String8List
{
    String8Node *first;
    String8Node *last;
    u64 nodeCount;
    u64 totalSize;
};

typedef struct DecodedCodepoint DecodedCodepoint;
struct DecodedCodepoint
{
    u32 codepoint;
    u32 advance;
};

//////////////////////
//~ gsp: string operation types

typedef u32 MatchFlags;
enum
{
    MatchFlag_None             = (1<<0),
    MatchFlag_CaseInsensitive  = (1<<1),
    MatchFlag_RightSideSloppy  = (1<<2),
    MatchFlag_SlashInsensitive = (1<<3),
    MatchFlag_FindLast         = (1<<4),
    MatchFlag_KeepEmpties      = (1<<5),
};

typedef struct StringJoin StringJoin;
struct StringJoin
{
    String8 pre;
    String8 sep;
    String8 post;
};

typedef enum PathKind
{
    PathKind_Relative,
    PathKind_DriveAbsolute,
    PathKind_RootAbsolute,
    PathKind_COUNT
}
PathKind;

//////////////////////
//~ gsp: char functions

inline_function b32 CharIsAlpha(u8 c);
inline_function b32 CharIsAlphaUpper(u8 c);
inline_function b32 CharIsAlphaLower(u8 c);
inline_function b32 CharIsDigit(u8 c);
inline_function b32 CharIsSymbol(u8 c);
inline_function b32 CharIsSpace(u8 c);
inline_function u8  CharToUpper(u8 c);
inline_function u8  CharToLower(u8 c);
// NOTE(gsp): turns '\\' to '/'
inline_function u8  CharToForwardSlash(u8 c);

//////////////////////
//~ gsp: String functions

inline_function u64 CalculateCStringLen(char *cstr);

//- gsp: Constructors 
inline_function String8 Str8(u8 *str, u64 size);
#define Str8C(cstring) Str8((u8 *)(cstring), CalculateCStringLen(cstring))
#define S8Lit(s) Str8((u8 *)(s), sizeof(s)-1)
#define S8LitComp(s) { sizeof(s)-1, (u8*)(s)}
inline_function String8 Str8Range(u8 *first, u8 *onePastLast);
inline_function String8 Str8Advance(String8 s, u64 ammount);
inline_function String16 Str16(u16 *str, u64 size);
inline_function String16 Str16C(u16 *ptr);
inline_function String32 Str32(u32 *str, u64 size);

#define S8VArg(s) (int)(s).size, (s).str
#define Rng2F32ToS8Arg(r) (float)(r).p0.x, (float)(r).p0.y, (float)(r).p1.x, (float)(r).p1.y

//- gsp: substrings 
function String8 SubStr8(String8 str, Rng1U64 rng);
function String8 Str8Skip(String8 str, u64 min);
function String8 Str8Chop(String8 str, u64 nmax);
function String8 Prefix8(String8 str, u64 size);
function String8 Suffix8(String8 str, u64 size);

//- gsp: matching 
function b32 Str8StrictMatch(String8 a, String8 b);
function b32 Str8Match(String8 a, String8 b, MatchFlags flags);
function u64 FindSubStr8(String8 haystack, String8 needle, u64 start_pos, MatchFlags flags);
function b32 Str8EndsIn(String8 haystack, String8 needle);

//- gsp: allocation 
function String8 PushStr8Copy(M_Arena *arena, String8 string);
function String8 PushStr8FmtVAList(M_Arena *arena, char *fmt, va_list args);
function String8 PushStr8Fmt(M_Arena *arena, char *fmt, ...);
function String8 PushStr8FillByte(M_Arena *arena, u64 size, u8 byte);

//////////////////////
//~ gsp: String lists
inline_function String8Node S8Node(String8 s);
#define S8NodeC(cstring) S8Node(Str8C(cstring));
#define S8NodeLit(s) S8Node(S8Lit(s));

function void Str8ListPushNode(String8List *list, String8Node *n);
function void Str8ListPushNodeFront(String8List *list, String8Node *n);
function void Str8ListPush(M_Arena *arena, String8List *list, String8 string);
function void Str8ListPushFront(M_Arena *arena, String8List *list, String8 string);
function void Str8ListConcatInPlace(String8List *list, String8List *toPush);
// NOTE(gsp): splits "string" into a String8List delimited by strings in "splits"
function String8List StrSplit8(M_Arena *arena, String8 string, u64 splitCount, String8 *splits);
// NOTE(gsp): appends null char to the end
function String8 Str8ListJoin(M_Arena *arena, String8List list, StringJoin *optionalParams);
// NOTE(gsp): allocates list.count * (char *) with each pointer pointing a node's string
function char **Str8ListGetStrCArray(M_Arena *arena, String8List list);

//////////////////////
//~ gsp: Range functions
function String8 Str8PutRange(String8 str, Rng1U64 r, String8 replacement, u64 maxLen);

//////////////////////
//~ gsp: Path Helpers

// NOTE(gsp): these functions will not return a string with a 0 char at the end
function String8 Str8PathChopLastPeriod(String8 string);
function String8 Str8PathChopPastLastSlash(String8 string);
function String8 Str8PathSkipLastPeriod(String8 string);
function String8 Str8PathChopAfterLastSlash(String8 string);
function void    Str8PathCurateBackslashes(String8 string);

function PathKind PathKindFromStr8(String8 path);

//////////////////////
//~ gsp: Unicode conversions
function DecodedCodepoint DecodeCodepointFromUtf8(u8 *str, u64 strLen);
function DecodedCodepoint DecodeCodepointFromUtf16(u16 *str, u64 strLen);
// NOTE(gsp): returns number of bytes written to "out"
function u32 Utf8FromCodepoint(u8 *out, u32 codepoint);
function u32 Utf16FromCodepoint(u16 *out, u32 codepoint);
function String8 String8From16(M_Arena *arena, String16 in);
function String16 String16From8(M_Arena *arena, String8 in);
function String32 String32From8(M_Arena *arena, String8 in);

#endif //BASE_STRING_INCLUDE_H
