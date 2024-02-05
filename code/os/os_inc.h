/* date = July 9th 2023 2:17 am */

#ifndef OS_INC_INCLUDE_H
#define OS_INC_INCLUDE_H

#if !defined(M_ArenaImpl_Reserve)
# define M_ArenaImpl_Reserve  OS_Reserve
#endif
#if !defined(M_ArenaImpl_Release)
# define M_ArenaImpl_Release  OS_Release
#endif
#if !defined(M_ArenaImpl_Commit)
# define M_ArenaImpl_Commit   OS_Commit
#endif
#if !defined(M_ArenaImpl_Decommit)
# define M_ArenaImpl_Decommit OS_Decommit
#endif

#if BUILD_DEBUG && !defined(L_DebugPrint_Impl)
# define L_DebugPrint_Impl OS_OutputDebugString
#endif

#include "base/base_inc.h"

#if !defined(OS_FEATURE_GFX)
# define OS_FEATURE_GFX 0
#endif

#if !defined(OS_FEATURE_GET_CMD_LINE)
# define OS_FEATURE_GET_CMD_LINE 0
#endif

#include "os_core.h"
#if OS_FEATURE_GFX
# include "os_gfx.h"
#endif // # if OS_FEATURE_GFX

#if OS_WINDOWS
# include "win32/os_core_win32.h"

# if OS_FEATURE_GFX
#  include "win32/os_gfx_win32.h"
# endif // # if OS_FEATURE_GFX
#endif

#endif //OS_INC_INCLUDE_H
