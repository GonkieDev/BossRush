function U_DebugPerfCounter
U_DebugPerfCounterInit(void)
{
    U_DebugPerfCounter perfCounter = {
        .cycleCount = OS_TimeCycles(),
        .time = OS_TimeMicroseconds(),
    };
    return perfCounter;
}

function U_DebugPerfCounter 
U_EndTimeBlock(U_DebugPerfCounter startPerfCounter)
{
    U_DebugPerfCounter endPerfCounter = {
        .cycleCount = OS_TimeCycles() - startPerfCounter.cycleCount,
        .time = OS_TimeMicroseconds() - startPerfCounter.time,
    };
    return endPerfCounter;
}