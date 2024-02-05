#ifndef ASSETS_CORE_INCLUDE_C
#define ASSETS_CORE_INCLUDE_C

//////////////////////////////////////
//~ gsp: includes

#if COMPILER_MSVC
# pragma warning( disable : 4820 4242 4244)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT Assert
#if !BUILD_DEBUG
#define STBI_NO_FAILURE_STRINGS
#endif
#include "stb_image.h"

#if COMPILER_MSVC
# pragma warning( default : 4820 4242 4244)
#endif

//////////////////////////////////////
//~ gsp: Image functions

// TODO(gsp): @clenaup @easy @arena use stbi_io_callbacks and stbi_load_from_callbacks
function A_Image
A_ImageFromFile(M_Arena *arena, u32 desiredChannels, String8 path)
{
    Assert(desiredChannels <= 4);
    UnreferencedParameter(arena);
    A_Image result = {0};
    
    int x, y, channels_in_file;
    stbi_uc* data = stbi_load((const char*)path.str, &x, &y, &channels_in_file, desiredChannels);
    
    result.size.x = x;
    result.size.y = y;
    result.channels = desiredChannels == 0 ? channels_in_file : desiredChannels;
    result.data = (void*)data;
    
    if (!data)
        MemoryZeroStruct(&result);
    
    return result;
}


//////////////////////////////////////
//~ End of file
#endif // ASSETS_CORE_INCLUDE_C