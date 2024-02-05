#ifndef FONT_PROVIDER_INC_INCLUDE_C
#define FONT_PROVIDER_INC_INCLUDE_C

//////////////////////////////////////////////////////
//~ [h] includes
#include "font_provider_core_dwrite.hpp"
#include "os/os_inc.h"

// TODO(gsp): handle "size" of fonts more ellegantly, and centralise getting dpi

//////////////////////////////////////////////////////
//~ gsp: Globals
fp_global FP_DW_State *fp_dw_state;

//////////////////////////////////////////////////////
//~ gsp: font provider hooks

fp_function
b32 FP_Init(void)
{
 b32 result = 1;
 if (T_IsMainThread() && (fp_dw_state == 0))
 {
  //- gsp: allocate/prep global state
  {
   M_Arena *arena = M_ArenaAlloc(GB(4));
   fp_dw_state = M_PushStruct(arena, FP_DW_State);
   fp_dw_state->arena = arena;
  }
  
  HRESULT error;
  
  //- Create dwrite factory 
  error = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), (IUnknown**)&fp_dw_state->dwFactory);
  FP_DW_CheckPtr(error, fp_dw_state->dwFactory, result = 0);
  
  //- Register font file loader
#if 0
  if (result)
  {
   error = fp_dw_state->dwFactory->RegisterFontFileLoader(fp_g_dwFontFileLoader);
   FP_DW_CheckPtr(error, fp_dw_state->dwFontFileLoader, result = 0);
  }
#endif
  
  //- Default rendering params
  if (result)
  {
   error = fp_dw_state->dwFactory->CreateRenderingParams(&fp_dw_state->dwDefaultRenderingParams);
   FP_DW_CheckPtr(error, fp_dw_state->dwDefaultRenderingParams, result = 0);
  }
  
  //- Rendering params 
  if (result)
  {
   FLOAT gamma = fp_dw_state->dwDefaultRenderingParams->GetGamma();
   FLOAT enhancedContrast = fp_dw_state->dwDefaultRenderingParams->GetEnhancedContrast();
   FLOAT clearTypeLevel = fp_dw_state->dwDefaultRenderingParams->GetClearTypeLevel();
   error = fp_dw_state->dwFactory->CreateCustomRenderingParams(gamma, enhancedContrast, clearTypeLevel, DWRITE_PIXEL_GEOMETRY_FLAT, DWRITE_RENDERING_MODE_DEFAULT, &fp_dw_state->dwRenderingParams);
   FP_DW_CheckPtr(error, fp_dw_state->dwRenderingParams, result = 0);
  }
  
  //- GDI Interop 
  if (result)
  {
   error = fp_dw_state->dwFactory->GetGdiInterop(&fp_dw_state->dwGdiInterop);
   FP_DW_CheckPtr(error, fp_dw_state->dwGdiInterop, result = 0);
  }
 }
 return result;
}

fp_function FP_Handle
FP_LoadFont(String8 filepath)
{
 FP_DW_Font dwFont = {};
 ScratchArenaScope(scratch, 0, 0)
 {
  HRESULT error = S_OK;
  b32 success = 1;
  String16 filepath16 = String16From8(scratch.arena, filepath);
  
  IDWriteFontFile *dwFontFile = 0;
  error = fp_dw_state->dwFactory->CreateFontFileReference((WCHAR*)filepath16.str, 0, &dwFontFile);
  FP_DW_CheckPtr(error, dwFontFile, success = 0);
  
  if (success)
  {
   IDWriteFontFace *fontFace0 = 0;
   error = fp_dw_state->dwFactory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1, &dwFontFile, 0, DWRITE_FONT_SIMULATIONS_NONE, &fontFace0);
   error = fontFace0->QueryInterface(&dwFont.fontFace);
   FP_DW_CheckPtr(error, dwFont.fontFace, success = 0);
  }
  
  if (dwFontFile)
  {
   dwFontFile->Release();
  }
 }
 return FP_HandleFromDWFont(dwFont);
}

fp_function void
FP_CloseFont(FP_Handle handle)
{
 FP_DW_Font dwFont = FP_DWFontFromHandle(handle);
 if (dwFont.fontFace)
 {
  dwFont.fontFace->Release();
 }
}

fp_function FP_Metrics
FP_MetricsFromFont(FP_Handle fontHandle, f32 size)
{
 FP_DW_Font dwFont = FP_DWFontFromHandle(fontHandle);
 return FP_MetricsFromDWFont(dwFont, size);
}

fp_function FP_GlyphIndicesArray
FP_GlyphIndicesFromString(M_Arena *arena, FP_Handle fontHandle, String8 string)
{
 FP_GlyphIndicesArray array = {};
 ScratchArenaScope(scratch, &arena, 1)
 {
  FP_DW_Font font = FP_DWFontFromHandle(fontHandle);
  if (font.fontFace)
  {
   String32 str32 = String32From8(scratch.arena, string);
   array.indicesCount = str32.size;
   array.indices = M_PushArray(arena, u16, array.indicesCount);
   HRESULT error = font.fontFace->GetGlyphIndices(str32.str, U64SafeTruncateToU32(str32.size), array.indices);
   Assert(error == S_OK);
  }
 }
 return array;
}

fp_function f32 *
FP_GetKerningPairAdjustments(M_Arena *arena, FP_Handle fontHandle, f32 size, FP_GlyphIndicesArray indices)
{
 size *= 96.f/72.f; // @dpi
 
 f32 *result = M_PushArray(arena, f32, indices.indicesCount);
 FP_DW_Font font = FP_DWFontFromHandle(fontHandle);
 if(font.fontFace && (font.fontFace->HasKerningPairs() == TRUE))
 {
  DWRITE_FONT_METRICS fontMetrics = {};
  font.fontFace->GetMetrics(&fontMetrics);
  f32 designUnitsPerEm = (f32)fontMetrics.designUnitsPerEm;
  
  ScratchArenaScope(scratch, &arena, 1)
  {
   i32 *advanceAdjustments = M_PushArray(scratch.arena, i32, indices.indicesCount);
   font.fontFace->GetKerningPairAdjustments(U64SafeTruncateToU32(indices.indicesCount), indices.indices, advanceAdjustments);
   for (u32 it = 0; it < indices.indicesCount; ++it)
   {
    result[it] = (f32)advanceAdjustments[it] * size / designUnitsPerEm;
   }
  }
 }
 return result;
}

fp_function FP_RasterResult
FP_Raster(M_Arena *arena, FP_Handle fontHandle, f32 size, FP_GlyphIndicesArray indicesArray, Vec2S32 granularity)
{
 FP_RasterResult result = {};
 ScratchArenaScope(scratch, 0, 0)
 {
  FP_DW_Font font = FP_DWFontFromHandle(fontHandle);
  HRESULT error;
  COLORREF bg_color = RGB(0, 0, 0);
  COLORREF fg_color = RGB(255, 255, 255);
  f32 dpi = 96.f;
  
  DWRITE_FONT_METRICS fontMetrics = {};
  if(font.fontFace)
  {
   font.fontFace->GetMetrics(&fontMetrics);
  }
  f32 designUnitsPerEm = (f32)fontMetrics.designUnitsPerEm;
  
  DWRITE_GLYPH_METRICS *glyphsMetrics = M_PushArrayNoZero(scratch.arena, DWRITE_GLYPH_METRICS, indicesArray.indicesCount);
  if(font.fontFace)
  {
   error = font.fontFace->GetGdiCompatibleGlyphMetrics(size * (dpi/72.f), 1.f, 0, 1, indicesArray.indices, U64SafeTruncateToU32(indicesArray.indicesCount), glyphsMetrics, 0);
  }
  
  //- gsp: derive info from metrics 
  f32 advance = 0;
  Vec2S32 atlasDim = {};
  if(font.fontFace)
  {
   atlasDim.y = (i32)((dpi/72.f) * size * (f32)(fontMetrics.ascent + fontMetrics.descent) / designUnitsPerEm);
   for (u32 idx = 0; idx < indicesArray.indicesCount; ++idx)
   {
    DWRITE_GLYPH_METRICS *currGlyphMetric = &glyphsMetrics[idx];
    UnusedVariable(currGlyphMetric);
    f32 glyphAdvanceWidth  = (dpi/72.f) * size * (f32)glyphsMetrics->advanceWidth  / designUnitsPerEm;
    f32 glyphAdvanceHeight = (dpi/72.f) * size * (f32)glyphsMetrics->advanceHeight / designUnitsPerEm;
    UnusedVariable(glyphAdvanceHeight);
    advance += (f32)glyphAdvanceWidth;
    atlasDim.x = (i32)Max((f32)atlasDim.x, advance);
   }
   atlasDim.x += 7;
   atlasDim.x -= atlasDim.x%8;
   atlasDim.x += 4;
   atlasDim.y += 2;
  }
  
  //- gsp: Make dwrite bitmap for rendering
  IDWriteBitmapRenderTarget *renderTarget = 0;
  if (fp_dw_state->dwGdiInterop)
  {
   error = fp_dw_state->dwGdiInterop->CreateBitmapRenderTarget(0, (u32)atlasDim.x, (u32)atlasDim.y, &renderTarget);
   renderTarget->SetPixelsPerDip(1.f);
  }
  
  //- gsp: Get bitmap & clear
  HDC dc = 0;
  if (renderTarget)
  {
   dc = renderTarget->GetMemoryDC();
   HGDIOBJ original = SelectObject(dc, GetStockObject(DC_PEN));
   SetDCPenColor(dc, bg_color);
   SelectObject(dc, GetStockObject(DC_BRUSH));
   SetDCBrushColor(dc, bg_color);
   Rectangle(dc, 0, 0, atlasDim.x, atlasDim.y);
   SelectObject(dc, original);
  }
  
  //- gsp: Draw glyph run
  HMM_Vec2 drawPos;
  drawPos.x = 1.f;
  drawPos.y = ((f32)atlasDim.y - 2.f);
  
  if (font.fontFace)
  {
   f32 descent = (dpi / 72.f) * size * fontMetrics.descent / designUnitsPerEm;
   drawPos.y -= descent;
  }
  DWRITE_GLYPH_RUN glyphRun = {};
  if (font.fontFace)
  {
   glyphRun.fontFace     = font.fontFace;
   glyphRun.fontEmSize   = size * dpi / 72.f;
   glyphRun.glyphCount   = U64SafeTruncateToU32(indicesArray.indicesCount);
   glyphRun.glyphIndices = indicesArray.indices;
  }
  RECT boundingBox = {};
  if (font.fontFace)
  {
   error = renderTarget->
    DrawGlyphRun(drawPos.x, drawPos.y, DWRITE_MEASURING_MODE_NATURAL, &glyphRun, fp_dw_state->dwRenderingParams, fg_color, &boundingBox);
  }
  
  DIBSECTION dib = {};
  if (font.fontFace)
  {
   HBITMAP bitmap = (HBITMAP)GetCurrentObject(dc, OBJ_BITMAP);
   GetObject(bitmap, sizeof(dib), &dib);
  }
  
  if (font.fontFace)
  {
   result = {};
   Vec2S32 alignedAtlasDim = atlasDim;
   {
    result.notAlignedAtlasDim = atlasDim; 
    if (granularity.x != 0)
    {
     alignedAtlasDim.x = atlasDim.x + (granularity.x - (atlasDim.x % granularity.x));
    }
    if (granularity.y != 0)
    {
     alignedAtlasDim.y = atlasDim.y + (granularity.y - (atlasDim.y % granularity.y));
    }
    result.atlasDim = alignedAtlasDim;
    result.atlas = M_PushArray(arena, u8, alignedAtlasDim.x * alignedAtlasDim.y * 4);
   }
   result.advance = advance;
   result.height = (f32)boundingBox.bottom + 4.f;
   
   // gsp: fill atlas
   {
    u8 *inData   = (u8*)dib.dsBm.bmBits;
    u32 inPitch  = (u32)dib.dsBm.bmWidthBytes;
    u8 *outData  = (u8*)result.atlas;
    u32 outPitch = (u32)alignedAtlasDim.x * 4;
    
    u8 *inLine  = inData;
    u8 *outLine = outData;
    for (i32 y = 0; y < atlasDim.y; ++y)
    {
     u8 *inPixel  = inLine;
     u8 *outPixel = outLine;
     for (i32 x = 0; x < atlasDim.x; ++x)
     {
#if 1
      outPixel[0] = 255;
      outPixel[1] = 255;
      outPixel[2] = 255;
      outPixel[3] = inPixel[0];
#else
      outPixel[0] = inPixel[0];
      outPixel[1] = inPixel[1];
      outPixel[2] = inPixel[2];
      outPixel[3] = inPixel[0];
#endif
      
      inPixel  += 4;
      outPixel += 4;
     }
     inLine  += inPitch;
     outLine += outPitch;
    }
   }
  }
  
  renderTarget->Release();
 }
 return result;
}

fp_function b32
FP_IsHandleValid(FP_Handle handle)
{
 return (handle.a[0] != 0);
}

//////////////////////////////////////////////////////
//~ Helpers
fp_function FP_Handle
FP_HandleFromDWFont(FP_DW_Font font)
{
 FP_Handle handle = {};
 handle.a[0] = (u64)font.fontFace;
 return handle;
}

fp_function FP_DW_Font
FP_DWFontFromHandle(FP_Handle handle)
{
 FP_DW_Font dwFont = {};
 dwFont.fontFace = (IDWriteFontFace1 *)handle.a[0];
 return dwFont;
}

//////////////////////////////////////////////////////
//~ DWrite functions
fp_function FP_Metrics
FP_MetricsFromDWFont(FP_DW_Font dwFont, f32 size)
{
 DWRITE_FONT_METRICS dwFontMetrics = {};
 if(dwFont.fontFace)
 {
  dwFont.fontFace->GetMetrics(&dwFontMetrics);
 }
 
 FP_Metrics result = {};
 if(dwFont.fontFace)
 {
  // NOTE(gsp): 96.f is default DPI
  // NOTE(gsp): Inch/point is always 72.f
  f32 designUnitsPerEm = (f32)dwFontMetrics.designUnitsPerEm;
  f32 coeff = (96.f/72.f) * size;
  result.lineGap = coeff * (f32)dwFontMetrics.lineGap / designUnitsPerEm;
  result.ascent  = coeff * (f32)dwFontMetrics.ascent / designUnitsPerEm;
  result.descent = coeff * (f32)dwFontMetrics.descent / designUnitsPerEm;
  result.capitalHeight = coeff * (f32)dwFontMetrics.capHeight / designUnitsPerEm;
 }
 return result;
}

//////////////////////////////////////////////////////
//~ [c] includes
#pragma warning(push)
#pragma warning(disable: 4668 4820 4042 5246 4365 5246 4365 4505 4514)

#include "base/base_inc.c"
#include "os/os_inc.c"

#pragma warning(pop)

//////////////////////////////////////////////////////
//~ End of file
#endif // #ifndef FONT_PROVIDER_INC_INCLUDE_C