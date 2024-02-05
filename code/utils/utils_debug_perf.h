#ifndef UTILS_DEBUG_PERF_INCLUDE_H
#define UTILS_DEBUG_PERF_INCLUDE_H

enum
{
    U_DebugPerfCounter_Input,
    U_DebugPerfCounter_Update,
    U_DebugPerfCounter_Physics,
    U_DebugPerfCounter_Render,
    U_DebugPerfCounter_Render_UI,
    U_DebugPerfCounter_Render_3D,
    U_DebugPerfCounter_TOTAL,
    
    U_DebugPerfCounter_Count
};

read_only global String8 u_debug_perf_counter_string[U_DebugPerfCounter_Count] = {
    [U_DebugPerfCounter_Input]     = S8LitComp("Input    "),
    [U_DebugPerfCounter_Update]    = S8LitComp("Update   "),
    [U_DebugPerfCounter_Physics]   = S8LitComp("Physics  "),
    [U_DebugPerfCounter_Render]    = S8LitComp("Render   "),
    [U_DebugPerfCounter_Render_UI] = S8LitComp("Render_UI"),
    [U_DebugPerfCounter_Render_3D] = S8LitComp("Render_3D"),
    [U_DebugPerfCounter_TOTAL]     = S8LitComp("Total    "),
};

typedef struct U_DebugPerfCounter U_DebugPerfCounter;
struct U_DebugPerfCounter
{
    u64 cycleCount;
    u64 time; // in microseconds
};

global U_DebugPerfCounter u_debug_perf_counters_1[U_DebugPerfCounter_Count];
global U_DebugPerfCounter u_debug_perf_counters_2[U_DebugPerfCounter_Count];
global U_DebugPerfCounter *u_debug_perf_counters = u_debug_perf_counters_1;
global U_DebugPerfCounter *u_debug_perf_counters_old = u_debug_perf_counters_2;

#if BUILD_DEBUG
# define U_BEGIN_TIMED_BLOCK(ID) U_DebugPerfCounter u_startPerfCount_##ID = U_DebugPerfCounterInit()
# define U_END_TIMED_BLOCK(ID) u_debug_perf_counters[U_DebugPerfCounter_##ID] = U_EndTimeBlock(u_startPerfCount_##ID)
# define U_TIME_BLOCK_END_ALL() Swap(U_DebugPerfCounter *, u_debug_perf_counters, u_debug_perf_counters_old)
#else // #if BUILD_DEBUG
# define U_BEGIN_TIMED_BLOCK(ID)
# define U_END_TIMED_BLOCK(ID)
# define U_TIME_BLOCK_END_ALL()
#endif // #if BUILD_DEBUG

function U_DebugPerfCounter U_DebugPerfCounterInit(void);
function U_DebugPerfCounter U_EndTimeBlock(U_DebugPerfCounter startPerfCounter);

//////////////////////////////////////////////////////
//~ END OF FILE
#endif //UTILS_DEBUG_PERF_INCLUDE_H
