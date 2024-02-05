/* date = July 11th 2023 7:44 pm */

#ifndef LOGGER_INCLUDE_H
#define LOGGER_INCLUDE_H

// TODO(gsp): thread safe file logger

/////////////////
//~ Types

typedef struct L_Receipt L_Receipt;
struct L_Receipt
{
 u64 receipt[1];
};

typedef struct L_State L_State;
struct L_State
{
 OS_Handle mainLogFileHandle;
};


/////////////////
//~ Constants


/////////////////
//~ General API functions

function L_Receipt LogInit(Str8 logFilePath);
function void LogShutdown();

/////////////////
//~ Logging functions

function void LogOutput(M_Arena *arena, String8 fmt, ...);





/////////////////
//~ End of file
#endif //LOGGER_INCLUDE_H
