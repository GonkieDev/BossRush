/* date = July 15th 2023 5:08 pm */

#ifndef OS_GFX_WIN32_INCLUDE_H
#define OS_GFX_WIN32_INCLUDE_H

#include <windowsx.h> // for GET_X_LPARAM (and Y)
#include <hidusage.h> // for rawinput constants

//////////////////////////////////////
//~ Types

typedef struct OS_W32_Window OS_W32_Window;
struct OS_W32_Window
{
    OS_W32_Window *prev;
    OS_W32_Window *next;
    
    OS_WindowFlags flags;
    
    f32  monitorRefreshRate;
    
    HWND hwnd;
    HDC  hdc;
    WINDOWPLACEMENT wpPrev;
    b32 wpPrevInitialised;
};

typedef struct OS_W32_Gfx_State OS_W32_Gfx_State;
struct OS_W32_Gfx_State
{
    M_Arena *arena;
    
    // gsp: general info
    f32 refreshRate;
    b32 isCursorHidden;
    
    // gsp: windows
    SRWLOCK windowsSRWLock;
    OS_W32_Window *firstWindow;
    OS_W32_Window *lastWindow;
    OS_W32_Window *freeWindow;
};

//////////////////////////////////////
//~ Functions

//- gsp: Helpers 
inline_function void OS_W32_InitGfxReceiptSetValid(OS_InitGfxReceipt *osGfxReceipt);

inline_function OS_W32_Window *OS_W32_WindowFromHandle(OS_Handle handle);
inline_function OS_Handle OS_W32_HandleFromWindow(OS_W32_Window *window);

function OS_KeyModifierFlags OS_W32_GetKeyModifiers(void);

//- gsp: Window callback 
function LRESULT OS_W32_WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
function OS_Key OS_W32_KeyFromWParam(WPARAM wParam);

//- gsp: dpi
#define OS_W32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((void *)-4)
typedef BOOL OS_W32_SetProcessDpiAwarenessContextFunctionType(void *value);

///////////////////
//~ Bottom of file
#endif //OS_GFX_WIN32_INCLUDE_H
