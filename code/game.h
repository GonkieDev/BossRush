#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#if COMPILER_MSVC
#pragma warning(disable: 4820)
#endif


typedef struct G_State G_State;
struct G_State
{
    M_Arena *arena;
    
    //-
    Camera mainCam;
    
    //- Window
    b32 windowIsFocused;
    b32 clipMouse;
    OS_Handle osWindowHandle;
    
    //- Input
    Input inputs[2];
    Input *input;
    Input *oldInput;
    
    //- Assets
    EzmModel *cubeModel;
    R_MeshKey cube;
    EzmModel *uvsphereModel;
    R_MeshKey uvsphere;
    
    EzmModel *planeModel;
    R_MeshKey plane;
    
    EzmModel *mvMachineModel;
    R_MeshKey mvMachine;
    R_TexHandle mvMachineTex;
    
    R_TexHandle mageSS;
    
    //- UI
    FP_Handle uiFont;
    f32 uiFontSize;
    
    //- Physics
    P_PhysicsState physicsState;
    
    //- Stats
    f64 runTime;
    
    //- Debug
#if BUILD_DEBUG
    EzmModel *arrowModel;
    R_MeshKey arrow;
    
    b32 drawDebugInfo;
#endif // #if BUILD_DEBUG
};

#if COMPILER_MSVC
#pragma warning(default: 4820)
#endif

function b32 G_Init(OS_Handle osWindowHandle);
function void G_Main(void);
function void G_Shutdown(void);


/////////////////////////////////////////////////////
//~ END OF FILE
#endif //GAME_INCLUDE_H
