#ifndef BASE_STRING_INCLUDE_C
#define BASE_STRING_INCLUDE_C


#if COMPILER_MSVC
# pragma warning( disable : 4820)
#endif

#if !BUILD_EXTERNAL_USAGE
# define STB_SPRINTF_IMPLEMENTATION
# include "stb_sprintf.h"
#endif

#if COMPILER_MSVC
# pragma warning( default : 4820)
#endif

#if !defined(MemoryCopy)
# error "MemoryCopy must be defined to use base_string.c."
#endif
#if !defined(MemorySet)
# error "MemorySet must be defined to use base_string.c."
#endif

//////////////////////
//~ gsp: char functions

inline_function
b32 CharIsAlpha(u8 c)
{
    return CharIsAlphaUpper(c) || CharIsAlphaLower(c);
}

inline_function
b32 CharIsAlphaUpper(u8 c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

inline_function
b32 CharIsAlphaLower(u8 c)
{
    return ((c >= 'a') && (c <= 'z'));
}

inline_function
b32 CharIsDigit(u8 c)
{
    return ((c >= '0') && (c <= '9'));
}

inline_function
b32 CharIsSymbol(u8 c)
{
    return (c == '~' || c == '!'  || c == '$' || c == '%' || c == '^' ||
            c == '&' || c == '*'  || c == '-' || c == '=' || c == '+' ||
            c == '<' || c == '.'  || c == '>' || c == '/' || c == '?' ||
            c == '|' || c == '\\' || c == '{' || c == '}' || c == '(' ||
            c == ')' || c == '\\' || c == '[' || c == ']' || c == '#' ||
            c == ',' || c == ';'  || c == ':' || c == '@');
}

inline_function
b32 CharIsSpace(u8 c)
{
    return (c == ' ');
}

inline_function
u8 CharToUpper(u8 c)
{
    return CharIsAlphaUpper(c) ? c : ('A' + c - 'a');
}

inline_function u8
CharToLower(u8 c)
{
    return CharIsAlphaLower(c) ? c : ('a' + c - 'A');
}

inline_function u8 
CharToForwardSlash(u8 c)
{
    return (c == '\\' ? '/' : c);
}

//////////////////////
//~ gsp: String functions

inline_function
u64 CalculateCStringLen(char *cstr)
{
    u64 length = 0;
    for (;cstr[length]; length++);
    return length;
}

//- gsp: Constructors 

inline_function String8
Str8(u8 *str, u64 size)
{
    String8 string;
    string.str = str;
    string.size = size;
    return string;
}

inline_function String8
Str8Range(u8 *first, u8 *onePastLast)
{
    String8 string;
    string.str = first;
    string.size = (u64)(onePastLast - first);
    return string;
}

inline_function String8
Str8Advance(String8 s, u64 ammount)
{
    s.size -= ammount;
    s.str  += ammount;
    return s;
}

inline_function String16
Str16(u16 *str, u64 size)
{
    String16 result;
    result.str = str;
    result.size = size;
    return result;
}

inline_function String16
Str16C(u16 *ptr)
{
    u16 *p = ptr;
    for (;*p; p += 1);
    String16 result = Str16(ptr, p - ptr);
    return result;
}

inline_function String32
Str32(u32 *str, u64 size)
{
    String32 string = {0};
    string.str = str;
    string.size = size;
    return string;
}

//- gsp: substrings 
function String8
SubStr8(String8 str, Rng1U64 rng)
{
    u64 min = Min(rng.min, str.size);
    u64 max = Min(rng.max, str.size);
    if(min > max)
    {
        u64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

function String8
Str8Skip(String8 str, u64 min)
{
    return SubStr8(str, R1U64(min, str.size));
}

function String8
Str8Chop(String8 str, u64 nmax)
{
    return SubStr8(str, R1U64(0, str.size - nmax));
}

function String8
Prefix8(String8 str, u64 size)
{
    return SubStr8(str, R1U64(0, size));
}

function String8
Suffix8(String8 str, u64 size)
{
    return SubStr8(str, R1U64(str.size - size, str.size));
}

//- gsp: Matching 
function b32
Str8StrictMatch(String8 a, String8 b)
{
    b32 result = 0;
    if (a.size == b.size)
    {
        result = 1;
        for (u64 i = 0; i < a.size; i += 1)
        {
            if ((a.str[i] != b.str[i]))
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

function b32
Str8Match(String8 a, String8 b, MatchFlags flags)
{
    b32 result = 0;
    if (a.size == b.size || flags & MatchFlag_RightSideSloppy)
    {
        result = 1;
        for (u64 i = 0; i < Min(a.size, b.size); i += 1)
        {
            b32 match = (a.str[i] == b.str[i]);
            if(flags & MatchFlag_CaseInsensitive)
            {
                match |= (CharToLower(a.str[i]) == CharToLower(b.str[i]));
            }
            if (flags & MatchFlag_SlashInsensitive)
            {
                match |= (CharToForwardSlash(a.str[i]) == CharToForwardSlash(b.str[i]));
            }
            if (match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

function u64
FindSubStr8(String8 haystack, String8 needle, u64 start_pos, MatchFlags flags)
{
    b32 found = 0;
    u64 foundIDX = haystack.size;
    for (u64 i = start_pos; i < haystack.size; i += 1)
    {
        if (i + needle.size <= haystack.size)
        {
            String8 substr = SubStr8(haystack, R1U64(i, i+needle.size));
            if (Str8Match(substr, needle, flags))
            {
                foundIDX = i;
                found = 1;
                if (!(flags & MatchFlag_FindLast))
                {
                    break;
                }
            }
        }
        else
        {
            break;
        }
    }
    return foundIDX;
}

function b32
Str8EndsIn(String8 haystack, String8 needle)
{
    b32 result = 0;
    if (haystack.size >= needle.size)
    {
        String8 haystackEnd = Str8Advance(haystack, haystack.size - needle.size);
        result = Str8StrictMatch(haystackEnd, needle);
    }
    return result;
}

//- gsp: allocation 
function String8
PushStr8Copy(M_Arena *arena, String8 string)
{
    String8 result;
    result.size = string.size;
    result.str = M_PushArrayNoZero(arena, u8, string.size);
    MemoryCopy(result.str, string.str, result.size);
    return result;
}

function String8
PushStr8FmtVAList(M_Arena *arena, char *fmt, va_list args)
{
    String8 result = {0};
    va_list args2;
    va_copy(args2, args);
    u64 neededBytes = stbsp_vsnprintf(0, 0, fmt, args)+1;
    result.str = M_PushArrayNoZero(arena, u8, neededBytes);
    result.size = neededBytes - 1;
    stbsp_vsnprintf((char*)result.str, U64SafeTruncateToI32(neededBytes), fmt, args2);
    return result;
}

function String8
PushStr8Fmt(M_Arena *arena, char *fmt, ...)
{
    String8 result = {0};
    va_list args;
    va_start(args, fmt);
    result = PushStr8FmtVAList(arena, fmt, args);
    va_end(args);
    return result;
}

function String8
PushStr8FillByte(M_Arena *arena, u64 size, u8 byte)
{
    String8 result = {0};
    result.str = M_PushArrayNoZero(arena, u8, size);
    MemorySet(result.str, byte, size);
    result.size = size;
    return result;
}

//////////////////////
//~ gsp: string lists
inline_function String8Node
S8Node(String8 s)
{
    String8Node node = {0};
    node.string = s;
    return node;
}

function void
Str8ListPushNode(String8List *list, String8Node *n)
{
    QueuePush(list->first, list->last, n);
    list->nodeCount += 1;
    list->totalSize += n->string.size;
}

function void
Str8ListPushNodeFront(String8List *list, String8Node *n)
{
    QueuePushFront(list->first, list->last, n);
    list->nodeCount += 1;
    list->totalSize += n->string.size;
}

function void
Str8ListPush(M_Arena *arena, String8List *list, String8 string)
{
    String8Node *n = M_PushStruct(arena, String8Node);
    n->string = string;
    Str8ListPushNode(list, n);
}

function void
Str8ListPushFront(M_Arena *arena, String8List *list, String8 string)
{
    String8Node *n = M_PushStruct(arena, String8Node);
    n->string = string;
    Str8ListPushNodeFront(list, n);
}

function void
Str8ListConcatInPlace(String8List *list, String8List *toPush)
{
    if (toPush->first)
    {
        list->nodeCount += toPush->nodeCount;
        list->totalSize += toPush->totalSize;
        if (0 == list->last)
        {
            *list = *toPush;
        }
        else
        {
            list->last->next = toPush->first;
            list->last = toPush->last;
        }
    }
    MemoryZero(toPush, sizeof(*toPush));
}

function String8List
StrSplit8(M_Arena *arena, String8 string, u64 splitCount, String8 *splits)
{
    String8List list = {0};
    
    u64 splitStart = 0;
    for(u64 i = 0; i < string.size; i += 1)
    {
        b32 wasSplit = 0;
        for(u64 splitIDX = 0; splitIDX < splitCount; splitIDX += 1)
        {
            b32 match = 0;
            if(i + splits[splitIDX].size <= string.size)
            {
                match = 1;
                for(u64 splitIndex = 0; splitIndex < splits[splitIDX].size && i + splitIndex < string.size; splitIndex += 1)
                {
                    if(splits[splitIDX].str[splitIndex] != string.str[i + splitIndex])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                String8 splitString = Str8(string.str + splitStart, i - splitStart);
                Str8ListPush(arena, &list, splitString);
                splitStart = i + splits[splitIDX].size;
                i += splits[splitIDX].size - 1;
                wasSplit = 1;
                break;
            }
        }
        
        if(wasSplit == 0 && i == string.size - 1)
        {
            String8 splitString = Str8(string.str + splitStart, i+1 - splitStart);
            Str8ListPush(arena, &list, splitString);
            break;
        }
    }
    
    return list;
}

function String8
Str8ListJoin(M_Arena *arena, String8List list, StringJoin *optionalParams)
{
    StringJoin join  = {0};
    if (optionalParams)
    {
        MemoryCopy(&join, optionalParams, sizeof(StringJoin));
    }
    
    // gsp: calculate size & allocate
    u64 sepCount = 0;
    if (list.nodeCount > 1)
    {
        sepCount = list.nodeCount - 1;
    }
    String8 result = {0};
    result.size = (list.totalSize + join.pre.size + sepCount*join.sep.size + join.post.size);
    // gsp: result.size + 1 for null byte
    result.str = M_PushArrayNoZero(arena, u8, result.size + 1);
    
    // gsp: fill
    u8 *ptr = result.str;
    MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for (String8Node *node = list.first; node; node = node->next)
    {
        MemoryCopy(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        if (node != list.last)
        {
            MemoryCopy(ptr, join.sep.str, join.sep.size);
            ptr += join.sep.size;
        }
    }
    MemoryCopy(ptr, join.post.str, join.post.size);
    
    return result;
}

function char **
Str8ListGetStrCArray(M_Arena *arena, String8List list)
{
    char **result = 0;
    if (list.nodeCount > 0)
    {
        result = M_PushArray(arena, char *, list.nodeCount);
        String8Node *node = list.first;
        for (u32 nodeIdx = 0; nodeIdx < list.nodeCount; ++nodeIdx)
        {
            result[nodeIdx] = (char*)node->string.str;
            node = node->next;
        }
    }
    return result;
}

//////////////////////
//~ gsp: Range functions
// TODO(gsp): this function probably needs more checking
function String8
Str8PutRange(String8 str, Rng1U64 r, String8 replacement, u64 maxLen)
{
    Assert(maxLen);
    // Cap size and make range right way around
    if (r.max < r.min)
        Swap(u64, r.max, r.min);
    
    if (r.min == maxLen)
        return str;
    
    Rng1U64 uncappedRange = r;
    r.min = Min(r.min, str.size);
    r.max = Min(r.max, str.size);
    replacement.size = Min(replacement.size, maxLen);
    
    u64 rangeSize = Dim1U64(r);
    // NOTE(gsp): emptySpace not used
    //u64 emptySpace = (maxLen - str.size); // before adding
    u64 newSize = Min(maxLen, str.size - rangeSize + replacement.size);
    u64 sizeAfterPlacement = (maxLen >= r.min + replacement.size) ? maxLen - r.min - replacement.size : 0;
    Assert(maxLen >= sizeAfterPlacement);
    
    // Advance string after r.max
    if (str.size && (uncappedRange.max < str.size) && (replacement.size != rangeSize) && (replacement.size <= sizeAfterPlacement))
    {
        b32 overflow = maxLen <= newSize;
        u64 replEnd = (overflow ? maxLen : newSize) - 1;
        u64 readEnd = (overflow ? r.max + sizeAfterPlacement : str.size) - 1;
        Assert(replEnd != readEnd);
        if (replEnd > readEnd)
        {
            Assert(overflow ? readEnd <= maxLen : 1);
            u64 copyLen = readEnd - r.max + 1; Assert((i64)readEnd - (i64)r.max + 1 > 0);
            for (u64 it = 0; it < copyLen; ++it)
            {
                str.str[replEnd - it] = str.str[readEnd - it];
            }
        }
        else
        {
            u64 readStart = r.max;
            u64 replStart = r.min + replacement.size;
            for (u32 it = 0; it < sizeAfterPlacement; ++it)
            {
                str.str[replStart + it] = str.str[readStart + it];
            }
        }
        
    }
    
    // Copy replacement to string
    for (u64 it = 0; it < replacement.size; ++it)
    {
        str.str[r.min + it] = replacement.str[it];
    }
    
    // Commit to return
    String8 result = str;
    result.size = newSize;
    Assert(result.size <= maxLen);
    return result;
}

#if 0
// Str8PutRange tests
{
    u64 textMaxLen = 10;
    String8 textStr = Str8(M_PushArray(scratch.arena, u8, textMaxLen), 0);
    
    String8 replacement;
    
    replacement = S8Lit("c");
    textStr = Str8PutRange(textStr, R1U64(0, 0), replacement, textMaxLen);
    
    replacement = S8Lit("a");
    textStr = Str8PutRange(textStr, R1U64(1, 1), replacement, textMaxLen);
    
    replacement = S8Lit("a");
    textStr = Str8PutRange(textStr, R1U64(0, 0), replacement, textMaxLen);
    
    replacement = S8Lit("nice");
    textStr = Str8PutRange(textStr, R1U64(0, 0), replacement, textMaxLen);
    
    replacement = S8Lit("1234567890123");
    textStr = Str8PutRange(textStr, R1U64(0, 0), replacement, textMaxLen);
    
    replacement = S8Lit("nice");
    textStr = Str8PutRange(textStr, R1U64(0, 0), replacement, textMaxLen);
    
    replacement = S8Lit("wow");
    textStr = Str8PutRange(textStr, R1U64(1, 3), replacement, textMaxLen);
    
    replacement = S8Lit("abcd");
    textStr = Str8PutRange(textStr, R1U64(5, 9), replacement, textMaxLen);
    
    replacement = S8Lit("boom");
    textStr = Str8PutRange(textStr, R1U64(0, textMaxLen), replacement, textMaxLen);
    
    replacement = S8Lit("_oo_");
    textStr = Str8PutRange(textStr, R1U64(1, 3), replacement, textMaxLen);
    
    replacement = S8Lit("boom");
    textStr = Str8PutRange(textStr, R1U64(0, textMaxLen), replacement, textMaxLen);
    
    replacement = S8Lit("123456789");
    textStr = Str8PutRange(textStr, R1U64(1, 3), replacement, textMaxLen);
    
    replacement = S8Lit("gugu");
    textStr = Str8PutRange(textStr, R1U64(7, 6), replacement, textMaxLen);
    
    replacement = S8Lit("meow");
    textStr = Str8PutRange(textStr, R1U64(4, 2), replacement, textMaxLen);
    
    replacement = S8Lit("xyz");
    textStr = Str8PutRange(textStr, R1U64(6, 0), replacement, textMaxLen);
    
    replacement = S8Lit("");
    textStr = Str8PutRange(textStr, R1U64(0, 2), replacement, textMaxLen);
    
    replacement = S8Lit("");
    textStr = Str8PutRange(textStr, R1U64(0, textMaxLen), replacement, textMaxLen);
    
    replacement = S8Lit("12345");
    textStr = Str8PutRange(textStr, R1U64(1, 3), replacement, textMaxLen);
    
    replacement = S8Lit("12345");
    textStr = Str8PutRange(textStr, R1U64(3, 20), replacement, textMaxLen);
}
#endif

//////////////////////
//~ gsp: Path Helpers

function String8
Str8PathChopLastPeriod(String8 string)
{
    u64 periodPos = FindSubStr8(string, S8Lit("."), 0, MatchFlag_FindLast);
    if (periodPos < string.size)
    {
        string.size = periodPos;
    }
    return string;
}

function String8
Str8PathChopPastLastSlash(String8 string)
{
    u64 slashPos = FindSubStr8(string, S8Lit("/"), 0, MatchFlag_SlashInsensitive | MatchFlag_FindLast);
    if (slashPos < string.size)
    {
        string.size = slashPos;
    }
    return string;
}

function String8
Str8PathSkipLastPeriod(String8 string)
{
    u64 lastPeriodPos = FindSubStr8(string, S8Lit("."), 0, MatchFlag_FindLast);
    if (lastPeriodPos < string.size)
    {
        string.size -= lastPeriodPos + 1;
        string.str += lastPeriodPos + 1;
    }
    return string;
}

function String8
Str8PathSkipLastSlash(String8 string)
{
    u64 lastSlashPos = FindSubStr8(string, S8Lit("/"), 0, MatchFlag_SlashInsensitive | MatchFlag_FindLast);
    if (lastSlashPos < string.size)
    {
        string.size -= lastSlashPos + 1;
        string.str += lastSlashPos + 1;
    }
    return string;
}

function PathKind
PathKindFromStr8(String8 path)
{
    PathKind kind = PathKind_Relative;
    if(path.size >= 1 && path.str[0] == '/')
    {
        kind = PathKind_RootAbsolute;
    }
    if(path.size >= 2 && CharIsAlpha(path.str[0]) && path.str[1] == ':')
    {
        kind = PathKind_DriveAbsolute;
    }
    return kind;
}

function void
Str8PathCurateBackslashes(String8 string)
{
    for (u64 i = 0; i < string.size; ++i)
    {
        if (string.str[i] == '\\')
            string.str[i] = '/';
    }
}

//////////////////////
//~ gsp: Unicode conversions

// Reference video that helped me understand:
// https://www.youtube.com/watch?v=uTJoJtNYcaQ

read_only global u8 utf8_class[32] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0-16
    0,0,0,0,0,0,0,0,                 // 17-24
    2,2,2,2,                         // 25-28
    3,3,                             // 29-30
    4,                               // 31
    5,                               // 32
};

function DecodedCodepoint
DecodeCodepointFromUtf8(u8 *str, u64 strLen)
{
    DecodedCodepoint result = {~((u32)0), 1};
    u8 byte = str[0];
    u8 byteClass = utf8_class[byte >> 3];
    switch(byteClass)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= strLen)
            {
                u8 contByte = str[1];
                if (0 == utf8_class[contByte >> 3])
                {
                    result.codepoint = (byte & bitmask5) << 6;
                    result.codepoint |= (contByte & bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= strLen)
            {
                u8 contByte[2] = { str[1], str[2] };
                if (0 == utf8_class[contByte[0] >> 3] &&
                    0 == utf8_class[contByte[1] >> 3])
                {
                    result.codepoint = ((byte & bitmask4)  << 12);
                    result.codepoint |= ((byte & bitmask6) << 6);
                    result.codepoint |= (byte & bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= strLen)
            {
                u8 contByte[3] = { str[1], str[2], str[3] };
                if (0 == utf8_class[contByte[0] >> 3] &&
                    0 == utf8_class[contByte[1] >> 3] &&
                    0 == utf8_class[contByte[2] >> 3])
                {
                    result.codepoint = ((byte & bitmask3)  << 18);
                    result.codepoint |= ((byte & bitmask6) << 12);
                    result.codepoint |= ((byte & bitmask6) << 6);
                    result.codepoint |= (byte & bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    return result;
}

function DecodedCodepoint
DecodeCodepointFromUtf16(u16 *str, u64 strLen)
{
    DecodedCodepoint result = {~((u32)0), 1};
    result.codepoint = str[0];
    result.advance = 1;
    if(1 < strLen       &&
       0xD800 <= str[0] &&
       str[0] < 0xDC00  &&
       0xDC00 <= str[1] &&
       str[1] < 0xE000)
    {
        result.codepoint = ((str[0] - 0xD800) << 10) | (str[1] - 0xDC00) + 0x10000;
        result.advance = 2;
    }
    return result;
}

function u32
Utf8FromCodepoint(u8 *out, u32 codepoint)
{
    u32 advance = 1;
    if (codepoint < 0x80)
    {
        out[0] = (u8)codepoint;
        advance = 1;
    }
    else if (codepoint < 0x800)
    {
        out[0] = (bitmask2 << 6) | ((codepoint >> 6) & bitmask5);
        out[1] = (0b10 << 6) | (codepoint & bitmask6);
        advance = 2;
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (0b1110 << 4) | ((codepoint >> 12) & bitmask4);
        out[1] =   (0b10 << 6) | ((codepoint >> 6) & bitmask6);
        out[2] =   (0b10 << 6) | (codepoint & bitmask6);
        advance = 3;
    }
    else if(codepoint <= 0x10FFFF)
    {
        out[0] = (bitmask4 << 4) | ((codepoint >> 18) & bitmask3);
        out[1] =     (0b10 << 6) | ((codepoint >> 12) & bitmask6);
        out[2] =     (0b10 << 6) | ((codepoint >>  6) & bitmask6);
        out[3] =     (0b10 << 6) | ( codepoint        & bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return advance;
}

function u32
Utf16FromCodepoint(u16 *out, u32 codepoint)
{
    u32 advance = 1;
    if(codepoint == ~((u32)0))
    {
        out[0] = (u16)'?';
    }
    else if(codepoint < 0x10000)
    {
        out[0] = (u16)codepoint;
    }
    else
    {
        u64 v = codepoint - 0x10000;
        out[0] = (u16)(0xD800 + (v >> 10));
        out[1] = 0xDC00 + (v & bitmask10);
        advance = 2;
    }
    return advance;
}

function String8         
String8From16(M_Arena *arena, String16 in)
{
    u64 cap = in.size*3;
    u8 *str = M_PushString(arena, u8, cap);
    u16 *ptr = in.str;
    u16 *opl = ptr + in.size;
    u64 size = 0;
    DecodedCodepoint consume;
    for(;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    M_ArenaPop(arena, cap - size); // := ((cap + 1) - (size + 1))
    return Str8(str, size);
}


function String16        
String16From8(M_Arena *arena, String8 in)
{
    u64 maxSize = in.size*2;
    u16 *str = M_PushString(arena, u16, maxSize);
    u8 *ptr = in.str;
    u8 *opl = ptr + in.size;
    u64 size = 0;
    DecodedCodepoint consume;
    for(;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    M_ArenaPop(arena, 2*(maxSize - size)); // := 2*((maxSize + 1) - (size + 1))
    String16 result = {size, str};
    return result;
}

function String32
String32From8(M_Arena *arena, String8 in)
{
    u64 cap = in.size;
    u32 *str = M_PushArrayNoZero(arena, u32, cap + 1);
    u8 *ptr = in.str;
    u8 *opl = ptr + in.size;
    u64 size = 0;
    DecodedCodepoint consume;
    for(;ptr < opl;)
    {
        consume = DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    M_ArenaPop(arena, 4*(cap - size)); // := 4*((cap + 1) - (size + 1))
    String32 result = {size, str};
    return result;
}

//////////////////////
//~ BOTTOM
#endif // #ifdef BASE_STRING_INCLUDE_C