#ifndef INPUT_INCLUDE_H
#define INPUT_INCLUDE_H

typedef struct I_ButtonState I_ButtonState;
struct I_ButtonState
{
    f32 duration; // TODO
    b16 endedDown;
    u16 halfTransitions;
};

typedef struct Input Input;
struct Input
{
    I_ButtonState buttonStates[OS_Key_Count];
    HMM_Vec3 move;
    HMM_Vec2 cameraRotate;
#if BUILD_DEBUG
    b32 freeCameraSpdUp;
#endif
};

inline_function void I_Swap(void);
inline_function void I_KeyUpdate(OS_Key key, b32 isDown, b32 wasDown, f32 dt);
inline_function void I_MouseUpdate(OS_Event *mouseMoveEvent);
inline_function void I_ClearCurrentInput(void);

inline_function b32  I_KeyIsDown(OS_Key key);

inline_function void I_ProcessActions(void);

//~

///////////////////////////////////////////////////////////////////////////
//~ END OF FILE
#endif //INPUT_INCLUDE_H
