#ifndef OS_GFX_INCLUDE_C
#define OS_GFX_INCLUDE_C

global OS_W32_Gfx_State *os_w32_gfx_state = 0;
per_thread M_Arena *os_w32_event_list_arena = 0;
per_thread OS_EventList *os_w32_event_list = 0; 
OS_W32_GetDpiForWindowType *os_w32_GetDpiForWindowType = 0;
read_only global WCHAR *OS_W32_WindowClassString = L"WindowApplicationClass";

#define OS_W32_WindowStyleDefault (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME | WS_VISIBLE)
#define OS_W32_WindowStyleExtendedDefault (WS_EX_APPWINDOW)

/////////////////////////
//~ gsp: Helpers
inline_function void OS_W32_InitGfxReceiptSetValid(OS_InitGfxReceipt *osGfxReceipt)
{
    osGfxReceipt->a[0] = 1;
}

inline_function OS_W32_Window *
OS_W32_WindowFromHandle(OS_Handle handle)
{
    OS_W32_Window *result = (OS_W32_Window *)handle.a[0];
    return result;
}

inline_function OS_Handle
OS_W32_HandleFromWindow(OS_W32_Window *window)
{
    OS_Handle result = {0};
    result.a[0] = (u64)window;
    return result;
}

function OS_KeyModifierFlags
OS_W32_GetKeyModifiers(void)
{
    OS_KeyModifierFlags modifiers = 0;
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        modifiers |= OS_KeyModifierFlag_Ctrl;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        modifiers |= OS_KeyModifierFlag_Shift;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        modifiers |= OS_KeyModifierFlag_Alt;
    }
    if(GetKeyState(VK_CAPITAL))
    {
        modifiers |= OS_KeyModifierFlag_Caps;
    }
    return modifiers;
}


//////////////////////////////////////
//~ gsp: @per_os_backend General API functions

function OS_InitGfxReceipt
OS_InitGfx(OS_InitReceipt osInitReceipt)
{
    OS_InitGfxReceipt receipt = {0};
    
    if (OS_InitWasSuccessful(osInitReceipt) && tctx.isMainThread && (0 == os_w32_gfx_state))
    {
        M_Arena *arena = M_ArenaAlloc(GB(1));
        if (arena)
        {
            os_w32_gfx_state = M_PushStruct(arena, OS_W32_Gfx_State);
            os_w32_gfx_state->arena = arena;
            
            InitializeSRWLock(&os_w32_gfx_state->windowsSRWLock);
            
            // gsp: Register window class
            {
                WNDCLASSEXW wcex = {0};
                wcex.cbSize = sizeof(wcex);
                wcex.lpfnWndProc = OS_W32_WindowCallback;
                wcex.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
                wcex.hInstance = os_w32_hInstance;
                wcex.lpszClassName = OS_W32_WindowClassString;
                wcex.hCursor = LoadCursor(0, IDC_ARROW);
                if (!RegisterClassExW(&wcex))
                {
                    // @logging
                    LogPrint(LogLevel_Error, "OS W32 Gfx: Failed to register window class.");
                }
            }
            
            {
                OS_W32_SetProcessDpiAwarenessContextFunctionType *setDPIAwarenessFunction = 0;
                HMODULE user32 = LoadLibraryA("user32.dll");
                if(user32)
                {
                    setDPIAwarenessFunction =
                    (OS_W32_SetProcessDpiAwarenessContextFunctionType *)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
                    os_w32_GetDpiForWindowType = (OS_W32_GetDpiForWindowType*)GetProcAddress(user32, "GetDpiForWindow");
                    FreeLibrary(user32);
                }
                if(setDPIAwarenessFunction)
                {
                    setDPIAwarenessFunction(OS_W32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                }
            }
            
            //- refresh rate 
            {
                os_w32_gfx_state->refreshRate = 60.f;
                DEVMODEA deviceMode = {0};
                if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &deviceMode))
                {
                    os_w32_gfx_state->refreshRate = (f32)deviceMode.dmDisplayFrequency;
                }
            }
            
            //- setup raw input
            {
                RAWINPUTDEVICE Rid;
                Rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
                Rid.usUsage = HID_USAGE_GENERIC_MOUSE;
                // NOTE(gsp): when RIDEV_NOLEGACY is on we get no legacy mouse messages, we still want those tho
                // for window resizing / closing / etc.
                //Rid.dwFlags = RIDEV_NOLEGACY; // adds mouse and also ignores legacy mouse messages
                Rid.hwndTarget = 0;
                
                if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) == FALSE)
                {
                    // TODO: handle this with somemthing else besides logging
                    LogError(0, "OS W32 Layer: failed to register raw input devices.");
                    //registration failed. Call GetLastError for the cause of the error
                }
            }
            
            OS_W32_InitGfxReceiptSetValid(&receipt);
        }
        else
        {
            LogPrint(LogLevel_Error, "OS Win32: Failed to allocate memory.");
        }
    }
    
    return receipt;
}

inline_function b32
OS_InitGfxWasSuccessful(OS_InitGfxReceipt osGfxReceipt)
{
    return (osGfxReceipt.a[0] == 1);
}

//////////////////////////////////////
//~ gsp: @per_os_backend Windows

function OS_Handle
OS_WindowCreate(Vec2S64 clientSize, String8 title, OS_WindowFlags flags)
{
    OS_Handle result = OS_HandleZero();
    OS_W32_Window *window = 0;
    AcquireSRWLockExclusive(&os_w32_gfx_state->windowsSRWLock);
    {
        window = os_w32_gfx_state->freeWindow;
        if (0 != window)
        {
            StackPop(os_w32_gfx_state->freeWindow);
        }
        else
        {
            window = M_PushStructNoZero(os_w32_gfx_state->arena, OS_W32_Window);
        }
        MemoryZeroStruct(window);
        DLLPushBack(os_w32_gfx_state->firstWindow, os_w32_gfx_state->lastWindow, window);
        
        {
            DWORD extendedWindowStyle = OS_W32_WindowStyleExtendedDefault;
            DWORD windowStyle = OS_W32_WindowStyleDefault;
            
            if (OS_WindowFlags_AcceptsFile & flags)
                extendedWindowStyle |= WS_EX_ACCEPTFILES;
            
            if (OS_WindowFlags_NoBorderResizing & flags)
                windowStyle &= ~WS_THICKFRAME;
            
            {
                // gsp: Obtain border size
                RECT borderRect = {0, 0, 0, 0};
                AdjustWindowRectEx(&borderRect, windowStyle, 0, extendedWindowStyle);
                u32 borderWidth  = borderRect.right - borderRect.left;
                u32 borderHeight = borderRect.bottom - borderRect.top;
                // gsp: Grow window size by size of os border
                clientSize.x += borderWidth;
                clientSize.y += borderHeight;
            }
            
            HWND hwnd = 0;
            HDC  hdc  = 0;
            {
                M_ArenaTemp scratch = GetScratch(0, 0);
                String16 title16 = String16From8(scratch.arena, title);
                hwnd = CreateWindowExW(extendedWindowStyle, OS_W32_WindowClassString, (WCHAR*)title16.str, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, (i32)clientSize.x, (i32)clientSize.y, 0, 0, os_w32_hInstance, 0);
                hdc = GetDC(hwnd);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
                EndScratch(scratch);
            }
            
            {
                window->hwnd = hwnd;
                window->hdc = hdc;
            }
            
            result = OS_W32_HandleFromWindow(window);
        }
    }
    ReleaseSRWLockExclusive(&os_w32_gfx_state->windowsSRWLock);
    return result;
}


function void
OS_WindowClose(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    AcquireSRWLockExclusive(&os_w32_gfx_state->windowsSRWLock);
    {
        DLLRemove(os_w32_gfx_state->firstWindow, os_w32_gfx_state->lastWindow, window);
        StackPush(os_w32_gfx_state->freeWindow, window);
        if (window->hdc)
        {
            ReleaseDC(window->hwnd, window->hdc);
        }
        if (window->hwnd)
        {
            DestroyWindow(window->hwnd);
        }
    }
    ReleaseSRWLockExclusive(&os_w32_gfx_state->windowsSRWLock);
}

function void
OS_WindowShow(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);
}

function void
OS_WindowSetTitle(OS_Handle handle, String8 title)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    M_ArenaTemp scratch = GetScratch(0, 0);
    String16 title16 = String16From8(scratch.arena, title);
    SetWindowTextW(window->hwnd, (WCHAR *)title16.str);
    EndScratch(scratch);
}

//- gsp: maximize, minimize, focusing

function b32
OS_WindowIsMaximized(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    b32 result = !!(IsZoomed(window->hwnd));
    return result;
}

function b32
OS_WindowIsMinimized(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    b32 result = !!(IsIconic(window->hwnd));
    return result;
}

function void
OS_WindowMaximize(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    ShowWindow(window->hwnd, SW_MAXIMIZE);
}

function void
OS_WindowMinimize(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    ShowWindow(window->hwnd, SW_MINIMIZE);
}

function void
OS_WindowRestore(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    ShowWindow(window->hwnd, SW_RESTORE);
}

function b32
OS_WindowIsFocused(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    b32 result = GetForegroundWindow() == window->hwnd;
    return result;
}

//- gsp: fullscreen 

function b32
OS_WindowIsFullscreen(OS_Handle handle)
{
    b32 result = 0;
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    DWORD dwStyle = GetWindowLong(window->hwnd, GWL_STYLE);
    if (!(OS_W32_WindowStyleDefault & dwStyle))
    {
        result = 1;
    }
    return result;
}

function void
OS_WindowToggleFullscreen(OS_Handle handle)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    HWND hwnd = window->hwnd;
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    
    if (!window->wpPrevInitialised)
    {
        GetWindowPlacement(hwnd, &window->wpPrev);
        window->wpPrevInitialised = 1;
    }
    
    if (OS_W32_WindowStyleDefault & dwStyle)
    {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &window->wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | OS_W32_WindowStyleDefault);
        SetWindowPlacement(hwnd, &window->wpPrev);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

/////////////////////////
//~ WindowCallback
function LRESULT
OS_W32_WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    OS_Event *event = 0;
    OS_W32_Window *window = (OS_W32_Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    OS_Handle windowHandle = OS_W32_HandleFromWindow(window);
    M_ArenaTemp scratch = GetScratch(&os_w32_event_list_arena, 1);
    OS_EventList fallbackEventList = {0};
    if(0 == os_w32_event_list_arena)
    {
        os_w32_event_list_arena = scratch.arena;
        os_w32_event_list = &fallbackEventList;
    }
    b32 isReleased = 0;
    Axis2 scrollAxis = Axis2_Y;
    switch (uMsg)
    {
        default:
        {
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }break;
        
        case WM_CLOSE: // triggered when close button or ALT+F4 is pressed
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_WindowClose;
            event->window = windowHandle;
        }break;
        
        //- key up/down
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        {
            DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }fallthrough;
        case WM_KEYUP:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_Release;
            event->key = OS_W32_KeyFromWParam(wParam);
            event->keyWasDown = ((lParam & (1 << 30)) != 0);
        }break;
        case WM_KEYDOWN:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_Press;
            event->key = OS_W32_KeyFromWParam(wParam);
            event->keyWasDown = ((lParam & (1 << 30)) != 0);
        }break;
        
        //- mousewheel
        case WM_MOUSEHWHEEL: scrollAxis = Axis2_X; fallthrough;
        case WM_MOUSEWHEEL:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_MouseWheelScroll;
            event->mouseWheelDelta.elements[scrollAxis] = GET_WHEEL_DELTA_WPARAM(wParam) > 0.f ? 1.f : -1.f;
        }break;
        //- mousebuttons
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:  
        {
            isReleased = 1; fallthrough;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = isReleased ? OS_EventKind_Release : OS_EventKind_Press;
            OS_Key key = OS_Key_MouseLeft;
            switch(uMsg)
            {
                case WM_MBUTTONUP: case WM_MBUTTONDOWN: key = OS_Key_MouseMiddle; break;
                case WM_RBUTTONUP: case WM_RBUTTONDOWN: key = OS_Key_MouseRight; break;
            }
            event->key = key;
        }break;
        
        //- mouse delta
#if 0
        case WM_MOUSEMOVE:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_MouseMove;
            HMM_Vec2 newMousePos = HMM_V2((f32)(GET_X_LPARAM(lParam)), (f32)(GET_Y_LPARAM(lParam)));
            event->mouseDelta = Sub2F32(newMousePos, window->prevMouse);
            window->prevMouse = newMousePos;
        }break;
#endif
        case WM_INPUT:
        {
            UINT dwSize;
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = M_PushArray(scratch.arena, u8, dwSize);
            if (lpb == NULL) 
            {
                break;
            } 
            
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
            {
                Assert(0);
                break;
            }
            
            RAWINPUT* raw = (RAWINPUT*)lpb;
            if (raw->header.dwType == RIM_TYPEMOUSE) 
            {
                //LogInfo(0, "mouse: %d %d", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
                event = M_PushStruct(os_w32_event_list_arena, OS_Event);
                event->kind = OS_EventKind_MouseMove;
                event->mouseDelta = HMM_V2((f32)raw->data.mouse.lLastX, (f32)raw->data.mouse.lLastY);
            }
            
        }break;
        
        //- focus 
        case WM_KILLFOCUS:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_WindowLoseFocus;
        }break;
        case WM_SETFOCUS:
        {
            event = M_PushStruct(os_w32_event_list_arena, OS_Event);
            event->kind = OS_EventKind_WindowGainFocus;
        }break;
        
        //- dpi change 
        case WM_DPICHANGED:
        {
            //f32 newDpi = (f32)wParam;
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }break;
        
        //- drop files 
        case WM_DROPFILES:
        {
            if (window->flags & OS_WindowFlags_AcceptsFile)
            {
                event = M_PushStruct(os_w32_event_list_arena, OS_Event);
                event->kind = OS_EventKind_DropFiles;
                HANDLE hDrop = (HANDLE)wParam;
                u32 numberOfFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, 0, 0);
                POINT mousePoint;
                if (numberOfFiles && DragQueryPoint(hDrop, &mousePoint))
                {
                    event->mousePosFileDrop = Vec2S32FromVec(mousePoint);
                    for (u32 fileIdx = 0; fileIdx < numberOfFiles; ++fileIdx)
                    {
                        // NOTE(gsp): does not include null char
                        u32 filenameSize = DragQueryFileW(hDrop, fileIdx, 0, 0);
                        if (filenameSize)
                        {
                            String16 filename16;
                            filename16.size = filenameSize;
                            filename16.str  = M_PushArray(scratch.arena, u16, filenameSize + 1);
                            u32 charactersCopied = DragQueryFileW(hDrop, fileIdx, (WCHAR*)filename16.str, filenameSize + 1);
                            if (charactersCopied == filenameSize)
                            {
                                String8Node *node = M_PushStruct(os_w32_event_list_arena, String8Node);
                                node->string = String8From16(os_w32_event_list_arena, filename16);
                                Str8ListPushNode(&event->filePaths, node);
                            }
                        }
                    }
                }
            }
            else
            {
                result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
        }break;
    }
    
    if (event)
    {
        event->modifiers = OS_W32_GetKeyModifiers();
        event->window = windowHandle;
        DLLPushBack(os_w32_event_list->first, os_w32_event_list->last, event);
        os_w32_event_list->count += 1;
    }
    
    if (os_w32_event_list_arena == scratch.arena)
    {
        os_w32_event_list_arena = 0;
        os_w32_event_list = 0;
    }
    
    EndScratch(scratch);
    return result;
}

function OS_Key
OS_W32_KeyFromWParam(WPARAM wParam)
{
    OS_Key result = OS_Key_Null;
    switch (wParam)
    {
        case VK_LBUTTON:      result = OS_Key_MouseLeft;       break;
        case VK_RBUTTON:      result = OS_Key_MouseRight;      break;
        case VK_MBUTTON:      result = OS_Key_MouseMiddle;     break;
        case VK_XBUTTON1:     result = OS_Key_MouseX1;         break;
        case VK_XBUTTON2:     result = OS_Key_MouseX2;         break;
        case VK_BACK:         result = OS_Key_Backspace;       break;
        case VK_TAB:          result = OS_Key_Tab;             break;
        case VK_RETURN:       result = OS_Key_Enter;           break;
        case VK_SHIFT:        result = OS_Key_Shift;           break;
        case VK_CONTROL:      result = OS_Key_Ctrl;            break;
        case VK_MENU:         result = OS_Key_Alt;             break;
        case VK_ESCAPE:       result = OS_Key_Esc;             break;
        case VK_SPACE:        result = OS_Key_Space;           break;
        case VK_PRIOR:        result = OS_Key_PageUp;          break;
        case VK_NEXT:         result = OS_Key_PageDown;        break;
        case VK_END:          result = OS_Key_End;             break;
        case VK_HOME:         result = OS_Key_Home;            break;
        case VK_LEFT:         result = OS_Key_Left;            break;
        case VK_RIGHT:        result = OS_Key_Right;           break;
        case VK_UP:           result = OS_Key_Up;              break;
        case VK_DOWN:         result = OS_Key_Down;            break;
        case VK_DELETE:       result = OS_Key_Delete;          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':           result = (OS_Key)wParam;      break;
        case VK_F1:         result = OS_Key_F1;           break;
        case VK_F2:         result = OS_Key_F2;           break;
        case VK_F3:         result = OS_Key_F3;           break;
        case VK_F4:         result = OS_Key_F4;           break;
        case VK_F5:         result = OS_Key_F5;           break;
        case VK_F6:         result = OS_Key_F6;           break;
        case VK_F7:         result = OS_Key_F7;           break;
        case VK_F8:         result = OS_Key_F8;           break;
        case VK_F9:         result = OS_Key_F9;           break;
        case VK_F10:        result = OS_Key_F10;          break;
        case VK_F11:        result = OS_Key_F11;          break;
        case VK_F12:        result = OS_Key_F12;          break;
        case VK_F13:        result = OS_Key_F13;          break;
        case VK_F14:        result = OS_Key_F14;          break;
        case VK_F15:        result = OS_Key_F15;          break;
        case VK_F16:        result = OS_Key_F16;          break;
        case VK_F17:        result = OS_Key_F17;          break;
        case VK_F18:        result = OS_Key_F18;          break;
        case VK_F19:        result = OS_Key_F19;          break;
        case VK_F20:        result = OS_Key_F20;          break;
        case VK_F21:        result = OS_Key_F21;          break;
        case VK_F22:        result = OS_Key_F22;          break;
        case VK_F23:        result = OS_Key_F23;          break;
        case VK_F24:        result = OS_Key_F24;          break;
        case VK_OEM_PLUS:   result = OS_Key_Equal;        break;
        case VK_OEM_MINUS:  result = OS_Key_Minus;        break;
        case VK_OEM_COMMA:  result = OS_Key_Comma;        break;
        case VK_OEM_PERIOD: result = OS_Key_Period;       break;
        case VK_OEM_1:      result = OS_Key_Semicolon;    break;
        case VK_OEM_2:      result = OS_Key_ForwardSlash; break;
        case VK_OEM_3:      result = OS_Key_Tilda;        break;
        case VK_OEM_4:      result = OS_Key_LeftBracket;  break;
        case VK_OEM_6:      result = OS_Key_RightBracket; break;
        case VK_OEM_7:      result = OS_Key_Quote;        break;
    }
    return result;
}

//////////////////////////////////////
//~ gsp: Accessors

function Rng2F32
OS_RectFromWindow(OS_Handle handle)
{
    Rng2F32 rect = {0};
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    if (window)
    {
        RECT w32rect;
        if (GetWindowRect(window->hwnd, &w32rect))
        {
            rect.x0 = (f32)w32rect.left;
            rect.y0 = (f32)w32rect.top;
            rect.x1 = (f32)w32rect.right;
            rect.y1 = (f32)w32rect.bottom;
        }
    }
    return rect;
}

function Rng2F32
OS_ClientRectFromWindow(OS_Handle handle)
{
    Rng2F32 clientRect = {0};
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    if (window)
    {
        RECT w32rect;
        if (GetClientRect(window->hwnd, &w32rect))
        {
            clientRect.x0 = (f32)w32rect.left;
            clientRect.y0 = (f32)w32rect.top;
            clientRect.x1 = (f32)w32rect.right;
            clientRect.y1 = (f32)w32rect.bottom;
        }
    }
    return clientRect;
}

function HMM_Vec2   
OS_ClientDimsFromWindow(OS_Handle handle)
{
    return Dim2F32(OS_ClientRectFromWindow(handle));
}

function f32
OS_DPIFromWindow(OS_Handle handle)
{
    f32 result = 96.f;
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    if(window != 0)
    {
        HWND hwnd = window->hwnd;
        HDC dc = window->hdc;
        f32 dpi;
        if(0 == os_w32_GetDpiForWindowType)
        {
            f32 x = (f32)GetDeviceCaps(dc, LOGPIXELSX);
            //f32 y = (f32)GetDeviceCaps(dc, LOGPIXELSY);
            dpi = x;
        }
        else
        {
            dpi = (f32)os_w32_GetDpiForWindowType(hwnd);
        }
        result = dpi;
    }
    return result;
}

function HMM_Vec2
OS_MouseFromWindow(OS_Handle handle)
{
    HMM_Vec2 mouse = {0};
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    POINT w32Point;
    if (GetCursorPos(&w32Point) && ScreenToClient(window->hwnd, &w32Point))
    {
        mouse = HMM_V2((f32)w32Point.x, (f32)w32Point.y);
    }
    return mouse;
}

function void
OS_MouseSetRelPosition(OS_Handle windowHandle, HMM_Vec2 pos)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(windowHandle);
    POINT clientPos;
    clientPos.x = (u32)pos.x;
    clientPos.y = (u32)pos.y;
    ClientToScreen(window->hwnd, &clientPos);
    SetCursorPos(clientPos.x, clientPos.y);
}

function void
OS_MouseClipToWindow(OS_Handle windowHandle, Rng2F32 rect)
{
    OS_W32_Window *window = OS_W32_WindowFromHandle(windowHandle);
    
    POINT p0;
    p0.x = (u32)rect.p0.x;
    p0.y = (u32)rect.p0.y;
    ClientToScreen(window->hwnd, &p0);
    
    POINT p1;
    p1.x = (u32)rect.p1.x;
    p1.y = (u32)rect.p1.y;
    ClientToScreen(window->hwnd, &p1);
    
    RECT w32rect;
    w32rect.left   = p0.x;
    w32rect.right  = p1.x;
    w32rect.top    = p0.y;
    w32rect.bottom = p1.y;
    ClipCursor(&w32rect);
    
    LogInfo(0, "Clipping between: %d,%d and %d,%d", p0.x, p0.y, p1.x, p1.y);
}

function void
OS_MouseUnclip(void)
{
    LogInfo(0, "Unclipped mouse.");
    ClipCursor(0);
}

//////////////////////////////////////
//~ gsp: @per_os_backend Events

function OS_EventList
OS_GetEvents(OS_Handle handle, M_Arena *arena)
{
    OS_EventList list = {0};
    OS_W32_Window *window = OS_W32_WindowFromHandle(handle);
    os_w32_event_list_arena = arena;
    os_w32_event_list = &list;
    for (MSG msg; PeekMessageW(&msg, window->hwnd, 0, 0, PM_REMOVE);)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    list.mousePos = OS_MouseFromWindow(handle);
    list.os_window = handle;
    os_w32_event_list_arena = 0;
    os_w32_event_list = 0;
    return list;
}

function void
OS_EatEvent(OS_EventList *events, OS_Event *event)
{
    DLLRemove(events->first, events->last, event);
    events->count -= 1;
    event->kind = OS_EventKind_Null;
}

//////////////////////////////////////
//~ gsp: @per_os_backend System info

function f32
OS_GetRefreshRate(void)
{
    return os_w32_gfx_state->refreshRate;
}

//////////////////////////////////////
//~ gsp: @per_os_backend Cursor
function void
OS_SetCursor(OS_CursorKind kind)
{
    LPCTSTR cursor = IDC_ARROW;
    switch (kind)
    {
        default: AssertMsg(0, "Invalid OS_CursorKind"); break;
        case OS_CursorKind_Arrow:      cursor = IDC_ARROW; break;
        case OS_CursorKind_TextSelect: cursor = IDC_IBEAM; break;
    }
    HCURSOR hCursor = LoadCursor(NULL, cursor);
    SetCursor(hCursor);
}

function void 
OS_ShowCursor(b32 show)
{
    i32 targetVal = show ? 0 : -1;
    while (ShowCursor(show) != targetVal) {}
    os_w32_gfx_state->isCursorHidden = !show;
}

function void 
OS_ToggleCursorVisibility(void)
{
    OS_ShowCursor(os_w32_gfx_state->isCursorHidden);
}

///////////////////
//~ Checks
#if !OS_FEATURE_GFX
# error "OS_FEATURE_GFX must be 1 in order to use os_gfx_win32.c"
#endif

///////////////////
//~ Bottom of file
#endif // #ifndef OS_GFX_INCLUDE_C