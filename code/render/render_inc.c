#ifndef RENDER_INC_INCLUDE_C
#define RENDER_INC_INCLUDE_C

#include "render_types.c"

#if !defined(R_BACKEND_VK)
#define R_BACKEND_VK 0
#endif

#if !defined(R_BACKEND_GL)
#define R_BACKEND_GL 0
#endif

#if !defined(R_BACKEND_D11)
#define R_BACKEND_D11 0
#endif

#if !defined(R_BACKEND_D12)
#define R_BACKEND_D12 0
#endif

#if R_BACKEND_VK
# error "[R] R_BACKEND_VK not supoported."
#elif R_BACKEND_GL
# pragma message("[R] Using OpenGL renderer backend.")
# include "gl/render_gl.c"
#elif R_BACKEND_D11
# error "[R] R_BACKEND_D11 not supported."
#elif R_BACKEND_D12
# error "[R] R_BACKEND_D12 not supported."
#else
# error "[R] Invalid R_BACKEND_XXX used."
#endif

#include "render_core.c"

///////////////////////////////
//~ End of file
#endif //RENDER_INC_INCLUDE_C
