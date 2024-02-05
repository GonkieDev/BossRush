#ifndef UI_BUILDER_INCLUDE_C
#define UI_BUILDER_INCLUDE_C
//~

function UI_Widget *
UI_ButtonMake(String8 string)
{
    UI_Widget *button = UI_WidgetMake(UI_WidgetFlag_Clickable|
                                      UI_WidgetFlag_DrawBorder|
                                      UI_WidgetFlag_DrawText|
                                      UI_WidgetFlag_DrawBackground|
                                      UI_WidgetFlag_HotAnimation|
                                      UI_WidgetFlag_ActiveAnimation, 
                                      string);
    
    UI_WidgetEquipSizes(button, UI_SizeText(1.f));
    return button;
}

function UI_Comm
UI_Button_(String8 string)
{
    UI_Widget *button = UI_ButtonMake(string);
    UI_Comm comm = UI_CommFromWidget(button);
    return comm;
}

function UI_Comm
UI_Checkbox_(b32 *pBool, String8 string)
{
    UI_Comm comm;
    UI_Widget *boundingBox = 0;
    // Bounding box
    {
        String8 boundingBoxString = PushStr8Fmt(UI_FrameArena(), "%.*s boundingbox", S8VArg(string));
        boundingBox = UI_WidgetMake(UI_WidgetFlag_Clickable|
                                    UI_WidgetFlag_DrawBorder|
                                    UI_WidgetFlag_HotAnimation|
                                    UI_WidgetFlag_ActiveAnimation, 
                                    boundingBoxString);
        
        boundingBox->borderColor = HMM_V4(0.1f, 0.1f, 0.1f, 1.0f);
        UI_WidgetEquipSizes(boundingBox, UI_SizeMake(UI_SizeKind_Pixels, 1.0f, ui_state->fontSize * 1.25f));
        
        
        comm = UI_CommFromWidget(boundingBox);
    }
    
    UI_Parent(boundingBox)
    {
        // Inner box
        if (*pBool)
        {
            String8 innerBoxString = PushStr8Fmt(UI_FrameArena(), "%.*s innerbox", S8VArg(string));
            UI_Widget *innerBox = UI_WidgetMake(UI_WidgetFlag_DrawBackground |
                                                UI_WidgetFlag_HotAnimation|
                                                UI_WidgetFlag_ActiveAnimation, 
                                                innerBoxString);
            
            HMM_Vec4 innerBoxBgDefColor = HMM_V4(0.65f, 0.65f, 0.65f, 1.f);
            HMM_Vec4 innerBoxBgColor = UI_IsActive(boundingBox) ? 
                Add4F32(innerBoxBgDefColor, HMM_V4(0.3f, 0.3f, 0.3f, 0.f)) :  
            UI_IsHot(boundingBox) ? Add4F32(innerBoxBgDefColor, HMM_V4(0.15f, 0.15f, 0.15f, 0.f)) : innerBoxBgDefColor;
            innerBox->color = innerBoxBgColor;
            
            UI_Size innerBoxUISize = UI_SizeMake(UI_SizeKind_PercentOfParent, 1.0f, 0.8f);
            UI_WidgetEquipSize(innerBox, Axis2_X, innerBoxUISize);
            UI_WidgetEquipSize(innerBox, Axis2_Y, innerBoxUISize);
            
        }
        
    }
    
    if (comm.clicked)
    {
        *pBool = !(*pBool);
        LogInfo(0, "bool changed lel %d", *pBool);
    }
    
    return comm;
}

function UI_Widget *
UI_Text_(String8 text)
{
    UI_Widget *widget = UI_WidgetMake(UI_WidgetFlag_DrawText, text);
    UI_WidgetEquipSizes(widget, UI_SizeText(1.f));
    return widget;
}

function UI_Widget * 
UI_TextF(char *fmt, ...)
{
    UI_Widget *result = 0;
    {
        va_list args;
        va_start(args, fmt);
        String8 formattedString = PushStr8FmtVAList(UI_FrameArena(), fmt, args);
        va_end(args);
        
        result = UI_Text_(formattedString);
    }
    return result;
}

function void
UI_EvenSpacer(void)
{
    UI_Widget *spacer = UI_WidgetMake(0, S8Lit("spacer342347284"));
    UI_WidgetEquipSize(spacer, spacer->parent->axis, UI_SizeMake(UI_SizeKind_PercentOfParent, 0.0f, 1.f));
}

function UI_Widget *
UI_DropDownMenuMake_(String8 string, b32 *show)
{
    UI_Widget *menu = UI_WidgetMake(UI_WidgetFlag_DrawBackground | UI_WidgetFlag_DrawOnTop | UI_WidgetFlag_NoAutolayout | UI_WidgetFlag_Clickable, string);
    
    if (menu->firstTouchedGen != UI_FrameIndex())
    {
        UI_Comm comm = UI_CommFromWidget(menu);
        if (!comm.hoveredIncludingChildren)
        {
            *show = 0;
        }
    }
    
    return menu;
}

function void
UI_DropDownMenuStart(String8 string, b32 *show)
{
    UI_Widget *menu = UI_DropDownMenuMake_(string, show);
    UI_PushParent(menu);
}

function void
UI_DropDownMenuEnd(void)
{
    UI_PopParent();
}

function void
UI_TextInput(String8 name, String8 *buf, u64 maxLen)
{
    //- Create text input widget
    UI_Widget *textInput = 0;
    {
        String8 textInputMakeStr = PushStr8Fmt(UI_FrameArena(), "%.*s#textinput", S8VArg(name));
        UI_Key textInputKey = UI_KeyFromString(textInputMakeStr);
        
        b32 textInputIsFocused = UI_IsFocusFromKey(textInputKey);
        
        UI_WidgetFlags textInputFlags = UI_WidgetFlag_Clickable|
            UI_WidgetFlag_DrawBorder|
            UI_WidgetFlag_DrawBackground|
        (textInputIsFocused ? 
         0 : UI_WidgetFlag_HotAnimation| UI_WidgetFlag_ActiveAnimation);
        textInput = UI_WidgetMakeFromKey(textInputFlags, textInputMakeStr, textInputKey);
        UI_WidgetEquipSizes(textInput, UI_SizeMake(UI_SizeKind_TextContent, 1.0f, 0.f));
        textInput->borderColor = HMM_V4(0.1f, 0.1f, 0.1f, 1.0f);
    }
    
    UI_Comm comm = UI_CommFromWidget(textInput);
    OS_SetCursor(comm.hoveredIncludingChildren ? OS_CursorKind_TextSelect : 0);
    
    //- Draw text
    UI_Widget *text = 0;
    UI_Parent(textInput)
    {
        text = UI_Text_(*buf);
    }
    
    UI_Widget *prevTextWidget = UI_GetHashedWidgetPreviousFrame(text);
    
    //- gsp: Selection
    if (prevTextWidget)
    {
        //- Cursor pos
        if (comm.pressed)
        {
            f32 mouseRelativeOffset = UI_Mouse().x - prevTextWidget->finalRect.p0.x;
            f32 totalAdvance = ui_state->textPadding[Axis2_X];
            u32 it = 0;
            for (; it < buf->size; ++it)
            {
                totalAdvance += text->glyphRun.infos[it]->advanceWidth / 2.f;
                if (totalAdvance > mouseRelativeOffset) { break; }
                totalAdvance += text->glyphRun.infos[it]->advanceWidth / 2.f + text->glyphRun.kerningAdjustments[it];
            }
            textInput->cursorPos = it;
        }
        //- Cursor mark
        else if (UI_IsActive(textInput))
        {
            f32 mouseRelativeOffset = UI_Mouse().x - prevTextWidget->finalRect.p0.x;
            f32 totalAdvance = ui_state->textPadding[Axis2_X];
            u32 it = 0;
            for (; it < buf->size; ++it)
            {
                totalAdvance += text->glyphRun.infos[it]->advanceWidth / 2.f;
                if (totalAdvance > mouseRelativeOffset) { break; }
                totalAdvance += text->glyphRun.infos[it]->advanceWidth / 2.f + text->glyphRun.kerningAdjustments[it];
            }
            textInput->markPos = it;
        }
    }
    
    //- gsp: Handle text input
    if (UI_IsFocus(textInput))
    {
        //- gsp: Draw selection
        UI_Parent(textInput)
        {
            if (prevTextWidget)
            {    
                f32 fontSize = UI_GetFontSize();
                
                Assert(textInput->cursorPos <= buf->size);
                Assert(textInput->markPos <= buf->size);
                
                HMM_Vec2 cursorOffset = prevTextWidget->finalRect.p0;
                cursorOffset.x += ui_state->textPadding[Axis2_X];
                for (u32 it = 0; it < textInput->cursorPos; ++it)
                {
                    cursorOffset.x += text->glyphRun.infos[it]->advanceWidth + text->glyphRun.kerningAdjustments[it];
                }
                
                
                // gsp: Draw cursor bar
                {
                    UI_SetNextBgColor(UI_GetTextColor());
                    UI_Widget *cursorBar = UI_WidgetMake(UI_WidgetFlag_DrawBackground | UI_WidgetFlag_DrawOnTop | UI_WidgetFlag_NoAutolayout, S8Lit("##ui_cursor_bar_9999"));
                    
                    UI_WidgetEquipManualPos(cursorBar, cursorOffset);
                    UI_WidgetEquipSize(cursorBar, Axis2_X, UI_SizePixels(1.f, fontSize * 0.125f));
                    UI_WidgetEquipSize(cursorBar, Axis2_Y, UI_SizePP(1.f, 1.f));
                }
                
                // gsp: Draw mark & selection
                if (textInput->cursorPos != textInput->markPos)
                {
                    
                    HMM_Vec2 markOffset = prevTextWidget->finalRect.p0;
                    markOffset.x += ui_state->textPadding[Axis2_X];
                    for (u32 it = 0; it < textInput->markPos; ++it)
                    {
                        markOffset.x += text->glyphRun.infos[it]->advanceWidth + text->glyphRun.kerningAdjustments[it];
                    }
                    
                    if (cursorOffset.x != markOffset.x)
                    {
                        {
                            UI_SetNextBgColor(HMM_V4(1.0f, 0.f, 0.f, 0.25f));
                            UI_Widget *selectionQuad = UI_WidgetMake(UI_WidgetFlag_DrawBackground | UI_WidgetFlag_DrawOnTop | UI_WidgetFlag_NoAutolayout, S8Lit("##ui_text_selection_9999"));
                            
                            b32 cursorIsStart = cursorOffset.x <= markOffset.x;
                            f32 quadStart = cursorIsStart ? cursorOffset.x : markOffset.x;
                            f32 quadSize  = cursorIsStart ? markOffset.x - cursorOffset.x : cursorOffset.x - markOffset.x;
                            quadSize += fontSize * 0.125f;
                            
                            UI_WidgetEquipManualPos(selectionQuad, HMM_V2(quadStart, markOffset.y));
                            UI_WidgetEquipSize(selectionQuad, Axis2_X, UI_SizePixels(1.f, quadSize));
                            UI_WidgetEquipSize(selectionQuad, Axis2_Y, UI_SizePP(1.f, 1.f));
                        }
                        
                        {
                            UI_SetNextBgColor(UI_GetTextColor());
                            UI_Widget *markBar = UI_WidgetMake(UI_WidgetFlag_DrawBackground | UI_WidgetFlag_DrawOnTop | UI_WidgetFlag_NoAutolayout, S8Lit("##ui_mark_bar_9999"));
                            
                            UI_WidgetEquipManualPos(markBar, markOffset);
                            UI_WidgetEquipSize(markBar, Axis2_X, UI_SizePixels(1.f, fontSize * 0.125f));
                            UI_WidgetEquipSize(markBar, Axis2_Y, UI_SizePP(1.f, 1.f));
                        }
                    }
                }
            }
        }
        
        OS_EventList *eventList = UI_EventList();
        for (OS_Event *event = eventList->first; event != 0; event = event->next)
        {
            if (event->kind != OS_EventKind_Press)
                continue;
            
            StaticAssert(OS_Key_Count < 255, "");
            char c = (char)event->key;
            Rng1U64 range = R1U64(textInput->cursorPos, textInput->markPos);
            
            b32 characterInputted = 0;
            
            if (CharIsAlphaUpper(c))
            {
                if (c == 'A' && event->modifiers & OS_KeyModifierFlag_Ctrl) // Ctrl + A
                {
                    textInput->cursorPos = 0;
                    textInput->markPos = buf->size;
                    continue;
                }
                else
                {
                    c = (event->modifiers & OS_KeyModifierFlag_Caps) ?
                    ((event->modifiers & OS_KeyModifierFlag_Shift) ? CharToLower(c) : c) :
                    ((event->modifiers & OS_KeyModifierFlag_Shift) ? c : CharToLower(c));
                    
                    characterInputted = 1;
                    OS_EatEvent(eventList, event);
                }
            }
            else if (CharIsDigit(c))
            {
                characterInputted = 1;
                OS_EatEvent(eventList, event);
            }
            else if (event->key == OS_Key_Space)
            {
                characterInputted = 1;
                c = ' ';
                OS_EatEvent(eventList, event);
            }
            else if (event->key == OS_Key_Left)
            {
                b8 moveSelection = event->modifiers & OS_KeyModifierFlag_Shift;
                //b8 moveWord = event->modifiers & OS_KeyModifierFlag_Ctrl;  // TODO(gsp): @ui move word - text input
                u64 *pos = (textInput->cursorPos <= textInput->markPos) ? (&textInput->cursorPos) : (&textInput->markPos);
                
                if (moveSelection)
                {
                    textInput->markPos = textInput->markPos == 0 ? textInput->markPos : textInput->markPos - 1;
                }
                else
                {
                    textInput->cursorPos = (((*pos) == 0) || (textInput->cursorPos != textInput->markPos)) ? (*pos) : (*pos) - 1;
                    textInput->markPos = textInput->cursorPos;
                }
                
            }
            else if (event->key == OS_Key_Right)
            {
                b8 moveSelection = event->modifiers & OS_KeyModifierFlag_Shift;
                //b8 moveWord = event->modifiers & OS_KeyModifierFlag_Ctrl;  // TODO(gsp): @ui move word - text input
                u64 *pos = (textInput->cursorPos <= textInput->markPos) ? (&textInput->markPos) : (&textInput->cursorPos);
                
                if (moveSelection)
                {
                    textInput->markPos = textInput->markPos >= buf->size ? textInput->markPos : textInput->markPos + 1;
                }
                else
                {
                    textInput->cursorPos = (((*pos) >= buf->size) || (textInput->cursorPos != textInput->markPos)) ? (*pos) : (*pos) + 1;
                    textInput->markPos = textInput->cursorPos;
                }
            }
            else if (event->key == OS_Key_Backspace && buf->size)
            {
                Rng1U64 deleteRange = range;
                deleteRange.min = textInput->cursorPos <= textInput->markPos ? textInput->cursorPos : textInput->markPos;
                deleteRange.min = (deleteRange.min == deleteRange.max && deleteRange.min != 0) ? deleteRange.min - 1 : deleteRange.min;
                textInput->cursorPos = textInput->markPos = deleteRange.min;
                
                *buf = Str8PutRange(*buf, deleteRange, S8Lit(""), maxLen);
                OS_EatEvent(eventList, event);
                continue;
            }
            else
            {
                continue;
            }
            
            if (characterInputted)
            {
                *buf = Str8PutRange(*buf, range, Str8((u8*)(&c), 1), maxLen);
                textInput->cursorPos = Min3(textInput->cursorPos + 1, textInput->markPos + 1, buf->size);
                textInput->markPos = textInput->cursorPos;
            }
            
        }
    }
}

function void
UI_ToolTipFromTarget(String8 text, UI_Widget *target)
{
    UI_Comm comm = UI_CommFromWidget(target);
    if (comm.hovered)
    {
        UI_ToolTip(text, UI_Mouse());
    }
}

function void
UI_ToolTip(String8 text, HMM_Vec2 pos)
{
    UI_Widget *tooltip = UI_WidgetMake(UI_WidgetFlag_DrawOnTop| 
                                       UI_WidgetFlag_NoAutolayout|
                                       UI_WidgetFlag_DrawBackground|
                                       UI_WidgetFlag_DrawBorder|
                                       UI_WidgetFlag_DrawText, 
                                       text);
    UI_WidgetEquipSizes(tooltip, UI_SizeText(1.f));
    UI_WidgetEquipManualPos(tooltip, pos);
}

function void 
UI_Slider(String8 name, f32 *value, Rng1F32 limits)
{
    UI_Widget *sliderParent = UI_WidgetMake(UI_WidgetFlag_DrawBorder, PushStr8Fmt(UI_FrameArena(), "%.*s##slider_parent", S8VArg(name)));
    UI_WidgetEquipSize(sliderParent, Axis2_X, UI_SizePP(1.f, 1.f));
    UI_WidgetEquipSize(sliderParent, Axis2_Y, UI_SizePixels(1.f, 20.f));
    
    UI_Parent(sliderParent)
    {
        UI_SetNextBgColor(HMM_V4(1.f, 0.f, 0.f, 0.25f));
        UI_Widget *sliderLine = UI_WidgetMake(UI_WidgetFlag_DrawBackground, PushStr8Fmt(UI_FrameArena(), "%.*s##slider_line", S8VArg(name)));
        UI_WidgetEquipSizes(sliderLine, UI_SizePP(1.f, 1.f));
        
        UI_Widget *sliderLinePrev = UI_GetHashedWidgetPreviousFrame(sliderLine);
        if (sliderLinePrev)
        {
            UI_SetNextBgColor(HMM_V4(1.f, 0.3f, 0.3f, 0.75f));
            UI_Widget *sliderDot = UI_WidgetMake(UI_WidgetFlag_Clickable|
                                                 UI_WidgetFlag_DrawBackground|
                                                 UI_WidgetFlag_NoAutolayout,
                                                 PushStr8Fmt(UI_FrameArena(), "%.*s##slider_dot", S8VArg(name)));
            UI_WidgetEquipSizes(sliderDot, UI_SizePixels(1.f, 10.f));
            
            HMM_Vec2 dotPos = sliderLinePrev->finalRect.p0;
            dotPos.x += ((*value - limits.min) / Dim1F32(limits)) * sliderLinePrev->computedSize[Axis2_X];
            UI_WidgetEquipManualPos(sliderDot, dotPos);
            
            UI_Comm comm = UI_CommFromWidget(sliderDot);
            if (comm.dragged)
            {
                f32 mouseX = UI_Mouse().x;
                f32 mouseRelativeX = mouseX - sliderLinePrev->finalRect.p0.x;
                f32 coeff = (mouseRelativeX / sliderLinePrev->computedSize[Axis2_X]);
                coeff = Clamp(0.f, coeff, 1.f);
                *value = limits.min + Dim1F32(limits) * coeff;
                
                HMM_Vec2 delta = UI_MouseDelta();
                LogInfo(0, "Dragged! %f %f coeff: %f value: %f", delta.x, delta.y, coeff, *value);
            }
        }
    }
}


//////////////////////////////////////////////
//~ End of file
#endif //UI_BUILDER_INCLUDE_C