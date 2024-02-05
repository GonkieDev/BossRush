/* date = July 15th 2023 4:37 pm */

#ifndef OS_GFX_INCLUDE_H
#define OS_GFX_INCLUDE_H

///////////////////
//~ Types

typedef struct OS_InitGfxReceipt OS_InitGfxReceipt;
struct OS_InitGfxReceipt
{
    u64 a[1];
};

//- gsp: Window types 

typedef u32 OS_WindowFlags;
enum
{
    OS_WindowFlags_NoBorderResizing            = (1 << 0),
    OS_WindowFlags_AcceptsFile                 = (1 << 1),
    // TODO(gsp): OS_WindowFlags_NoTopBar
    OS_WindowFlags_NoTopBar                    = (1 << 3), // No minimise/maximise/close buttons or titlebar
};


//- gsp: Event/input types 

typedef u32 OS_Key;
enum
{
    OS_Key_Null,
    OS_Key_0 = '0',
    OS_Key_1,
    OS_Key_2,
    OS_Key_3,
    OS_Key_4,
    OS_Key_5,
    OS_Key_6,
    OS_Key_7,
    OS_Key_8,
    OS_Key_9,
    OS_Key_A = 'A',
    OS_Key_B,
    OS_Key_C,
    OS_Key_D,
    OS_Key_E,
    OS_Key_F,
    OS_Key_G,
    OS_Key_H,
    OS_Key_I,
    OS_Key_J,
    OS_Key_K,
    OS_Key_L,
    OS_Key_M,
    OS_Key_N,
    OS_Key_O,
    OS_Key_P,
    OS_Key_Q,
    OS_Key_R,
    OS_Key_S,
    OS_Key_T,
    OS_Key_U,
    OS_Key_V,
    OS_Key_W,
    OS_Key_X,
    OS_Key_Y,
    OS_Key_Z,
    OS_Key_Esc,
    OS_Key_F1,
    OS_Key_F2,
    OS_Key_F3,
    OS_Key_F4,
    OS_Key_F5,
    OS_Key_F6,
    OS_Key_F7,
    OS_Key_F8,
    OS_Key_F9,
    OS_Key_F10,
    OS_Key_F11,
    OS_Key_F12,
    OS_Key_F13,
    OS_Key_F14,
    OS_Key_F15,
    OS_Key_F16,
    OS_Key_F17,
    OS_Key_F18,
    OS_Key_F19,
    OS_Key_F20,
    OS_Key_F21,
    OS_Key_F22,
    OS_Key_F23,
    OS_Key_F24,
    OS_Key_Tilda,
    OS_Key_Minus,
    OS_Key_Equal,
    OS_Key_Backspace,
    OS_Key_Delete,
    OS_Key_Tab,
    OS_Key_Space,
    OS_Key_Enter,
    OS_Key_Ctrl,
    OS_Key_Shift,
    OS_Key_Alt,
    OS_Key_Up,
    OS_Key_Left,
    OS_Key_Down,
    OS_Key_Right,
    OS_Key_PageUp,
    OS_Key_PageDown,
    OS_Key_Home,
    OS_Key_End,
    OS_Key_ForwardSlash,
    OS_Key_Period,
    OS_Key_Comma,
    OS_Key_Quote,
    OS_Key_LeftBracket,
    OS_Key_RightBracket,
    OS_Key_MouseLeft,
    OS_Key_MouseMiddle,
    OS_Key_MouseRight,
    OS_Key_MouseX1,
    OS_Key_MouseX2,
    OS_Key_Semicolon,
    
    OS_Key_Count
};

typedef u32 OS_EventKind;
enum
{
    OS_EventKind_Null,
    OS_EventKind_Press,
    OS_EventKind_Release,
    OS_EventKind_MouseMove,
    OS_EventKind_MouseWheelScroll,
    OS_EventKind_WindowClose,
    OS_EventKind_WindowLoseFocus,
    OS_EventKind_WindowGainFocus,
    OS_EventKind_DropFiles,
    OS_EventKind_COUNT
};

typedef u32 OS_KeyModifierFlags;
enum
{
    OS_KeyModifierFlag_Alt        = (1 << 0),
    OS_KeyModifierFlag_LShift     = (1 << 2), // Future use
    OS_KeyModifierFlag_RShift     = (1 << 3), // Future use
    OS_KeyModifierFlag_Shift      =  OS_KeyModifierFlag_RShift | OS_KeyModifierFlag_LShift,
    OS_KeyModifierFlag_LCtrl      = (1 << 4), // Future use
    OS_KeyModifierFlag_RCtrl      = (1 << 5), // Future use
    OS_KeyModifierFlag_Ctrl       = OS_KeyModifierFlag_RCtrl | OS_KeyModifierFlag_LCtrl,
    OS_KeyModifierFlag_Caps       = (1 << 6),
};

typedef struct OS_Event OS_Event;
struct OS_Event
{
    OS_Event *next;
    OS_Event *prev;
    
    OS_EventKind kind;
    OS_KeyModifierFlags modifiers;
    // TODO(gsp): @os_gfx.h remove OS_Handle window from OS_Event
    OS_Handle window;  // the event belongs to this window
    
    union
    {
        struct
        {
            OS_Key key;
            b32 keyWasDown;
        };
        HMM_Vec2 mouseDelta;
        HMM_Vec2 mouseWheelDelta; // -1 or 1
        
        // for OS_EventKind_DropFile
        struct
        {
            String8List filePaths;
            Vec2S32 mousePosFileDrop;
        };
        
    };
    
};

typedef struct OS_EventList OS_EventList;
struct OS_EventList
{
    OS_Event *first;
    OS_Event *last;
    u64 count;
    
    HMM_Vec2 mousePos;
    OS_Handle os_window;
};


typedef u32 OS_CursorKind;
enum
{
    OS_CursorKind_Arrow        = (0),
    OS_CursorKind_TextSelect   = (1 << 0),
};

//////////////////////////////////////
//~ gsp: @per_os_backend General API functions

function OS_InitGfxReceipt OS_InitGfx(OS_InitReceipt osInitReceipt);
inline_function b32 OS_InitGfxWasSuccessful(OS_InitGfxReceipt osGfxReceipt);

//////////////////////////////////////
//~ gsp: @per_os_backend Windows

// NOTE(gsp): Functions which take a OS_Handle, assume that it is a valid handle and do not do any checking to ensure this!

//- gsp: open/close/title/icon 
function OS_Handle OS_WindowCreate(Vec2S64 size, String8 title, OS_WindowFlags flags);
function void      OS_WindowClose(OS_Handle handle);
function void      OS_WindowShow(OS_Handle handle);
function void      OS_WindowSetTitle(OS_Handle handle, String8 title);
// TODO(gsp): OS_WindowSetIcon
function void      OS_WindowSetIcon(OS_Handle handle, Vec2S32 size, String8 rgba_data);

//- gsp: maximize, minimize, focusing
function b32       OS_WindowIsMaximized(OS_Handle handle);
function b32       OS_WindowIsMinimized(OS_Handle handle);
function void      OS_WindowMinimize(OS_Handle handle);
function void      OS_WindowMaximize(OS_Handle handle);
function void      OS_WindowRestore(OS_Handle handle);
function b32       OS_WindowIsFocused(OS_Handle handle);

//- gsp: fullscreen 
function b32       OS_WindowIsFullscreen(OS_Handle handle);
function void      OS_WindowToggleFullscreen(OS_Handle handle);

//- gsp: handle styling
// TODO(gsp): OS_UpdateFlags
function OS_WindowFlags OS_UpdateFlags(OS_Handle *handle, OS_WindowFlags newFlags);

//- gsp: accessors
function Rng2F32   OS_RectFromWindow(OS_Handle handle);
function Rng2F32   OS_ClientRectFromWindow(OS_Handle handle);
function HMM_Vec2   OS_ClientDimsFromWindow(OS_Handle handle);
function f32       OS_DPIFromWindow(OS_Handle handle);
function HMM_Vec2   OS_MouseFromWindow(OS_Handle handle);

//- gsp: setters
function void OS_MouseSetRelPosition(OS_Handle windowHandle, HMM_Vec2 pos);
function void OS_MouseClipToWindow(OS_Handle windowHandle, Rng2F32 rect);
function void OS_MouseUnclip(void);

//////////////////////////////////////
//~ gsp: @per_os_backend Events
function OS_EventList OS_GetEvents(OS_Handle handle, M_Arena *arena);
function void OS_EatEvent(OS_EventList *events, OS_Event *event);

//////////////////////////////////////
//~ gsp: @per_os_backend system info

function f32 OS_GetRefreshRate(void);

//////////////////////////////////////
//~ gsp: @per_os_backend Cursor
function void OS_SetCursor(OS_CursorKind kind);
function void OS_ShowCursor(b32 show);
function void OS_ToggleCursorVisibility(void);

////////////////////////////////
//~ gsp: @os_per_backend Clipboard

//function void OS_SetClipboardText(String8 string);
//function String8 OS_GetClipboardText(M_Arena *arena);


///////////////////
//~ Bottom of file
#endif //OS_GFX_INCLUDE_H
