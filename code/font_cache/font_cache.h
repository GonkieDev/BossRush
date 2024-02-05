/* date = August 17th 2023 10:32 pm */

#ifndef FONT_CACHE_INCLUDE_H
#define FONT_CACHE_INCLUDE_H

#if !defined(FC_LOG_FILE)
# define FC_LOG_FILE 0
#endif

/////////////////////////////////////////////////////////
//~ gsp: Types

typedef struct FC_GlyphInfo FC_GlyphInfo;
struct FC_GlyphInfo
{
    R_TexHandle texture;
    Rng2S32 uv;
    f32 advanceWidth;
    f32 advanceHeight;
    HMM_Vec2 dim;
};

typedef struct FC_GlyphInfoTableEntry FC_GlyphInfoTableEntry;
struct FC_GlyphInfoTableEntry
{
    FC_GlyphInfo info;
    u32 hash; // hash without being modulo'd 
    b32 used;
};

typedef struct FC_GlyphInfoTable FC_GlyphInfoTable;
struct FC_GlyphInfoTable
{
    u32 entriesCount;
    u32 maxEntriesCount;
    FC_GlyphInfoTableEntry *infos;
};


/////////////////////////////////////////////////////////
//~ Atlas

typedef struct FC_Atlas FC_Atlas; 
struct FC_Atlas
{
    M_Atlas     *allocator;
    R_TexHandle  texture;
};

typedef struct FC_AtlasNode FC_AtlasNode; 
struct FC_AtlasNode
{
    FC_Atlas      atlas;
    FC_AtlasNode *next;
};

typedef struct FC_AtlasList FC_AtlasList;
struct FC_AtlasList
{
    FC_AtlasNode *first;
    u64           count;
};

typedef struct FC_AtlasRegionAllocResult FC_AtlasRegionAllocResult;
struct FC_AtlasRegionAllocResult
{
    FC_Atlas *atlas;
    Rng2S32 region;
};

/////////////////////////////////////////////////////////
//~ gsp: Run
typedef struct FC_GlyphRun FC_GlyphRun;
struct FC_GlyphRun
{
    FP_GlyphIndicesArray indicesArray;
    FC_GlyphInfo **infos;
    f32 *kerningAdjustments;
    union
    {
        struct
        {
            f32 axisAdvance[Axis2_COUNT];
        };
        struct
        {
            f32 advance;
            f32 height;
        };
    };
};

/////////////////////////////////////////////////////////
//~ gsp: FC State
typedef struct FC_State FC_State; 
struct FC_State
{
    M_Arena *arena;
    FC_AtlasList atlasList;
    FC_GlyphInfoTable glyphInfosTable;
};

/////////////////////////////////////////////////////////
//~ gsp: API

function b32           FC_Init(Vec2S32 atlasSize);
function void          FC_Shutdown(void);
function FC_GlyphInfo *FC_GetGlyphInfo(FP_Handle font, u16 glyphIndex, f32 size, Vec2S32 alignment);

/////////////////////////////////////////////////////////
//~ gsp: Glyph functions
function u32           FC_HashGlyph(FP_Handle font, u16 glyphIndex, f32 size);

/////////////////////////////////////////////////////////
//~ gsp: GlyphRun functions
function FC_GlyphRun   FC_GlyphRunFromString(M_Arena *arena, FP_Handle font, f32 size, String8 string, Vec2S32 alignment);

/////////////////////////////////////////////////////////
//~ gsp: Atlas functions
function FC_AtlasNode *FC_AtlasNodeInit(Vec2S32 atlasSize);
function void          FC_AddAtlasToList(FC_AtlasNode *node);
function FC_AtlasRegionAllocResult FC_AtlasRegionAlloc(Vec2S32 size);

/////////////////////////////////////////////////////////
//~ End of file
#endif //FONT_CACHE_INCLUDE_H
