#ifndef FONT_CACHE_INCLUDE_C
#define FONT_CACHE_INCLUDE_C

#include "meow_hash_inc.h"

/////////////////////////////////////////////////////////
//~ gsp: Globals

#if BUILD_ROOT
global FC_State *fc_state;
#endif

/////////////////////////////////////////////////////////
//~ gsp: API functions

function b32
FC_Init(Vec2S32 atlasSize)
{
    if (T_IsMainThread() && (0 == fc_state))
    {
        {
            M_Arena *arena = M_ArenaAllocDefault();
            if (arena)
            {
                fc_state = M_PushStruct(arena, FC_State);
                fc_state->arena = arena;
            }
            else
            {
                LogFatal(FC_LOG_FILE, "Failed  to allocate font cache state arena.");
            }
        }
        
        // Init table
        {
            FC_GlyphInfoTable *table = &fc_state->glyphInfosTable;
            table->entriesCount    = 0;
            table->maxEntriesCount = 2000;
            table->infos           = M_PushArray(fc_state->arena, FC_GlyphInfoTableEntry, table->maxEntriesCount);
        }
        
        // Pre init 1 atlas node
        {
            FC_AtlasNode *node = FC_AtlasNodeInit(atlasSize);
            if (node)
            {
                FC_AddAtlasToList(node);
            }
            else
            {
                LogError(FC_LOG_FILE, "Failed to init initial font cache atlas node.");
                return 0;
            }
        }
        
    }
    
    return 1;
}

function void
FC_Shutdown(void)
{
    if (fc_state)
    {
        for (FC_AtlasNode *node = fc_state->atlasList.first;
             node != 0;
             node = node->next)
        {
            R_Texture2DRelease(node->atlas.texture);
        }
        M_ArenaRelease(fc_state->arena);
    }
}

// TODO(gsp): @FC verify alignment works with different alignments
function FC_GlyphInfo *
FC_GetGlyphInfo(FP_Handle font, u16 glyphIndex, f32 size, Vec2S32 alignment)
{
    FC_GlyphInfo *info = 0;
    FC_GlyphInfoTable *table = &fc_state->glyphInfosTable; 
    
    u32 hash = FC_HashGlyph(font, glyphIndex, size);
    
    FC_GlyphInfoTableEntry *entry = 0;
    for (u32 it = 0, index = hash % table->maxEntriesCount;
         it < table->maxEntriesCount;
         it += 1, index = (index + 1) % table->maxEntriesCount)
    {
        FC_GlyphInfoTableEntry *currEntry = &table->infos[index];
        if ((!currEntry->used) || (currEntry->used && (currEntry->hash == hash)))
        {
            entry = currEntry;
            break;
        }
    }
    
    info = &entry->info;
    if (!entry->used)
    {
        ScratchArenaScope(scratch, 0, 0)
        {
            FP_GlyphIndicesArray indicesArray = { .indicesCount = 1, .indices = &glyphIndex };
            FP_RasterResult rasterResult = FP_Raster(scratch.arena, font, size, indicesArray, alignment);
            Assert((alignment.x == 0) || ((rasterResult.atlasDim.x % alignment.x) == 0));
            Assert((alignment.y == 0) || ((rasterResult.atlasDim.y % alignment.y) == 0));
            
            FC_AtlasRegionAllocResult regionAllocResult =
                FC_AtlasRegionAlloc(Add2S32(rasterResult.atlasDim, alignment));
            
            FC_Atlas *atlas = regionAllocResult.atlas;
            Rng2S32 atlasRegion  = regionAllocResult.region;
            
            Rng2S32 fillUV = {0};
            {
                fillUV.p0 = atlasRegion.p0;
                if (alignment.x != 0 && atlasRegion.p0.x != 0)
                    fillUV.p0.x += alignment.x - (fillUV.p0.x % alignment.x);
                if (alignment.y != 0 && atlasRegion.p0.y != 0)
                    fillUV.p0.y += alignment.y - (fillUV.p0.y % alignment.y);
                fillUV.p1 = Add2S32(fillUV.p0, rasterResult.atlasDim);
                
                Assert(Dim2S32(fillUV).x == rasterResult.atlasDim.x);
                Assert(Dim2S32(fillUV).y == rasterResult.atlasDim.y);
            }
            
            Rng2S32 displayUV = {0};
            {
                displayUV.p0 = fillUV.p0;
                displayUV.p1 = Add2S32(displayUV.p0, rasterResult.notAlignedAtlasDim);
            }
            
            if (atlas)
            {
                R_Texture2DFillRegion(atlas->texture, fillUV, rasterResult.atlas);
                
                info->advanceWidth  = rasterResult.advance;
                info->advanceHeight = rasterResult.height;
                info->dim = HMM_V2((f32)rasterResult.notAlignedAtlasDim.x, (f32)rasterResult.notAlignedAtlasDim.y);
                info->texture = atlas->texture;
                info->uv = displayUV;
                
                entry->used = 1;
                entry->hash = hash;
                
                ++table->entriesCount;
            }
        }
    }
    
    return info;
}

/////////////////////////////////////////////////////////
//~ gsp: Glyph functions
function u32
FC_HashGlyph(FP_Handle font, u16 glyphIndex, f32 size)
{
    u64 buffer[3];
    buffer[0] = font.a[0];
    buffer[1] = (u64)glyphIndex;
    buffer[2] = (u64)size;
    
    meow_u128 hash = MeowHash(MeowDefaultSeed, sizeof(buffer), buffer);
    u32 hash32 = MeowU32From(hash, 0);
    return hash32;
}

/////////////////////////////////////////////////////////
//~ gsp: GlyphRun functions
function FC_GlyphRun   
FC_GlyphRunFromString(M_Arena *arena, FP_Handle font, f32 size, String8 string, Vec2S32 alignment)
{
    FC_GlyphRun run = {0};
    run.indicesArray = FP_GlyphIndicesFromString(arena, font, string);
    run.kerningAdjustments = FP_GetKerningPairAdjustments(arena, font, size, run.indicesArray);
    run.infos = M_PushArray(arena, FC_GlyphInfo*, string.size);
    for (u32 it = 0; it < string.size; ++it)
    {
        run.infos[it] = FC_GetGlyphInfo(font, run.indicesArray.indices[it], size, alignment);
        run.advance += run.infos[it]->advanceWidth + run.kerningAdjustments[it];
        if (run.height < run.infos[it]->advanceHeight)
        {
            run.height = run.infos[it]->advanceHeight;
        }
    }
    return run;
}

/////////////////////////////////////////////////////////
//~ gsp: Atlas functions
function FC_AtlasNode *
FC_AtlasNodeInit(Vec2S32 atlasSize)
{
    FC_AtlasNode node = {0};
    node.atlas.allocator = M_AtlasInit(fc_state->arena, atlasSize);
    if (!node.atlas.allocator)
        return 0;
    
    node.atlas.texture = R_Texture2DAlloc(atlasSize, 
                                          R_Texture2DFormat_R8_G8_B8_A8, 
                                          R_Texture2DSampling_Billinear, 
                                          0);
    
#if R_BACKEND_GL
    R_GL_NameTexture(node.atlas.texture, "(Tex) Font Cache Atlas");
#endif
    
    if (!R_TexHandleIsValid(node.atlas.texture))
        return 0;
    
    // All good - allocate
    FC_AtlasNode *allocNode = M_PushStruct(fc_state->arena, FC_AtlasNode);
    MemoryCopyStruct(allocNode, &node);
    
    return allocNode;
}

function void
FC_AddAtlasToList(FC_AtlasNode *node)
{
    FC_AtlasList *list = &fc_state->atlasList;
    StackPush(list->first, node);
    ++list->count;
}

function FC_AtlasRegionAllocResult
FC_AtlasRegionAlloc(Vec2S32 size)
{
    FC_AtlasRegionAllocResult result = {0};
    for (FC_AtlasNode *node = fc_state->atlasList.first; node != 0; node = node->next)
    {
        M_Atlas *allocator = node->atlas.allocator;
        Rng2S32 region = M_AtlasRegionAlloc(fc_state->arena, allocator, size);
        if (!R2IsNil(region))
        {
            result.region = region;
            result.atlas = &node->atlas;
            break;
        }
    }
    
    if (!result.atlas)
    {
        // TODO(gsp): add size check here?
        FC_AtlasNode *node = FC_AtlasNodeInit(size);
        FC_AddAtlasToList(node);
        result.atlas = &node->atlas;
        result.region = M_AtlasRegionAlloc(fc_state->arena, node->atlas.allocator, size);
    }
    
    return result;
}

/////////////////////////////////////////////////////////
//~ End of file
#endif //FONT_CACHE_INCLUDE_H