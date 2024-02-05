inline_function OS_Handle
OS_HandleZero(void)
{
    OS_Handle result = {0};
    return result;
}

inline_function b32
OS_CompareHandles(OS_Handle a, OS_Handle b)
{
    return (a.a[0] == b.a[0]);
}

inline_function b32
OS_HandleIsNil(OS_Handle handle)
{
    return OS_CompareHandles(handle, OS_HandleZero());
}

//~
#if !OS_FEATURE_GFX && !BUILD_EXTERNAL_USAGE

int
main(int c_argc, char **c_argv)
{
    String8 *argv = 0;
    // NOTE: the reason why we ++ argc down below is because it's a u32 and in this way we wont run into overflow issues
    u32 argc = 0;
    
#if OS_FEATURE_GET_CMD_LINE
    M_Arena *argsArena = 0;
    // NOTE: we start at one so that this matches wWinMain (CommandLineToArgvW does not return first arg)
    if (c_argc > 1)
    {
        argsArena = M_ArenaAlloc(MB(1));
        argv = M_PushArray(argsArena, String8, c_argc - 1);
        for (i32 i = 1; i < c_argc; ++i)
        {
            argc++;
            argv[i-1] = Str8C(c_argv[i]);
        }
    }
#endif
    
    EntryPoint(argc, argv);
    
#if OS_FEATURE_GET_CMD_LINE
    if (argsArena)
        M_ArenaRelease(argsArena);
#endif // #if OS_FEATURE_GET_CMD_LINE
    
    return 0;
}

#endif // #if !OS_FEATURE_GFX
//~