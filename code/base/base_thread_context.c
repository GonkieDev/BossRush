#ifndef BASE_THREAD_CONTEXT_INCLUDE_C
#define BASE_THREAD_CONTEXT_INCLUDE_C

per_thread ThreadContext tctx;
StaticAssert(ArrayCount(tctx.arenas) >= 2, "Need at least 2 arenas in a thread context to allow for scratch arenas to work!");

function T_InitReceipt
T_InitThreadContext(void)
{
    tctx.arenas[0] = M_ArenaAllocDefault();
    tctx.arenas[1] = M_ArenaAllocDefault();
    
    T_InitReceipt receipt;
    receipt.a[0] = 1;
    return receipt;
}

function T_InitReceipt
T_InitMainThreadContext(void)
{
    T_InitReceipt receipt = T_InitThreadContext();
    tctx.isMainThread = 1;
    return receipt;
}

function b32
T_IsMainThread(void)
{
    return (b32)tctx.isMainThread;
}

//~ End of file
#endif // #ifndef BASE_THREAD_CONTEXT_INCLUDE_C