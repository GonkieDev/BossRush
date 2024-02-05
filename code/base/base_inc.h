/* date = July 8th 2023 0:55 pm */

#ifndef BASE_INC_INCLUDE_H
#define BASE_INC_INCLUDE_H

///////////////////////////////////
//~ gsp: All the base layer .h files

#include "base_context_detection.h"
#include "base_types.h"

#define HANDMADE_MATH_USE_RADIANS
#include "HandmadeMath.h"

#define BASE_DISABLE_OMITTED_BASE_TYPES 0
#include "base_math.h"

#include "base_memory.h"
#include "base_thread_context.h"
#include "base_string.h"

#if !defined(BASE_LOGGER_USE_PRINTF)
# define BASE_LOGGER_USE_PRINTF 0
#endif

#if BASE_LOGGER_USE_PRINTF
# if COMPILER_MSVC
#  pragma message("Build: using subsystem:console")
#  pragma comment(linker, "/SUBSYSTEM:CONSOLE")
# endif // # if COMPILER_MSVC
#endif // #if BASE_LOGGER_USE_PRINTF

#include "base_logger.h"

/////////////////
//~ End of file
#endif //BASE_INC_INCLUDE_H
