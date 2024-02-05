#ifndef BOSSRUSH_INCLUDE_H
#define BOSSRUSH_INCLUDE_H

#define G_MAX_ENTITIES 500

#if COMPILER_MSVC
#pragma warning(disable: 4820)
#endif

typedef struct G_State G_State;
struct G_State
{
    M_Arena *arena;
    
    b32 invGrav;
    
    //- Cameras
    Camera *activeCam;
    Camera  mainCam;
    HMM_Vec3 playerCamOffset;
    
    //- Input
    Input *input;
    Input *oldInput;
    Input inputs[2];
    
    //- Entities
    G_Entity entities[G_MAX_ENTITIES];
    u32      entitiesCount;
    
    //- Assets
    SpriteAnimTreeDir mageAnimDirTree;
    SA_Slice fireballSpr;
    
    // Base models
    EzmModel *cubeModel;
    R_MeshKey cube;
    EzmModel *uvsphereModel;
    R_MeshKey uvsphere;
    EzmModel *planeModel;
    R_MeshKey plane;
    EzmModel *filledCircleModel;
    R_MeshKey filledCircle;
    EzmModel *wallModel;
    R_MeshKey wall;
    
    // Audio
    AudioHandle tango;
    AudioHandle water;
    AudioHandle kahoot_wurls;
    
    //- Draw
    R_CmdList RCmdList;
    
    //- UI
    FP_Handle uiFont;
    f32 uiFontSize;
    
    //- Sprite Atlas 
    SA_State *sa_state;
    
    //- Physics
    P2D_State p2d_state;
    
    //- Window
    b32 windowIsFocused;
    b32 clipMouse;
    u32 __pad1;
    OS_Handle osWindowHandle;
    
    //- Stats
    f64 runTime;
    u64 frameCount;
    
    //- Debug
#if BUILD_DEBUG
    EzmModel *arrowModel;
    R_MeshKey arrow;
    
    Camera freeCam;
    b32 drawDebugInfo;
    b32 showHitBoxes;
    u32 __pad4;
#endif // #if BUILD_DEBUG
};

#if COMPILER_MSVC
#pragma warning(default: 4820)
#endif


function b32  G_Init(OS_Handle osWindowHandle);
function void G_Main(void);
function void G_DrawDebugInfo(f32 dt);
function void G_Shutdown(void);

/////////////////////////////////////////////////////
//~ END OF FILE
#endif //BOSSRUSH_INCLUDE_H
