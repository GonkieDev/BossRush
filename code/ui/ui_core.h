/* date = August 18th 2023 5:19 pm */

#ifndef UI_INCLUDE_HORE_INCLUDE_H
#define UI_INCLUDE_HORE_INCLUDE_H
//~

//~ @UI_Todos
/*
- gaps - kinda bad rn
- gaps between text and borders look kinda off
- bug: when parent is smaller then children sum, parent gets smaller but children don't, either one or the other should happen
- text alignment - top/bottom/left/right
- floating windows
- hot/active animations
*/

//~
typedef u32 UI_SizeKind;
enum UI_SizeKind
{
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
};

typedef struct UI_Size UI_Size;
struct UI_Size
{
    UI_SizeKind kind;
    f32 value;
    f32 strictness; // [0.0, 1.0] 
};

typedef u32 UI_WidgetFlags;
enum
{
    UI_WidgetFlag_Clickable       = (1<<1),
    UI_WidgetFlag_ViewScroll      = (1<<2),
    UI_WidgetFlag_DrawText        = (1<<3),
    UI_WidgetFlag_DrawBorder      = (1<<4),
    UI_WidgetFlag_DrawBackground  = (1<<5),
    //UI_WidgetFlag_DrawDropShadow  = (1<<6),
    UI_WidgetFlag_Clip            = (1<<7),
    UI_WidgetFlag_HotAnimation    = (1<<8),
    UI_WidgetFlag_ActiveAnimation = (1<<9),
    UI_WidgetFlag_NoAutolayout    = (1<<10),
    UI_WidgetFlag_DrawOnTop       = (1<<11),
};

typedef u64 UI_Key;

typedef struct UI_Widget UI_Widget;
struct UI_Widget
{
    //- per-frame info provided by builders
    UI_WidgetFlags flags;
    Axis2 axis; // axis in which children grow
    String8 string;
    UI_Size semanticSizes[Axis2_COUNT];
    HMM_Vec4 color;
    HMM_Vec4 borderColor;
    HMM_Vec4 textColor;
    f32 manualPos[Axis2_COUNT];
    
    //- recomputed every frame
    
    // n-ary tree
    UI_Widget *first;  // first child
    UI_Widget *last;   // last child
    UI_Widget *next;   // next in current level in hiearchy
    UI_Widget *prev;   // prev in current level in hiearchy
    UI_Widget *parent;
    
    String8 displayString;
    FC_GlyphRun glyphRun;
    
    f32 computedRelPos[Axis2_COUNT]; // relative to parent
    f32 computedSize[Axis2_COUNT];
    Rng2F32 finalRect;
    
    // hash-links
    UI_Widget *hashNext;
    UI_Widget *hashPrev;
    
    UI_Key key;
    
    //- persistent across frames
    f32 hot_t;
    f32 active_t;
    u64 firstTouchedGen;
    u64 lastTouchedGen;
    
    u64 cursorPos;
    u64 markPos;
    
};

//////////////////////////////////////////////
//~ Widget node
typedef struct UI_WidgetStackNode UI_WidgetStackNode;
struct UI_WidgetStackNode
{
    UI_Widget *widget;
    UI_WidgetStackNode *next;
};

//////////////////////////////////////////////
//~ UI Widget state
typedef struct UI_WidgetState UI_WidgetState;
struct UI_WidgetState
{
    UI_Widget *root;
    UI_WidgetStackNode *currentParent;
};

//////////////////////////////////////////////
//~ UI Comm / Interaction response
typedef struct UI_Comm UI_Comm;
struct UI_Comm
{
    b8 clicked;
    b8 doubleClicked;
    b8 hovered;
    b8 hoveredIncludingChildren;
    
    b8 pressed;
    b8 dragged;
    b8 __pad[2];
};

//////////////////////////////////////////////
//~ Styling types
typedef struct UI_ColorNode UI_ColorNode;
struct UI_ColorNode
{
    HMM_Vec4 color;
    UI_ColorNode *next;
};

typedef struct UI_NextColor UI_NextColor;
struct UI_NextColor
{
    HMM_Vec4 color;
    b32 notUsed;
};


//////////////////////////////////////////////
//~ @DrawOnTop
typedef struct UI_DrawOnTopNode UI_DrawOnTopNode;
struct UI_DrawOnTopNode
{
    UI_Widget *widget;
    UI_DrawOnTopNode *next;
    UI_DrawOnTopNode *prev;
};


//////////////////////////////////////////////
//~ UI State
typedef struct UI_State UI_State;
struct UI_State
{
    M_Arena *arena;
    
    u64 frameIndex;
    
    OS_Handle os_window;
    OS_EventList *eventList;
    
    UI_WidgetState _widgetStates[2];
    UI_WidgetState *currentWidgetState;
    UI_WidgetState *previousWidgetState;
    
    M_Arena *currentFrameArena;
    M_Arena *previousFrameArena;
    
#define UI_STATE_WIDGETS_TABLE_COUNT 500
    UI_Widget *currentWidgetTable;
    UI_Widget *previousWidgetTable;
    
    UI_Key hotKey;
    UI_Key activeKey;
    UI_Key focusKey;
    
    //- Draw On Top
    UI_DrawOnTopNode *currFirstDrawOnTopNode;
    UI_DrawOnTopNode *currLastDrawOnTopNode;
    UI_DrawOnTopNode *currFreeDrawOnTopNode;
    
    UI_DrawOnTopNode *prevFirstDrawOnTopNode;
    UI_DrawOnTopNode *prevLastDrawOnTopNode;
    
    
    //- Styling stuff
    FP_Handle font;
    f32 fontSize;
    f32 textPadding[Axis2_COUNT];
    f32 gap;
    
    // Background color
    HMM_Vec4 bgColorDefault;
    UI_ColorNode *bgColorStack;
    UI_NextColor bgColorNext;
    // Text color
    HMM_Vec4 textColorDefault;
    UI_NextColor textColorNext;
    UI_ColorNode *textColorStack;
};

//////////////////////////////////////////////
//~ Basic type helpers
function UI_Key UI_KeyNull(void);
function b32    UI_IsKeyNull(UI_Key key);
function b32    UI_IsKeyValid(UI_Key key);
function UI_Key UI_KeyFromString(String8 string);
function b32    UI_KeyMatch(UI_Key a, UI_Key b);
function u64    UI_IndexFromKey(u64 key);

//////////////////////////////////////////////
//~ Widget building
function UI_Widget *UI_WidgetMakeFromKey(UI_WidgetFlags flags, String8 string, UI_Key key);
function UI_Widget *UI_WidgetMake(UI_WidgetFlags flags, String8 string);
function UI_Widget *UI_WidgetMakeF(UI_WidgetFlags flags, char *fmt, ...);

function UI_Size UI_SizeMake(UI_SizeKind kind, f32 strictness, f32 value);
function void UI_WidgetEquipSize(UI_Widget *widget, Axis2 axis, UI_Size size);
function void UI_WidgetEquipSizes(UI_Widget *widget, UI_Size size);
function void UI_WidgetEquipChildLayoutAxis(UI_Widget *widget, Axis2 axis);

function void UI_WidgetEquipManualPos(UI_Widget *widget, HMM_Vec2 offset);

#define UI_SizePixels(strictness, value) UI_SizeMake(UI_SizeKind_Pixels, strictness, value) 
#define UI_SizeText(strictness) UI_SizeMake(UI_SizeKind_TextContent, strictness,  0.f)
#define UI_SizePP(strictness, value) UI_SizeMake(UI_SizeKind_PercentOfParent, strictness, value)

//////////////////////////////////////////////
//~ Helpers
function f32 UI_ChildrenSumFromWidget(UI_Widget *widget, Axis2 axis);
function f32 UI_ChildrenMaxSizeFromWidget(UI_Widget *widget, Axis2 axis);

function b32 UI_IsParent(UI_Widget *parentQuery, UI_Widget *widget);
function UI_Widget *UI_GetWidgetFromKey(UI_Key key);
function UI_Widget *UI_GetHashedWidgetPreviousFrame(UI_Widget *widget);

//////////////////////////////////////////////
//~ Comm
function UI_Comm UI_CommFromWidget(UI_Widget *widget);

//- Comm helpers
function OS_Event *UI_GetKeyInputEvent(OS_EventKind kind, OS_Key key);
function OS_Event *UI_GetEvent(OS_EventKind kind);

//////////////////////////////////////////////
//~ @Stacks
#define UI_Parent(widget) DeferLoop(UI_PushParent(widget), UI_PopParent())
function UI_Widget *UI_PushParent(UI_Widget *widget);
function UI_Widget *UI_PopParent(void);

function void UI_WidgetAddToDrawOnTop(UI_Widget *widget);
function void UI_WidgetRemoveFromDrawOnTop(UI_Widget *widget);

//- @UI_Styling

//- Background color
inline_function HMM_Vec4 UI_GetBgColor(void);
#define UI_BgColor(color) DeferLoop(UI_PushBgColor(color), UI_PopBgColor)
inline_function void UI_PushBgColor(HMM_Vec4 color);
inline_function void UI_PopBgColor(void);
inline_function void UI_SetNextBgColor(HMM_Vec4 color);
//- Border Color
// TODO(gsp): 
inline_function HMM_Vec4 UI_GetBorderColor(void);
//- Text color
inline_function HMM_Vec4 UI_GetTextColor(void);
#define UI_TextColor(color) DeferLoop(UI_PushTextColor(color), UI_PopTextColor)
inline_function void UI_PushTextColor(HMM_Vec4 color);
inline_function void UI_PopTextColor(void);
inline_function void UI_SetNextTextColor(HMM_Vec4 color);
//- Text size
inline_function f32 UI_GetFontSize(void);

//- General
inline_function HMM_Vec4 UI_GetColor(HMM_Vec4 defaultColor, UI_ColorNode *stack, UI_NextColor *next);
inline_function void UI_SetNextColor(UI_NextColor *nextColor, HMM_Vec4 color);

//////////////////////////////////////////////
//~ Transitions
inline_function b32 UI_IsHot(UI_Widget *widget);
inline_function b32 UI_IsActive(UI_Widget *widget);
inline_function b32 UI_IsFocus(UI_Widget *widget);
inline_function b32 UI_IsFocusFromKey(UI_Key key);

inline_function void UI_SetHot(UI_Widget *widget);
inline_function void UI_SetNotHot(UI_Widget *widget);
inline_function void UI_SetActive(UI_Widget *widget);
inline_function void UI_SetNotActive(UI_Widget *widget);
inline_function void UI_SetFocus(UI_Widget *widget);

//////////////////////////////////////////////
//~ General API
function b32  UI_Init(void);
#define UI_FrameScope(os_window, eventList, arena, cmdList) DeferLoop(UI_FrameBegin(os_window, eventList), UI_FrameEnd(arena, cmdList))
function void UI_FrameBegin(OS_Handle os_window, OS_EventList *eventList);
function void UI_FrameEnd(M_Arena *arena, R_CmdList *cmdList);

//////////////////////////////////////////////
//~ @Autolayout passes
function void UI_CalculateConstantSizes(void);
function void UI_CalculateUpwardsDependentSizes(void);
function void UI_CalculateDownwardsDependentSizes(void);
function void UI_SolveViolations(void);
function void UI_Layout(void);
//- @UI_Render
function void UI_Render(M_Arena *arena, R_CmdList *cmdList);
function void UI_RenderWidget(M_Arena *arena, R_CmdList *cmdList, UI_Widget *widget);

//////////////////////////////////////////////
//~ State getters
inline_function UI_Widget    *UI_Root(void);
inline_function OS_Handle     UI_Window(void);
inline_function OS_EventList *UI_EventList(void);
inline_function M_Arena      *UI_FrameArena(void);
inline_function u64           UI_FrameIndex(void);
inline_function HMM_Vec2       UI_Mouse(void);
inline_function HMM_Vec2       UI_MouseDelta(void);

//////////////////////////////////////////////
//~ Widget iterators

//- Linear iteration
#define  UI_ForWidget_Linear(widgetName)                   \
for (UI_Widget *(widgetName) = UI_IterLinearGetFirst();   \
(widgetName) != 0;                                   \
(widgetName) = UI_IterLinearGetNext(widgetName))
function UI_Widget *UI_IterLinearGetFirst(void);
function UI_Widget *UI_IterLinearGetNext(UI_Widget *prev);

//- PreOrder iteration
#define  UI_ForWidget_PreOrder(widgetName, flagsToPrune)                       \
for (UI_Widget *(widgetName) = UI_IterPreOrderGetFirst();       \
(widgetName) != 0;                                         \
(widgetName) = UI_IterPreOrderGetNext(widgetName, flagsToPrune))
function UI_Widget *UI_IterPreOrderGetFirst(void);
function UI_Widget *UI_IterPreOrderGetNext(UI_Widget *prev, UI_WidgetFlags flagsToPrune);

//- PostOrder
#define  UI_ForWidget_PostOrder(widgetName, flagsToPrune)                       \
for (UI_Widget *(widgetName) = UI_IterPostOrderGetFirst();       \
(widgetName) != 0;                                         \
(widgetName) = UI_IterPostOrderGetNext(widgetName, flagsToPrune))
function UI_Widget *UI_IterPostOrderGetFirst(void);
function UI_Widget *UI_IterPostOrderGetNext(UI_Widget *prev, UI_WidgetFlags flagsToPrune);

//- Children
// NOTE(gsp): iterates through first gen only
#define UI_ForWidget_Children(widget, childName)    \
for (UI_Widget *(childName) = (widget)->first;     \
(childName) != 0;                             \
(childName) = (childName)->next)

//- Parent
#define UI_ForWidget_Parents(start, parentName)    \
for (UI_Widget *(parentName) = (start);   \
(parentName) != 0;                            \
(parentName) = (parentName)->parent)

//- Siblings
#define UI_ForWidget_Siblings(start, siblingName)  \
for (UI_Widget *(siblingName) = (start);    \
(siblingName) != 0;                           \
(siblingName) = (siblingName)->next)

//- Subtree iterators
#define UI_ForWidget_PreOrderSubtree(widgetName, subtree)        \
for (UI_Widget *(widgetName) = subtree;                         \
(widgetName) != 0;                                         \
(widgetName) = UI_IterSubtreePreOrderGetNext(subtree, widgetName))

function UI_Widget *UI_IterSubtreePreOrderGetNext(UI_Widget *subtreeRoot, UI_Widget *prev);

//////////////////////////////////////////////
//~ End of file
#endif //UI_INCLUDE_HORE_INCLUDE_H
