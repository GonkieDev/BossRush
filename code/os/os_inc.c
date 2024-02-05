#ifndef OS_INC_INCLUDE_C
#define OS_INC_INCLUDE_C

#if OS_WINDOWS
# pragma message("[OS] Compiling for windows")
# include "win32/os_core_win32.c"
# include "win32/os_entry_point_win32.c"

# if OS_FEATURE_GFX
#  include "win32/os_gfx_win32.c"
# endif // # if OS_FEATURE_GFX
#endif // #if OS_WINDOWS

#if OS_MAC
# error "OS Layer not implemented for MAC OS"
#endif

#if OS_LINUX
# error "OS Layer not implemented for LINUX OS"
#endif

#include "os_core.c"

#endif // #ifdef OS_INC_INCLUDE_C