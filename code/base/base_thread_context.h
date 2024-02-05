/* date = July 11th 2023 8:48 pm */

#ifndef BASE_THREAD_CONTEXT_INCLUDE
#define BASE_THREAD_CONTEXT_INCLUDE

/////////////////
//~ Types

typedef struct ThreadContext ThreadContext;
struct ThreadContext
{
    M_Arena *arenas[2];
    b64 isMainThread;
};

typedef struct T_InitReceipt T_InitReceipt;
struct T_InitReceipt
{
    u64 a[1];
};


/////////////////
//~ gsp: General API functions

function T_InitReceipt T_InitThreadContext(void);
// NOTE(gsp): just calls T_InitThreadContext but sets tctx.isMainThread = 1
function T_InitReceipt T_InitMainThreadContext(void);

function b32 T_IsMainThread(void);


//////////////////
//~ End of file
#endif //BASE_THREAD_CONTEXT_INCLUDE
