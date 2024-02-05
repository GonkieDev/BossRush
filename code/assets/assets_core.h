/* date = July 31st 2023 11:32 pm */

#ifndef ASSETS_CORE_INCLUDE_H
#define ASSETS_CORE_INCLUDE_H

//////////////////////////////////////
//~ gsp: Defines

#if !defined(PATH_EXE_TO_ASSETS)
#define PATH_EXE_TO_ASSETS "assets/"
#define PATH_EXE_TO_BUILTIN_ASSETS PATH_EXE_TO_ASSETS "builtin/"
#endif

//////////////////////////////////////
//~ gsp: Types

//- Images types
typedef struct A_Image A_Image;
struct A_Image
{
    void    *data;
    Vec2S32 size;
    u32     channels; // [0,4]
    u32     __pad;
};


//////////////////////////////////////
//~ gsp: Functions

//- Image functions 
// NOTE(gsp): pixels are always = channels * sizeof(byte), (1 byte per channel)
// NOTE(gsp): when using [1,4] for desired channels, it will force the output
// data to use that format. If 0 is provided then the format
// of data will be the one used by the image.
function A_Image A_ImageFromFile(M_Arena *arena, u32 desiredChannels, String8 path);



//////////////////////////////////////
//~ End of file
#endif // ASSETS_CORE_INCLUDE_H
