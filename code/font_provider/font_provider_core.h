/* date = August 10th 2023 4:10 pm */

#ifndef FONT_PROVIDER_CORE_INCLUDE_H
#define FONT_PROVIDER_CORE_INCLUDE_H

//////////////////////////////////////////////////////
//~ Types
typedef struct FP_Handle FP_Handle;
struct FP_Handle
{
    u64 a[1];
};

typedef struct FP_Metrics FP_Metrics;
struct FP_Metrics
{
    f32 lineGap;
    f32 ascent;
    f32 descent;
    f32 capitalHeight;
};

typedef struct FP_RasterResult FP_RasterResult;
struct FP_RasterResult
{
    Vec2S32 atlasDim;
    Vec2S32 notAlignedAtlasDim;
    void *atlas;
    f32 advance;
    f32 height;
};

typedef struct FP_GlyphIndicesArray FP_GlyphIndicesArray;
struct FP_GlyphIndicesArray
{
    u64 indicesCount;
    u16 *indices;
};

//////////////////////////////////////////////////////
//~ Defines
#define fp_function no_name_mangle
#define fp_global static

//////////////////////////////////////////////////////
//~ Funcions
fp_function b32                  FP_Init(void);
fp_function FP_Handle            FP_LoadFont(String8 filepath);
fp_function void                 FP_CloseFont(FP_Handle handle);
fp_function FP_Metrics           FP_MetricsFromFont(FP_Handle fontHandle, f32 size);
fp_function FP_GlyphIndicesArray FP_GlyphIndicesFromString(M_Arena *arena, FP_Handle fontHandle, String8 string);
fp_function f32 *                FP_GetKerningPairAdjustments(M_Arena *arena, FP_Handle fontHandle, f32 size, FP_GlyphIndicesArray indices);
fp_function FP_RasterResult      FP_Raster(M_Arena *arena, FP_Handle fontHandle, f32 size, FP_GlyphIndicesArray indicesArray, Vec2S32 granularity);

//- Helpers 
fp_function b32 FP_IsHandleValid(FP_Handle handle);


//////////////////////////////////////////////////////
//~ End of file
#endif //FONT_PROVIDER_CORE_INCLUDE_H
