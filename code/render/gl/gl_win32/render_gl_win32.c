global HGLRC r_gl_w32_context = 0; 
global wgl_create_context_attribs_arb *wglCreateContextAttribsARB = 0;
global wgl_choose_pixel_format_arb *wglChoosePixelFormatARB = 0;
global wgl_swap_interval_ext *wglSwapIntervalEXT = wglSwapIntervalEXT_Stub;

////////////////////////////////////////
//~ @per_os Backend Hooks
function void *
R_GL_GetGLFuncAddress(char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    // https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions#Windows
    AssertMsg(p != 0, "OpenGL function failed to load.");
    return p;
}

function b32
R_GL_OS_Init(OS_Handle osWindowHandle)
{
    if (!R_GL_W32_InitGLDummyContext())
    {
        return 0;
    }
    
    OS_W32_Window *window = OS_W32_WindowFromHandle(osWindowHandle);
    
    int pixelFormatAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };
    
    int pixelFormat;
    UINT numOfFormats = 0;
    wglChoosePixelFormatARB(window->hdc, pixelFormatAttribs, /* pfAttribList */ 0, 1, &pixelFormat, &numOfFormats);
    if (!numOfFormats)
    {
        LogError(GL_LOG_FILE, "[R, GL, W32] Failed to setup OpenGL pixel format.");
        return 0;
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(window->hdc, pixelFormat, sizeof(pfd), &pfd);
    if (!SetPixelFormat(window->hdc, pixelFormat, &pfd))
    {
        LogError(GL_LOG_FILE, "[R, GL, W32] Failed to set the OpenGL pixel format.");
        return 0;
    }
    
    int glAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, R_GL_MAJOR_VER,
        WGL_CONTEXT_MINOR_VERSION_ARB, R_GL_MINOR_VER,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if BUILD_DEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };
    
    r_gl_w32_context = wglCreateContextAttribsARB(window->hdc, 0, glAttribs);
    if (!r_gl_w32_context)
    {
        LogError(GL_LOG_FILE, "[R, GL, W32] Failed create OpenGL context.");
        return 0;
    }
    
    if (!wglMakeCurrent(window->hdc, r_gl_w32_context))
    {
        LogError(GL_LOG_FILE, "[R, GL, W32] Failed activate OpenGL rendering context.");
        return 0;
    }
    
    // @vsync
    // TODO: @cleanup check if extension is available
    // TODO(gsp): @vsync turn this into function
    wglSwapIntervalEXT = (wgl_swap_interval_ext*)wglGetProcAddress("wglSwapIntervalEXT");
#if 0 // 0: disable, 1: enable
    if (wglSwapIntervalEXT(1))
    {
        LogInfo(GL_LOG_FILE, "[Win32 GL] Vsync enabled");
    }
    else
    {
        LogError(GL_LOG_FILE, "[Win32 GL] Failed to enable Vsync.");
    }
#else
    if (wglSwapIntervalEXT(0))
    {
        LogInfo(GL_LOG_FILE, "[Win32 GL] Vsync disable");
    }
    else
    {
        LogError(GL_LOG_FILE, "[Win32 GL] Failed to disable Vsync.");
    }
#endif
    
    return 1;
}

function void
R_GL_OS_FinishFrame(OS_Handle osHandle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(osHandle);
    SwapBuffers(window->hdc);
}

//~ @GL_W32_Functions
function b32
R_GL_W32_InitGLDummyContext(void)
{
    b32 result = 0;
    
    WNDCLASSA glWindowClass = {0};
    glWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    glWindowClass.lpfnWndProc = DefWindowProcA;
    glWindowClass.hInstance = os_w32_hInstance;
    glWindowClass.lpszClassName = "DummyWGLWindowClass";
    
    if (RegisterClassA(&glWindowClass))
    {
        HWND dummyWindow = CreateWindowExA(0, glWindowClass.lpszClassName,"Dummy WGL Window", 0,
                                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                           0, 0, os_w32_hInstance, 0);
        
        if (dummyWindow)
        {
            HDC dummyDC = GetDC(dummyWindow);
            
            PIXELFORMATDESCRIPTOR pfd = {0};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.cColorBits = 32;
            pfd.cAlphaBits = 8;
            pfd.iLayerType = PFD_MAIN_PLANE;
            pfd.cDepthBits = 8;
            pfd.cStencilBits = 8;
            
            int pixelFormat = ChoosePixelFormat(dummyDC, &pfd);
            if (pixelFormat)
            {
                if (SetPixelFormat(dummyDC, pixelFormat, &pfd))
                {
                    HGLRC dummyContext = wglCreateContext(dummyDC);
                    if (dummyContext)
                    {
                        if (wglMakeCurrent(dummyDC, dummyContext))
                        {
                            wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");
                            wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
                            
                            result = (0 != wglCreateContextAttribsARB) && (0 != wglChoosePixelFormatARB);
                            
                            wglMakeCurrent(dummyDC, 0);
                        }
                        else
                        {
                            LogError(GL_LOG_FILE, "[R, GL, W32] Failed to activate dummy OpenGL rendering context.");
                        }
                        
                        wglDeleteContext(dummyContext);
                    }
                    else
                    {
                        LogError(GL_LOG_FILE, "[R, GL, W32] Failed to create dummy OpenGL context.");
                    }
                }
                else
                {
                    LogError(GL_LOG_FILE, "[R, GL, W32] Failed to set pixel format.");
                }
            }
            else
            {
                LogError(GL_LOG_FILE, "[R, GL, W32] Failed to find a suitable pixel format.");
            }
            
            ReleaseDC(dummyWindow, dummyDC);
            DestroyWindow(dummyWindow);
        }
        else
        {
            LogError(GL_LOG_FILE, "[R, GL, W32] Failed to create a dummy opengl window.");
        }
    }
    else
    {
        LogError(GL_LOG_FILE, "[R, GL, W32] Failed to register a dummy opengl window.");
    }
    
    return result;
}


//////////////////////////////
//~ End of file