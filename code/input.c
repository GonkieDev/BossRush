inline_function void
I_Swap(void)
{
    Swap(Input*, g_state->input, g_state->oldInput);
    MemoryZeroStruct(g_state->input);
    
    Input *oldInput = g_state->oldInput;
    Input *input    = g_state->input;
    for (u32 keyIdx = 0; keyIdx < OS_Key_Count; ++keyIdx)
    {
        I_ButtonState *persist    = &input->buttonStates[keyIdx];
        I_ButtonState *oldPersist = &oldInput->buttonStates[keyIdx];
        persist->endedDown = oldPersist->endedDown;
    }
}

inline_function void
I_KeyUpdate(OS_Key key, b32 isDown, b32 wasDown, f32 dt)
{
    Input *input = g_state->input;
    I_ButtonState *buttonState = &input->buttonStates[key];
    
    if (wasDown != isDown)
    {
        buttonState->endedDown = (b16)isDown;
        buttonState->halfTransitions += 1;
        
        if (buttonState->endedDown)
        {
            buttonState->duration += dt;
        }
    }
}

inline_function void
I_MouseUpdate(OS_Event *mouseMoveEvent)
{
    Assert(mouseMoveEvent->kind == OS_EventKind_MouseMove);
    Input *input = g_state->input;
    input->cameraRotate.x = (f32)mouseMoveEvent->mouseDelta.x;
    input->cameraRotate.y = -((f32)mouseMoveEvent->mouseDelta.y);
}

inline_function void
I_ClearCurrentInput(void)
{
    MemoryZeroStruct(g_state->input);
}

//~
inline_function b32
I_KeyIsDown(OS_Key key)
{
    Input *input = g_state->input;
    I_ButtonState *buttonState = &input->buttonStates[key];
    return (buttonState->endedDown);
}

//~
inline_function void
I_ProcessActions(void)
{
    Input *input = g_state->input;
    {
        HMM_Vec3 move = {0};
        if (I_KeyIsDown(OS_Key_D)) { move.x += 1.f; }
        if (I_KeyIsDown(OS_Key_A)) { move.x -= 1.f; }
        
        if (I_KeyIsDown(OS_Key_W)) { move.z += 1.f; }
        if (I_KeyIsDown(OS_Key_S)) { move.z -= 1.f; }
        
#if BUILD_DEBUG
        if (I_KeyIsDown(OS_Key_Q)) { move.y += 1.f; }
        if (I_KeyIsDown(OS_Key_E)) { move.y -= 1.f; }
#endif
        
        if (move.x != 0.f || move.y != 0.f || move.z != 0.f)
            move = HMM_Norm(move);
        input->move = move;
    }
    
#if BUILD_DEBUG
    input->freeCameraSpdUp = I_KeyIsDown(OS_Key_Shift);
#endif
}