#ifndef PHYSICS_CORE_INCLUDE_H
#define PHYSICS_CORE_INCLUDE_H

#if 0
typedef struct P_RigidBody P_RigidBody;
struct P_RigidBody
{
    // Constants - given at initialisation of rb
    f32       mass;      // M
    HMM_Mat3 Ibody;      // Inertia Matrix
    HMM_Mat3 IbodyInv;   // Inertia Matrix Inverse
    
    // State variables
    HMM_Vec3   x;          // x(t) - position at time t
    f32 __pad[2];
    HMM_Quat   q;          // q(t) - orientation at time t
    HMM_Vec3   P;          // P(t) - linear mmomentum at time t
    HMM_Vec3   L;          // L(t) - angular mmomentum at time t
    
    // Derived variables (auxilary variables)
    HMM_Vec3   Iinv;       // Inverse of I(t), I^(-1)(t)
    HMM_Vec3   v;          // v(t) - velocity at time t
    HMM_Vec3   omega;      // omega(t) - angular velocity
    
    // Computed quantities
    HMM_Vec3   force;      // F(t)
    HMM_Vec3   torque;     // tau(t)
    HMM_Vec3 __pad2;
};
#endif

typedef struct P_RigidBody P_RigidBody;
struct P_RigidBody
{
    // Initial properties
    HMM_Vec3 aabb;
    f32      mass;
    HMM_Mat3 IbodyInv;     // Inertia Matrix Inverse
    
    HMM_Vec3   x;          // x(t) - position at time t
    HMM_Mat3   R;
    HMM_Vec3   P;          // P(t) - linear mmomentum at time t
    HMM_Vec3   L;          // L(t) - angular mmomentum at time t
};

#define P_MAX_RIGIDBODIES 500

typedef struct P_PhysicsState P_PhysicsState;
struct P_PhysicsState
{
    P_RigidBody rbs[P_MAX_RIGIDBODIES];
    u32 rbsCount;
    PAD32(1);
    
    f64 physTimeAccu; // for fixed step
};

////////////////////////////////////////////////////////////////////////////////////////////
//~
function void P_Update(P_PhysicsState *state, f64 dt);
function void P_FixedStep(P_PhysicsState *state, f64 h);

////////////////////////////////////////////////////////////////////////////////////////////
//~
function void P_RigidBodyAdd(P_PhysicsState *state, HMM_Vec3 aabb, f32 mass, HMM_Vec3 pos);

////////////////////////////////////////////////////////////////////////////////////////////
//~
typedef void (*p_dydt_func)(f32 t, f32 *y, f32 *y_dot, u32 len);
// ordinary differential equation
function void P_ODE(f32 *y0, f32 y_end, u32 len, f32 t0, f32 t1, p_dydt_func dydt);

function void P_ComputeForceAndTorque(P_RigidBody *rbs, u32 count);


/////////////////////////////////////////////////////////
//~ END OF FILE
#endif // PHYSICS_CORE_INCLUDE_H
