#ifndef CAMERA_INCLUDE_H
#define CAMERA_INCLUDE_H

typedef struct Camera Camera;
struct Camera
{
    HMM_Vec3 pos;
    HMM_Vec3 right;
    HMM_Vec3 fwd;
    HMM_Vec3 up;
    
    // NOTE(gsp): rot is used by free cameras and is in degrees
    union
    {
        struct
        {
            f32 rotX;
            f32 rotY;
        };
        HMM_Vec2 rot;
    };
    
    f32 fov;
};

function Camera CameraMake(HMM_Vec3 pos, HMM_Vec3 target, f32 fov);
function void   CameraUpdate(Camera *cam, HMM_Vec3 pos, HMM_Vec3 target);

function void FreeCameraUpdate(Camera *cam, HMM_Vec3 posDelta, HMM_Vec2 rotDelta, f32 movSpd, f32 dt);

/////////////////////////////////////////////////////
//~ END OF FILE
#endif //CAMERA_INCLUDE_H
