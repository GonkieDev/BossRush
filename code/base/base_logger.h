/* date = July 14th 2023 1:58 pm */

#ifndef BASE_LOGGER_INCLUDE_H
#define BASE_LOGGER_INCLUDE_H

#ifndef LOG_PRINT_RELEASE
# define LOG_PRINT_RELEASE 0
#endif

/////////////////////
//~ Types

typedef u32 LogLevel;
enum
{
    LogLevel_None,
    LogLevel_Fatal,
    LogLevel_Error,
    LogLevel_Warn,
    LogLevel_Debug,
    LogLevel_Info,
    LogLevel_Trace,
    
    LogLevel_COUNT
};

read_only global String8 log_level_prefix_strings[LogLevel_COUNT] = {
    S8LitComp("        "),
    S8LitComp("[Fatal] "),
    S8LitComp("[Error] "),
    S8LitComp("[Warn]  "),
    S8LitComp("[Debug] "),
    S8LitComp("[Info]  "),
    S8LitComp("[Trace] "),
};

#define V2Args(v) v.x, v.y
#define V3Args(v) v.x, v.y, v.z

/////////////////////
//~ Functions

function String8
LogFmtVAList(M_Arena *arena, LogLevel level, b32 appendNewLineChar, char *fmt, va_list args);

#define LogPrint(level, fmt, ...) LogPrint_(level, 1, fmt, ##__VA_ARGS__)
#define LogPrintNNL(level, fmt, ...) LogPrint_(level, 0, fmt, ##__VA_ARGS__)
// NOTE(gsp): does nothing in non debug builds
function void LogPrint_(LogLevel level, b32 appendNewLineChar, char *fmt, ...);

#define LogFatal(file, fmt, ...) LogPrint_(LogLevel_Fatal, 1, fmt, ##__VA_ARGS__)
#define LogError(file, fmt, ...) LogPrint_(LogLevel_Error, 1, fmt, ##__VA_ARGS__)
#define LogWarn(file, fmt, ...) LogPrint_(LogLevel_Warn, 1, fmt, ##__VA_ARGS__)
#define LogDebug(file, fmt, ...) LogPrint_(LogLevel_Debug, 1, fmt, ##__VA_ARGS__)
#define LogInfo(file, fmt, ...) LogPrint_(LogLevel_Info, 1, fmt, ##__VA_ARGS__)
#define LogTrace(file, fmt, ...) LogPrint_(LogLevel_Trace, 1, fmt, ##__VA_ARGS__)

/////////////////
//~ End of file
#endif //BASE_LOGGER_INCLUDE_H
