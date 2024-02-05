/* date = August 10th 2023 5:02 pm */

#ifndef FONT_PROVIDER_CORE_DWRITE_INCLUDE_H
#define FONT_PROVIDER_CORE_DWRITE_INCLUDE_H

//////////////////////////////////////////////////////
//~ [h] includes
#pragma warning(push)
#pragma warning(disable: 4668 4820 4042 5246 4365 5246 4365 4505 4514 4514)

#define BUILD_EXTERNAL_USAGE 1
#include "base/base_inc.h"

#include <windows.h>
#include <dwrite_1.h>
#pragma comment(lib, "dwrite.lib")

#pragma warning(pop)

#include "font_provider/font_provider_core.h"


//////////////////////////////////////////////////////
//~ gsp: Types
struct FP_DW_State
{
    M_Arena *arena;
    IDWriteFactory1 *dwFactory;
    IDWriteGdiInterop *dwGdiInterop;
    IDWriteRenderingParams *dwDefaultRenderingParams;
    IDWriteRenderingParams *dwRenderingParams;
};

struct FP_DW_Font
{
    IDWriteFontFace1 *fontFace;
};

//////////////////////////////////////////////////////
//~ gsp: Internal functions
#define FP_DW_Check(error,r)        if ((error) != S_OK){ error = S_OK; r; }
#define FP_DW_CheckPtr(error,ptr,r) if ((ptr) == 0 || (error) != S_OK){ error = S_OK; r; }

//- gsp: Helpers 
fp_function FP_Handle  FP_HandleFromDWFont(FP_DW_Font font);
fp_function FP_DW_Font FP_DWFontFromHandle(FP_Handle  handle);


fp_function FP_Metrics FP_MetricsFromDWFont(FP_DW_Font dwFont, f32 size);



//////////////////////////////////////////////////////
//~ End of file
#endif //FONT_PROVIDER_CORE_DWRITE_INCLUDE_H
