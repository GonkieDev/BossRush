function void
P_Update(P_PhysicsState *state, f64 dt)
{
    U_BEGIN_TIMED_BLOCK(Physics);
    
    const f64 H = 0.001; 
    state->physTimeAccu += dt;
    while (state->physTimeAccu > H)
    {
        P_FixedStep(state, H);
        state->physTimeAccu -= H;
    }
    
    U_END_TIMED_BLOCK(Physics);
}

////////////////////////////////////////////////////////////////////////////////////////////
//~ @P_FixedStep
function void
P_FixedStep(P_PhysicsState *state, f64 h)
{
    UnreferencedParameter(state);
    
    f32 hf32 = (f32)h;
    UnusedVariable(hf32);
}

////////////////////////////////////////////////////////////////////////////////////////////
//~
function void
P_RigidBodyAdd(P_PhysicsState *state, HMM_Vec3 aabb, f32 mass, HMM_Vec3 pos)
{
    P_RigidBody rb = {
        .aabb     = aabb,
        .mass     = mass,
        .IbodyInv = HMM_InvGeneral(P_InertiaTensorFromAABBDensity1(aabb.x, aabb.y, aabb.z)),
        
        .x        = pos,
    };
    state->rbs[state->rbsCount++] = rb;
}
