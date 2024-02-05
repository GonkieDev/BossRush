#ifndef OS_ENTRY_POINT_WIN32_INCLUDE_C
#define OS_ENTRY_POINT_WIN32_INCLUDE_C

#if !BUILD_EXTERNAL_USAGE

# if OS_FEATURE_GFX

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    os_w32_hInstance = hInstance;
    UnusedVariable(nCmdShow);
    UnusedVariable(hPrevInstance);
    
    u32 argc = 0;
    String8 *argv = 0;
    
#if OS_FEATURE_GET_CMD_LINE
    // NOTE: the reason why we ++ argc down below is because it's a u32 and in this way we wont run into overflow issues
    i32 w32argc = 0;
    LPWSTR *argvW = CommandLineToArgvW(pCmdLine, &w32argc);
    M_Arena *argsArena = 0;
    if (w32argc > 0)
    {
        // TODO: @cleanup don't use arena here, calculate how much to allocate instead
        // if args > MB(1) then we'll crash
        argsArena = M_ArenaAlloc(MB(1));
        argv = M_PushArray(argsArena, String8, w32argc);
        for (i32 i = 0; i < w32argc; ++i)
        {
            String16 str16 = Str16C(argvW[i]);
            argv[i] = String8From16(argsArena, str16);
            argc++;
        }
    }
#else
    UnreferencedParameter(pCmdLine);
#endif // #if OS_FEATURE_GET_CMD_LINE
    
    EntryPoint(argc, argv);
    
#if OS_FEATURE_GET_CMD_LINE
    if (argsArena)
        M_ArenaRelease(argsArena);
#endif // #if OS_FEATURE_GET_CMD_LINE
    
    return 0;
}

# endif // #if !OS_FEATURE_GFX

#endif // if BUILD_EXTERNAL_USAGE

///////////////////////////////////////////////////////////////////////////
//~ EOF
#endif // #ifndef OS_ENTRY_POINT_WIN32_INCLUDE_C