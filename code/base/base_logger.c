#ifndef BASE_LOGGER_INCLUDE_C
#define BASE_LOGGER_INCLUDE_C

#if BASE_LOGGER_USE_PRINTF
# include <stdio.h>
#endif

#if BUILD_DEBUG && !defined(L_DebugPrint_Impl)
# error "L_DebugPrint_Impl must be defined to use base_logger.c"
#endif

function String8
LogFmtVAList(M_Arena *arena, LogLevel level, b32 appendNewLineChar, char *fmt, va_list args)
{
    String8 formattedLog = PushStr8FmtVAList(arena, fmt, args);
    char *finalFmt = appendNewLineChar ? "%s%s\n" : "%s%s";
    String8 finalString = PushStr8Fmt(arena, finalFmt, log_level_prefix_strings[level].str, formattedLog.str);
    
    return finalString;
}

function void
LogPrint_(LogLevel level, b32 appendNewLineChar, char *fmt, ...)
{
#if BUILD_DEBUG || LOG_PRINT_RELEASE
    M_ArenaTemp scratch = GetScratch(0, 0);
    
    va_list args;
    va_start(args, fmt);
    String8 formattedString = LogFmtVAList(scratch.arena, level, appendNewLineChar, fmt, args);
    va_end(args);
    
#if BASE_LOGGER_USE_PRINTF
    printf("%.*s", S8VArg(formattedString));
#else
    L_DebugPrint_Impl(formattedString.str);
#endif
    
    EndScratch(scratch);
#else
    UnreferencedParameter(level);
    UnreferencedParameter(appendNewLineChar);
    UnreferencedParameter(fmt);
#endif // #if BUILD_DEBUG
}



/////////////////
//~ End of file
#endif // #ifndef BASE_LOGGER_INCLUDE_C