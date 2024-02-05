function Camera
CameraMake(HMM_Vec3 pos, HMM_Vec3 target, f32 fov)
{
    HMM_Vec3 up    = HMM_V3(0.f, 1.f, 0.f);
    HMM_Vec3 fwd   = HMM_Norm(HMM_Sub(target, pos));
    HMM_Vec3 right = HMM_Norm(HMM_Cross(up, fwd));
    //up = HMM_Norm(HMM_Cross(right, fwd));
    Camera cam = {
        .pos   = pos,
        .fwd   = fwd,
        .right = right,
        .up    = up,
        .fov   = fov,
    };
    return cam;
}

function void
FreeCameraUpdate(Camera *cam, HMM_Vec3 posDelta, HMM_Vec2 rotDelta, f32 movSpd, f32 dt)
{
    rotDelta = HMM_Mul(rotDelta, dt);
    posDelta = HMM_Mul(posDelta, movSpd * dt);
    
    HMM_Vec2 rot = HMM_Add(cam->rot, rotDelta);
    // Clamp vertical rotation
    const f32 clampRotY = 88.f;
    rot.y = Clamp(-clampRotY, rot.y, clampRotY);
    
    // Calculate new axis
    HMM_Vec3 fwd;
    {
        f32 pitch = RadFromDegF32(rot.y);
        f32 yaw   = RadFromDegF32(rot.x);
        f32 cosPitch = cosf(pitch);
        fwd.x = cosf(yaw) * cosPitch;
        fwd.y = sinf(pitch);
        fwd.z = sinf(yaw) * cosPitch;
        fwd   = HMM_Norm(fwd);
    }
    
    HMM_Vec3 up = HMM_V3(0.f, 1.f, 0.f);
    HMM_Vec3 right = HMM_Norm(HMM_Cross(up, fwd));
    //up = HMM_Norm(HMM_Cross(right, up));
    
    // Move camera
    HMM_Vec3 move = {0};
    if ((posDelta.x != 0) || (posDelta.y != 0) || (posDelta.z != 0))
    {
        move   = HMM_Mul(fwd, posDelta.z);
        move   = HMM_Add(HMM_Mul(right, -posDelta.x), move);
        move.y = 0.f;
        move   = HMM_Add(HMM_Mul(HMM_V3(0.f, 1.f, 0.f), posDelta.y), move);
        move   = HMM_Norm(move);
        move   = HMM_Mul(move, movSpd * dt);
    }
    
    // Update values
    cam->fwd   = fwd;
    cam->right = right;
    cam->up    = up;
    cam->pos   = HMM_Add(cam->pos, move);
    cam->rot   = rot;
}