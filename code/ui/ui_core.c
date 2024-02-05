#ifndef UI_INCLUDE_CORE_INCLUDE_C
#define UI_INCLUDE_CORE_INCLUDE_C
//~

//////////////////////////////////////////////
//~ [h] includes
#include "meow_hash_inc.h"

//////////////////////////////////////////////
//~ gsp: Globals
global UI_State *ui_state;

//////////////////////////////////////////////
//~ Basic type helpers
function UI_Key
UI_KeyNull(void)
{
    return 0;
}

function b32    
UI_IsKeyNull(UI_Key key)
{
    return UI_KeyMatch(UI_KeyNull(), key);
}

function b32
UI_IsKeyValid(UI_Key key)
{
    return !UI_IsKeyNull(key);
}

function UI_Key
UI_KeyFromString(String8 string)
{
    meow_u128 hash = MeowHash(MeowDefaultSeed, string.size, string.str);
    u64 hash64 = MeowU64From(hash, 0) + 1;
    UI_Key key = (UI_Key)hash64;
    Assert(UI_KeyNull() != key);
    return key;
}

function b32    
UI_KeyMatch(UI_Key a, UI_Key b)
{
    return a == b;
}

function u64
UI_IndexFromKey(u64 key)
{
    u64 index = key % UI_STATE_WIDGETS_TABLE_COUNT;
    return index;
}

//////////////////////////////////////////////
//~ Helpers
function f32
UI_ChildrenSumFromWidget(UI_Widget *widget, Axis2 axis)
{
    f32 childrenSum = 0.f;
    UI_ForWidget_Children(widget, child)
    {
        if (child->flags & UI_WidgetFlag_NoAutolayout)
            continue;
        childrenSum += child->computedSize[axis] + ui_state->gap;
    }
    if (childrenSum != 0.f)
        childrenSum += ui_state->gap;
    return childrenSum;
}

function f32
UI_ChildrenMaxSizeFromWidget(UI_Widget *widget, Axis2 axis)
{
    f32 childrenMax = 0.f;
    UI_ForWidget_Children(widget, child)
    {
        if (child->flags & UI_WidgetFlag_NoAutolayout)
            continue;
        childrenMax = Max(childrenMax, child->computedSize[axis]);
    }
    return childrenMax;
}

function b32
UI_IsParent(UI_Widget *parentQuery, UI_Widget *widget)
{
    b32 result = 0;
    UI_ForWidget_PreOrderSubtree(child, parentQuery)
    {
        if (UI_KeyMatch(child->key, widget->key))
        {
            result = 1;
            break;
        }
    }
    return result;
}

function UI_Widget *
UI_GetWidgetFromKey(UI_Key key)
{
    UI_Widget *result = 0;
    if (UI_IsKeyValid(key) && ui_state->currentWidgetTable)
    {
        u64 startingIndex = UI_IndexFromKey(key);
        for (UI_Widget *hashedWidget = &ui_state->currentWidgetTable[startingIndex]; 
             hashedWidget && UI_IsKeyValid(hashedWidget->key);
             hashedWidget = hashedWidget->hashNext)
        {
            if (UI_KeyMatch(key, hashedWidget->key))
            {
                result = hashedWidget;
                break;
            }
        }
    }
    return result;
}

function UI_Widget *
UI_GetHashedWidgetPreviousFrame(UI_Widget *widget)
{
    UI_Widget *result = 0;
    if (ui_state->previousWidgetTable)
    {
        u64 widgetIndex =  UI_IndexFromKey(widget->key);
        for (UI_Widget *hashedWidget = &ui_state->previousWidgetTable[widgetIndex]; 
             hashedWidget && UI_IsKeyValid(hashedWidget->key);
             hashedWidget = hashedWidget->hashNext)
        {
            if (UI_KeyMatch(widget->key, hashedWidget->key))
            {
                result = hashedWidget;
                break;
            }
        }
    }
    return result;
}

//////////////////////////////////////////////
//~ Widget building
function UI_Widget *
UI_WidgetMakeFromKey(UI_WidgetFlags flags, String8 string, UI_Key key)
{
    u64 keyIndex = UI_IndexFromKey(key);
    UI_Widget *widget = &ui_state->currentWidgetTable[keyIndex];
    if (UI_IsKeyValid(widget->key))
    {
        UI_Widget *newWidget = M_PushStruct(UI_FrameArena(), UI_Widget);
        DLLInsert_NPZ(widget, widget->hashNext, widget, newWidget, hashNext, hashPrev, CheckNull, SetNull);
        widget = newWidget;
    }
    
    UI_WidgetStackNode *parentNode = ui_state->currentWidgetState->currentParent;
    
    // info
    widget->color = UI_GetBgColor();
    widget->textColor = UI_GetTextColor();
    widget->borderColor = UI_GetBorderColor();
    widget->string = PushStr8Copy(UI_FrameArena(), string);
    widget->displayString = widget->string;
    widget->flags  = flags;
    UI_WidgetEquipSizes(widget, UI_SizeMake(UI_SizeKind_ChildrenSum, 1.f, 0.f));
    widget->axis = Axis2_Y;
    widget->firstTouchedGen = widget->lastTouchedGen = ui_state->frameIndex;
    
    // n-ary tree
    if (parentNode)
    {
        UI_Widget *parent = parentNode->widget;
        widget->parent = parent;
        DLLPushBack(parent->first, parent->last, widget);
        if (parent->last != widget)
        {
            QueuePush(parent->last->next, parent->last->prev, widget);
        }
    }
    
    // hash-links
    widget->key = key;
    
    // persistent across frames
    if (ui_state->previousWidgetTable)
    {    
        // TODO(gsp): make function for this loop
        UI_Widget *lastFrameWidget = &ui_state->previousWidgetTable[keyIndex];
        for (; lastFrameWidget != 0; lastFrameWidget = lastFrameWidget->hashNext)
        {
            if (UI_KeyMatch(key, lastFrameWidget->key))
            {
                break;
            }
        }
        if (lastFrameWidget)
        {
            widget->hot_t    = lastFrameWidget->hot_t;
            widget->active_t = lastFrameWidget->active_t;
            widget->firstTouchedGen = lastFrameWidget->firstTouchedGen;
            widget->cursorPos = lastFrameWidget->cursorPos;
            widget->markPos = lastFrameWidget->markPos;
        }
    }
    
    if (widget->flags & UI_WidgetFlag_DrawOnTop)
    {
        UI_WidgetAddToDrawOnTop(widget);
    }
    
    return widget;
}

function UI_Widget *
UI_WidgetMake(UI_WidgetFlags flags, String8 string)
{
    u64 key = UI_KeyFromString(string);
    return UI_WidgetMakeFromKey(flags, string, key);
}

function UI_Widget *
UI_WidgetMakeF(UI_WidgetFlags flags, char *fmt, ...)
{
    UI_Widget *widget = 0;
    va_list args;
    va_start(args, fmt);
    String8 string = PushStr8FmtVAList(UI_FrameArena(), fmt, args);
    va_end(args);
    widget = UI_WidgetMake(flags, string);
    return widget;
}

function UI_Size
UI_SizeMake(UI_SizeKind kind, f32 strictness, f32 value)
{
    UI_Size innerBoxUISize = {0};
    innerBoxUISize.kind = kind;
    innerBoxUISize.strictness = strictness;
    innerBoxUISize.value = value;
    return innerBoxUISize;
}

function void
UI_WidgetEquipSize(UI_Widget *widget, Axis2 axis, UI_Size size)
{
    Assert(axis == Axis2_X || axis == Axis2_Y);
    widget->semanticSizes[axis] = size;
    if (size.kind == UI_SizeKind_TextContent && widget->glyphRun.advance == 0)
    {
        widget->glyphRun = FC_GlyphRunFromString(UI_FrameArena(), ui_state->font, ui_state->fontSize, widget->displayString, R_TextureTransferMinGranularity());
    }
}

function void
UI_WidgetEquipSizes(UI_Widget *widget, UI_Size size)
{
    UI_WidgetEquipSize(widget, Axis2_X, size);
    UI_WidgetEquipSize(widget, Axis2_Y, size);
}

function void
UI_WidgetEquipChildLayoutAxis(UI_Widget *widget, Axis2 axis)
{
    widget->axis = axis;
}

function void
UI_WidgetEquipManualPos(UI_Widget *widget, HMM_Vec2 offset)
{
    Assert(widget->flags & UI_WidgetFlag_NoAutolayout);
    widget->manualPos[Axis2_X] = offset.x;
    widget->manualPos[Axis2_Y] = offset.y;
}

//////////////////////////////////////////////
//~ Widget stack managing functions
function UI_Widget *
UI_PushParent(UI_Widget *widget)
{
    UI_Widget *result = 0;
    UI_WidgetStackNode *parentNode = ui_state->currentWidgetState->currentParent;
    if (parentNode)
    {
        result = ui_state->currentWidgetState->currentParent->widget;
    }
    UI_WidgetStackNode *node = M_PushStruct(UI_FrameArena(), UI_WidgetStackNode);
    node->widget = widget;
    StackPush(ui_state->currentWidgetState->currentParent, node);
    return result;
}

function UI_Widget *
UI_PopParent(void)
{
    UI_Widget *result = ui_state->currentWidgetState->currentParent->widget;
    StackPop(ui_state->currentWidgetState->currentParent);
    return result;
}

function void
UI_WidgetAddToDrawOnTop(UI_Widget *widget)
{
    UI_DrawOnTopNode *drawOnTopNode = ui_state->currFreeDrawOnTopNode;
    if (drawOnTopNode)
    {
        StackPop(ui_state->currFreeDrawOnTopNode);
    }
    else
    {
        drawOnTopNode = M_PushStructNoZero(UI_FrameArena(), UI_DrawOnTopNode);
    }
    MemoryZeroStruct(drawOnTopNode);
    
    drawOnTopNode->widget = widget;
    DLLPushFront(ui_state->currFirstDrawOnTopNode, ui_state->currLastDrawOnTopNode, drawOnTopNode);
}

function void
UI_WidgetRemoveFromDrawOnTop(UI_Widget *widget)
{
    UI_DrawOnTopNode *nodeToRemove = 0;
    for (UI_DrawOnTopNode *node = ui_state->currFirstDrawOnTopNode; node != 0; node = node->next)
    {
        if (UI_KeyMatch(widget->key, node->widget->key))
        {
            nodeToRemove = node;
            break;
        }
    }
    Assert(nodeToRemove);
    DLLRemove(ui_state->currFirstDrawOnTopNode, ui_state->currLastDrawOnTopNode, nodeToRemove);
    StackPush(ui_state->currFreeDrawOnTopNode, nodeToRemove);
}

//////////////////////////////////////////////
//~ Comm
function UI_Comm
UI_CommFromWidget(UI_Widget *widget)
{
    UI_Comm comm = {0};
    
    if ((widget->flags & UI_WidgetFlag_Clickable) && (ui_state->previousWidgetTable))
    {
        UI_Widget *prevFrameWidget = UI_GetHashedWidgetPreviousFrame(widget);
        
        if (prevFrameWidget)
        {
            HMM_Vec2 mousePos = OS_MouseFromWindow(ui_state->eventList->os_window);
            Rng2F32 clipRect = OS_ClientRectFromWindow(UI_Window());
            clipRect = Intersection2F32(prevFrameWidget->finalRect, clipRect);
            b32 inside = Contains2F32(clipRect, mousePos);
            
            // Check if there any 'draw on top' boxes blocking this
            if (inside)
            {
                for (UI_DrawOnTopNode *topNode = ui_state->prevFirstDrawOnTopNode; topNode != 0; topNode = topNode->next)
                {
                    if (UI_KeyMatch(topNode->widget->key, widget->key) || !(topNode->widget->flags & UI_WidgetFlag_Clickable))
                        continue;
                    
                    UI_Widget *drawOnTopWidget = topNode->widget;
                    UI_Widget *drawOnTopWidgetLastFrame = UI_GetHashedWidgetPreviousFrame(drawOnTopWidget);
                    
                    if (!UI_IsParent(drawOnTopWidget, widget) && 
                        drawOnTopWidgetLastFrame && 
                        Contains2F32(drawOnTopWidgetLastFrame->finalRect, mousePos))
                    {
                        // TODO(gsp): check which one is acc on top, when the current node being evaluated is also a floating window or a child of one
                        inside = 0;
                        break;
                    }
                }
            }
            
            // Check if any children obstruct mouse pos
            comm.hoveredIncludingChildren = (b8)inside;
            if (inside && widget->flags)
            {
                UI_ForWidget_Children(prevFrameWidget, lastFrameChild)
                {
                    if ((lastFrameChild->flags & UI_WidgetFlag_Clickable) && Contains2F32(lastFrameChild->finalRect, mousePos))
                    {
                        comm.hoveredIncludingChildren = 1;
                        inside = 0;
                        break;
                    }
                }
            }
            
            OS_Event *mouseDownEvent = UI_GetKeyInputEvent(OS_EventKind_Press, OS_Key_MouseLeft);
            OS_Event *mouseUpEvent   = UI_GetKeyInputEvent(OS_EventKind_Release, OS_Key_MouseLeft);
            OS_Event *mouseMoveEvent = UI_GetEvent(OS_EventKind_MouseMove);
            
            if (inside)
            {
                UI_SetHot(widget);
                comm.hovered = 1;
            }
            else
            {
                //UI_SetNotHot(widget);
            }
            
            
            if (UI_IsActive(prevFrameWidget))
            {
                if (mouseUpEvent)
                {
                    if (UI_IsHot(prevFrameWidget))
                    {
                        comm.clicked = 1;
                        OS_EatEvent(ui_state->eventList, mouseUpEvent);
                    }
                    UI_SetNotActive(widget);
                }
                else if (mouseMoveEvent)
                {
                    HMM_Vec2 mouseDelta = UI_MouseDelta();
                    comm.dragged = mouseDelta.x != 0 || mouseDelta.y != 0;
                    OS_EatEvent(UI_EventList(), mouseMoveEvent);
                }
            }
            else if (UI_IsHot(prevFrameWidget))
            {
                if (mouseDownEvent)
                {
                    comm.pressed = 1;
                    OS_EatEvent(UI_EventList(), mouseDownEvent);
                    UI_SetActive(widget);
                }
            }
            
        }
    }
    
    return comm;
}

function OS_Event *
UI_GetKeyInputEvent(OS_EventKind kind, OS_Key key)
{
    OS_Event *result = 0;
    for (OS_Event *event = ui_state->eventList->first; event != 0; event = event->next)
    {
        if (event->kind == kind && event->key == key)
        {
            result = event;
            break;
        }
    }
    return result;
}

function OS_Event *
UI_GetEvent(OS_EventKind kind)
{
    OS_Event *result = 0;
    for (OS_Event *event = UI_EventList()->first; event != 0; event = event->next)
    {
        if (event->kind == kind)
        {
            result = event;
            break;
        }
    }
    return result;
}


//////////////////////////////////////////////
//~ @UI_Styling
//- Background color
inline_function HMM_Vec4
UI_GetBgColor(void)
{
    return UI_GetColor(ui_state->bgColorDefault, ui_state->bgColorStack, &ui_state->bgColorNext);
}

inline_function void
UI_PushBgColor(HMM_Vec4 color)
{
    UI_ColorNode *node = M_PushStruct(UI_FrameArena(), UI_ColorNode);
    node->color = color;
    StackPush(ui_state->bgColorStack, node);
}

inline_function void
UI_PopBgColor(void)
{
    StackPop(ui_state->bgColorStack);
}

inline_function void
UI_SetNextBgColor(HMM_Vec4 color)
{
    UI_SetNextColor(&ui_state->bgColorNext, color);
}

//- Border Color
inline_function HMM_Vec4 
UI_GetBorderColor(void)
{
    return HMM_V4(0.1f, 0.1f, 0.1f, 1.0f);
}

//- Text color
inline_function HMM_Vec4
UI_GetTextColor(void)
{
    return UI_GetColor(ui_state->textColorDefault, ui_state->textColorStack, &ui_state->textColorNext);
}

inline_function void
UI_PushTextColor(HMM_Vec4 color)
{
    UI_ColorNode *node = M_PushStruct(UI_FrameArena(), UI_ColorNode);
    node->color = color;
    StackPush(ui_state->textColorStack, node);
}

inline_function void
UI_PopTextColor(void)
{
    StackPop(ui_state->textColorStack);
}

inline_function void 
UI_SetNextTextColor(HMM_Vec4 color)
{
    UI_SetNextColor(&ui_state->textColorNext, color);
}

//- Text size
inline_function f32
UI_GetFontSize(void)
{
    return ui_state->fontSize;
}

//- General
inline_function HMM_Vec4 
UI_GetColor(HMM_Vec4 defaultColor, UI_ColorNode *stack, UI_NextColor *next)
{
    HMM_Vec4 result;
    if (next->notUsed)
    {
        result = next->color;
        next->notUsed = 0;
    }
    else if (stack)
    {
        result = stack->color;
    }
    else
    {
        result = defaultColor;
    }
    return result;
}

inline_function void
UI_SetNextColor(UI_NextColor *nextColor, HMM_Vec4 color)
{
    Assert(nextColor);
    nextColor->color = color;
    nextColor->notUsed = 1;
}

//////////////////////////////////////////////
//~ State setters
inline_function b32
UI_IsHot(UI_Widget *widget)
{
    b32 result = UI_KeyMatch(widget->key, ui_state->hotKey);
    
    // TODO(gsp): remove
    {
        //UI_Widget *prevWidget = UI_GetHashedWidgetPreviousFrame(widget);
        UI_Widget *hotWidget  = 0;
        for (UI_Widget *hashedWidget = &ui_state->currentWidgetTable[UI_IndexFromKey(ui_state->hotKey)]; 
             hashedWidget && UI_IsKeyValid(hashedWidget->key);
             hashedWidget = hashedWidget->hashNext)
        {
            if (UI_KeyMatch(widget->key, hashedWidget->key))
            {
                hotWidget = hashedWidget;
                break;
            }
        }
    }
    
    return result;
}

inline_function b32
UI_IsActive(UI_Widget *widget)
{
    b32 result = UI_KeyMatch(widget->key, ui_state->activeKey);
    return result;
}

inline_function b32 
UI_IsFocus(UI_Widget *widget)
{
    b32 result = UI_IsFocusFromKey(widget->key);
    return result;
}

inline_function b32 
UI_IsFocusFromKey(UI_Key key)
{
    b32 result = UI_KeyMatch(key, ui_state->focusKey);
    return result;
}

inline_function void
UI_SetHot(UI_Widget *widget)
{
    if (UI_IsKeyNull(ui_state->activeKey))
    {
        ui_state->hotKey = widget->key;
    }
}


inline_function void
UI_SetNotHot(UI_Widget *widget)
{
    if (UI_IsHot(widget))
    {
        UI_SetNotActive(widget);
        ui_state->hotKey = UI_KeyNull();
    }
}

inline_function void
UI_SetActive(UI_Widget *widget)
{
    ui_state->activeKey = widget->key;
    UI_SetFocus(widget);
}

inline_function void
UI_SetNotActive(UI_Widget *widget)
{
    if (UI_IsActive(widget))
    {
        ui_state->activeKey = UI_KeyNull();
    }
}

inline_function void
UI_SetFocus(UI_Widget *widget)
{
    ui_state->focusKey = widget->key;
}

//////////////////////////////////////////////
//~ General API
function b32
UI_Init(void)
{
    b32 result = 1;
    if (T_IsMainThread() && (0 == ui_state))
    {
        {
            M_Arena *arena = M_ArenaAlloc(MB(500));
            ui_state = M_PushStruct(arena, UI_State);
            ui_state->arena = arena;
            M_ArenaSetCheckpoint(ui_state->arena);
        }
        
        ui_state->currentFrameArena   = M_ArenaAlloc(GB(1));
        ui_state->previousFrameArena  = M_ArenaAlloc(GB(1));
        ui_state->currentWidgetState  = &ui_state->_widgetStates[0];
        ui_state->previousWidgetState = &ui_state->_widgetStates[1];
        
        ui_state->hotKey = UI_KeyNull();
        ui_state->activeKey = UI_KeyNull();
        
        //- @UI_StylingInit 
        
        // font
        ui_state->fontSize = 25.f;
        ui_state->font = FP_LoadFont(S8Lit("C:/Windows/fonts/arial.ttf"));
        if (!FP_IsHandleValid(ui_state->font))
        {
            result = 0;
        }
        
        ui_state->textPadding[Axis2_X] = 5.f;
        ui_state->textPadding[Axis2_Y] = 2.5f;
        ui_state->gap = 5.f;
        
        // Colors
        ui_state->bgColorDefault = HMM_V4(0.2f, 0.2f, 0.2f, 1.0f);
        ui_state->textColorDefault = HMM_V4(1.f, 1.f, 1.f, 1.f);
    }
    return result;
}

function void
UI_FrameBegin(OS_Handle os_window, OS_EventList *eventList)
{
    ui_state->eventList = eventList;
    ui_state->os_window = os_window;
    
    M_ArenaClear(ui_state->currentFrameArena);
    ui_state->currentWidgetTable = M_PushArray(ui_state->currentFrameArena, UI_Widget, UI_STATE_WIDGETS_TABLE_COUNT);
    
    MemoryZeroStruct(ui_state->currentWidgetState);
    ui_state->currentWidgetState->root = UI_WidgetMake(0, S8Lit("ui_root_widget"));
    HMM_Vec2 windowSize = Dim2F32(OS_ClientRectFromWindow(os_window));
    UI_WidgetEquipSize(ui_state->currentWidgetState->root, Axis2_X, UI_SizeMake(UI_SizeKind_Pixels, 1.0f, windowSize.x));
    UI_WidgetEquipSize(ui_state->currentWidgetState->root, Axis2_Y, UI_SizeMake(UI_SizeKind_Pixels, 1.0f, windowSize.y));
    UI_PushParent(ui_state->currentWidgetState->root);
}

function void
UI_FrameEnd(M_Arena *arena, R_CmdList *cmdList)
{
    UI_Layout();
    UI_Render(arena, cmdList);
    
    //- Clear stuff
    ui_state->prevFirstDrawOnTopNode = ui_state->currFirstDrawOnTopNode;
    ui_state->prevLastDrawOnTopNode  = ui_state->currLastDrawOnTopNode;
    ui_state->currFreeDrawOnTopNode  = 0;
    ui_state->currFirstDrawOnTopNode = 0;
    ui_state->currLastDrawOnTopNode  = 0;
    
    //- Swap stuff
    Swap(UI_Widget *, ui_state->currentWidgetTable, ui_state->previousWidgetTable);
    Swap(UI_WidgetState *, ui_state->currentWidgetState, ui_state->previousWidgetState);
    Swap(M_Arena *, ui_state->currentFrameArena, ui_state->previousFrameArena);
    ++ui_state->frameIndex;
}

//////////////////////////////////////////////
//~ @Autolayout
function void
UI_CalculateConstantSizes(void)
{
    UI_ForWidget_Linear(widget)
    {
        For_Axis2(axis)
        {
            UI_Size *semanticSize = &widget->semanticSizes[axis];
            f32 *computedSize = &widget->computedSize[axis];
            
            switch(semanticSize->kind)
            {
                default: break;
                
                case UI_SizeKind_Pixels:
                {
                    *computedSize = semanticSize->value;
                }break;
                
                case UI_SizeKind_TextContent:
                {
                    *computedSize = ui_state->textPadding[axis] * 2.f;
                    if (widget->displayString.size)
                    {
                        *computedSize += widget->glyphRun.axisAdvance[axis];
                    }
                    else if (axis == Axis2_Y)
                    {
                        *computedSize += UI_GetFontSize();
                    }
                }break;
            }
        }
    }
}

function void 
UI_CalculateUpwardsDependentSizes(void)
{
    UI_ForWidget_PreOrder(widget, 0)
    {
        if (widget->parent == 0)
        {
            continue;
        }
        
        For_Axis2(axis)
        {
            UI_Size *semanticSize = &widget->semanticSizes[axis];
            f32 *computedSize = &widget->computedSize[axis];
            
            switch(semanticSize->kind)
            {
                default: break;
                
                case UI_SizeKind_PercentOfParent:
                {
                    f32 parentComputedSize = widget->parent->computedSize[axis];
                    Assert(semanticSize->value);
                    *computedSize = semanticSize->value * parentComputedSize - ui_state->gap * 2.f;
                }break;
            }
        }
    }
}

function void
UI_CalculateDownwardsDependentSizes(void)
{
    UI_ForWidget_PostOrder(widget, 0)
    {
        For_Axis2(axis)
        {
            UI_Size *semanticSize = &widget->semanticSizes[axis];
            f32 *computedSize = &widget->computedSize[axis];
            
            switch(semanticSize->kind)
            {
                default: break;
                
                case UI_SizeKind_ChildrenSum:
                {
                    f32 grow = axis == widget->axis ?
                        UI_ChildrenSumFromWidget(widget, axis) : (UI_ChildrenMaxSizeFromWidget(widget, axis) + 2.f*ui_state->gap);
                    
                    //*computedSize = (semanticSize->value == 0 ? 1 : semanticSize->value) * grow;
                    *computedSize = grow;
                    
                }break;
            }
        }
    }
}

function void
UI_SolveViolations(void)
{
    UI_ForWidget_PreOrder(widget, 0)
    {
        For_Axis2(axis)
        {
            f32 computedSize = widget->computedSize[axis];
            f32 childrenSum = UI_ChildrenSumFromWidget(widget, axis);
            f32 dif = computedSize - childrenSum;
            if (dif < 0.f)
            {
                
                u32 zeroStrictnessChildren = 0;
                UI_ForWidget_Children(widget, child)
                {
                    if (child->semanticSizes[axis].strictness != 0.f || (child->flags & UI_WidgetFlag_NoAutolayout))
                        continue;
                    
                    zeroStrictnessChildren += 1;
                }
                
                if (zeroStrictnessChildren != 0)
                {
                    f32 originalDif = dif;
                    UI_ForWidget_Children(widget, child)
                    {
                        if (child->semanticSizes[axis].strictness != 0.f || (child->flags & UI_WidgetFlag_NoAutolayout))
                            continue;
                        
                        //f32 oneMinusStrictness = 1.f - child->semanticSizes[axis].strictness;
                        f32 sizeShrink = AbsoluteValueF32(originalDif) / (f32)zeroStrictnessChildren;
                        
                        sizeShrink = sizeShrink < child->computedSize[axis] ? sizeShrink : child->computedSize[axis];
                        
                        child->computedSize[axis] -= sizeShrink;
                        dif += sizeShrink;
                    }
                }
                
                UI_ForWidget_Children(widget, child)
                {
                    if (dif > 0.f)
                        break;
                    
                    f32 oneMinusStrictness = 1.f - child->semanticSizes[axis].strictness;
                    f32 sizeShrink = oneMinusStrictness * AbsoluteValueF32(dif);
                    sizeShrink = sizeShrink < child->computedSize[axis] ? sizeShrink : child->computedSize[axis];
                    
                    child->computedSize[axis] -= sizeShrink;
                    dif += sizeShrink;
                }
            }
        }
    }
}

function void
UI_Layout(void)
{
    UI_CalculateConstantSizes();
    UI_CalculateUpwardsDependentSizes();
    UI_CalculateDownwardsDependentSizes();
    UI_SolveViolations();
    
    //- 5. (Pre-order) Compute relative positions, compute final screen coords
    UI_ForWidget_PreOrder(widget, 0)
    {
        For_Axis2(axis)
        {
            f32 *computedRelPos = &widget->computedRelPos[axis];
            Axis2 parentGrowingAxis = Axis2_Y;
            
            if (widget->flags & UI_WidgetFlag_NoAutolayout)
            {
                *computedRelPos = widget->manualPos[axis];
            }
            else
            {
                if (widget->parent)
                {
                    *computedRelPos = widget->parent->computedRelPos[axis];
                    *computedRelPos += ui_state->gap;
                    parentGrowingAxis = widget->parent->axis;
                }
                
                // Get last child that isn't draw on top
                if (axis == parentGrowingAxis)
                {
                    for (UI_Widget *prev = widget->prev; prev != 0; prev = prev->prev)
                    {
                        if (!(prev->flags & UI_WidgetFlag_NoAutolayout))
                        {
                            *computedRelPos = prev->computedRelPos[axis] + prev->computedSize[axis];
                            *computedRelPos += ui_state->gap;
                            break;
                        }
                    }
                }
            }
        }
        
        widget->finalRect.p0.x = widget->computedRelPos[Axis2_X];
        widget->finalRect.p0.y = widget->computedRelPos[Axis2_Y];
        widget->finalRect.p1.x = widget->finalRect.p0.x + widget->computedSize[Axis2_X];
        widget->finalRect.p1.y = widget->finalRect.p0.y + widget->computedSize[Axis2_Y];
    }
}

function void
UI_Render(M_Arena *arena, R_CmdList *cmdList)
{
    UI_ForWidget_PreOrder(widget, UI_WidgetFlag_DrawOnTop)
    {
        if (widget == ui_state->currentWidgetState->root)
            continue;
        
        UI_RenderWidget(arena, cmdList, widget);
    }
    
    // Render 'OnTop' Widgets
    for (UI_DrawOnTopNode *topNode = ui_state->currFirstDrawOnTopNode; topNode != 0; topNode = topNode->next)
    {
        UI_ForWidget_PreOrderSubtree(drawOnTopChild, topNode->widget)
        {
            UI_RenderWidget(arena, cmdList, drawOnTopChild);
        }
    }
    
    
    
}

function void
UI_RenderWidget(M_Arena *arena, R_CmdList *cmdList, UI_Widget *widget)
{
    if (widget->flags & UI_WidgetFlag_DrawBackground)
    {            
        HMM_Vec4 bgColor = widget->color;
        if (widget->flags & UI_WidgetFlag_ActiveAnimation && UI_IsActive(widget))
        {
            bgColor = Add4F32(bgColor, HMM_V4(0.3f, 0.3f, 0.3f, 0.f));
        }
        else if (widget->flags & UI_WidgetFlag_HotAnimation && UI_IsHot(widget))
        {
            bgColor = Add4F32(widget->color, HMM_V4(0.15f, 0.15f, 0.15f, 0.f));
        }
        
        R_Cmd bg = {0};
        bg.kind = R_CmdKind_Rect2D;
        bg.rect2d = widget->finalRect;
        bg.cornerColors[0] = bgColor;
        bg.cornerColors[1] = bgColor;
        bg.cornerColors[2] = bgColor;
        bg.cornerColors[3] = bgColor;
        bg.edgeSoftness = 2.f;
        R_CmdPush(arena, cmdList, bg);
    }
    
    if (widget->flags & UI_WidgetFlag_DrawBorder)
    {
        R_Cmd border = {0};
        border.kind = R_CmdKind_Rect2D;
        border.rect2d = widget->finalRect;
        border.cornerColors[0] = widget->borderColor;
        border.cornerColors[1] = widget->borderColor;
        border.cornerColors[2] = widget->borderColor;
        border.cornerColors[3] = widget->borderColor;
        border.borderThickness = 1.f;
        border.edgeSoftness = 2.f;
        R_CmdPush(arena, cmdList, border);
    }
    
    if (widget->flags & UI_WidgetFlag_DrawText)
    {
        FC_GlyphRun *glyphRun = &widget->glyphRun;
        HMM_Vec2 pos = widget->finalRect.p0;
        pos.x += ui_state->textPadding[Axis2_X];
        for (u32 it = 0; it < widget->displayString.size; ++it)
        {
            FC_GlyphInfo *glyphInfo = glyphRun->infos[it];
            Rng2F32 charRect = R2F32(pos, HMM_Add(pos, glyphInfo->dim));
            HMM_Vec4 textColor = widget->textColor;
            
            R_TexHandle texHandle = glyphInfo->texture;
            Rng2F32 uv = Rng2F32FromRng2(glyphInfo->uv);
            {
                HMM_Vec2 texSize = HMM_Vec2FromVec(R_SizeFromTexture2D(texHandle));
                uv.p0 = HMM_Div(uv.p0, texSize);
                uv.p1 = HMM_Div(uv.p1, texSize);
            }
            
            R_Cmd cmd = {0};
            cmd.kind = R_CmdKind_Rect2D;
            cmd.rect2d = charRect;
            cmd.uv = uv; 
            cmd.cornerColors[0] = cmd.cornerColors[1] = cmd.cornerColors[2] = cmd.cornerColors[3] = textColor;
            cmd.texHandle = texHandle;
            R_CmdPush(arena, cmdList, cmd);
            
            pos.x += glyphInfo->advanceWidth + glyphRun->kerningAdjustments[it];
        }
    }
}

//////////////////////////////////////////////
//~ State getters
inline_function UI_Widget *
UI_Root(void)
{
    return ui_state->currentWidgetState->root;
}

inline_function OS_Handle
UI_Window(void)
{
    return ui_state->os_window;
}

inline_function OS_EventList *
UI_EventList(void)
{
    return ui_state->eventList;
}

inline_function M_Arena *
UI_FrameArena(void)
{
    return ui_state->currentFrameArena;
}

inline_function u64
UI_FrameIndex(void)
{
    return ui_state->frameIndex;
}

inline_function HMM_Vec2
UI_Mouse(void)
{
    return UI_EventList()->mousePos;
}

inline_function HMM_Vec2
UI_MouseDelta(void)
{
    return UI_EventList()->mouseDelta;
}

//////////////////////////////////////////////
//~ Widget iterators

//- Linear iteratators 

function UI_Widget *
UI_IterLinearGetFirst(void)
{
    UI_Widget *widget = &ui_state->currentWidgetTable[0];
    if (UI_IsKeyNull(widget->key))
    {
        widget = UI_IterLinearGetNext(widget);
    }
    return widget;
}

function UI_Widget *
UI_IterLinearGetNext(UI_Widget *prev)
{
    Assert(prev);
    UI_Widget *result = 0;
    if (prev->hashNext)
    {
        Assert(UI_IsKeyValid(prev->hashNext->key));
        result = prev->hashNext;
    }
    else
    {
        u64 keyIndex = UI_IndexFromKey(prev->key);
        UI_Widget *widget = prev;
        for (u64 it = keyIndex+1; it < UI_STATE_WIDGETS_TABLE_COUNT; ++it)
        {
            b32 found = 0;
            for (UI_Widget *hashWidget = widget->hashNext; hashWidget != 0; hashWidget = hashWidget->hashNext)
            {
                if (UI_IsKeyValid(hashWidget->key))
                {
                    widget = hashWidget;
                    found = 1;
                    break;
                }
            }
            if (found) { break; }
            
            widget = &ui_state->currentWidgetTable[it];
            if (UI_IsKeyValid(widget->key))
            {
                break;
            }
        }
        
        if (UI_IsKeyValid(widget->key))
        {
            result = widget;
        }
    }
    return result;
}

//- PreOrder iterattion
function UI_Widget *
UI_IterPreOrderGetFirst(void)
{
    return ui_state->currentWidgetState->root; 
}

function UI_Widget *
UI_IterPreOrderGetNext(UI_Widget *prev, UI_WidgetFlags flagsToPrune)
{
    UI_Widget *result = 0;
    
#if 1
    UI_ForWidget_Children(prev, child)
    {
        if (!(child->flags & flagsToPrune))
        {
            result = child;
            break;
        }
    }
    
    if (!result)
    {
        UI_ForWidget_Parents(prev, parent)
        {
            
            UI_ForWidget_Siblings(parent->next, next)
            {
                if (!(next->flags & flagsToPrune))
                {
                    result = next;
                    break;
                }
            }
            
            
            if (result)
                break;
        }
    }
#else
    if (prev->first)
    {
        if (prev->first->flags & flagsToPrune)
        {
            for (UI_Widget *widget = prev->first->next;
                 widget != 0;
                 widget = widget->first)
            {
                if (!(widget->flags & flagsToPrune))
                {
                    result = widget;
                    break;
                }
            }
        }
        else
        {
            result = prev->first;
        }
    }
    else
    {
        for (UI_Widget *widget = prev;
             widget != 0;
             widget = widget->parent)
        {
            if (widget->next && !(widget->next->flags & flagsToPrune))
            {
                result = widget->next;
                break;
            }
        }
    }
#endif
    
    return result;
}

//- PostOrder
function UI_Widget *
UI_IterPostOrderGetFirst(void)
{
    UI_Widget *result = ui_state->currentWidgetState->root;
    for (; result->first != 0; result = result->first) {}
    return result;
}

function UI_Widget *
UI_IterPostOrderGetNext(UI_Widget *prev, UI_WidgetFlags flagsToPrune)
{
    // TODO(gsp): @ui flagsToPrune on post order
    UnreferencedParameter(flagsToPrune);
    UI_Widget *result = 0;
    if (prev->next)// && !(prev->next->flags & flagsToPrune))
    {
        result = prev->next;
        for (; result->first != 0; result = result->first)
        {
            if (result->first) // && !(result->first->flags & flagsToPrune))
            {
                break;
            }
        }
    }
    else
    {
        result = prev->parent;
    }
    return result;
}

//- Subtree iterators
function UI_Widget *
UI_IterSubtreePreOrderGetNext(UI_Widget *subtreeRoot, UI_Widget *prev)
{
    UI_Widget *result = 0;
    if (prev->first) // && !(widget->next->flags & flagsToPrune))
    {
        result = prev->first;
    }
    else
    {
        for (UI_Widget *widget = prev;
             widget != 0 && widget != subtreeRoot;
             widget = widget->parent)
        {
            if (widget->next) // && !(widget->next->flags & flagsToPrune))
            {
                result = widget->next;
                break;
            }
        }
        
    }
    return result;
}



//////////////////////////////////////////////
//~ End of file
#endif //UI_INCLUDE_CORE_INCLUDE_C
