#ifndef PHYSICS_GEOMETRY_INCLUDE_H
#define PHYSICS_GEOMETRY_INCLUDE_H

typedef struct P_MeshConstants P_MeshConstants;
struct P_MeshConstants
{
    HMM_Vec3   cm;         // center of mass
    f32       V;          // volume
    f32       m;          // mass
    HMM_Mat3 C;          // covariance
    HMM_Mat3 Ibody;      // Inertia Matrix
};

//-
function HMM_Mat3 P_InertiaTensorFromAABBDensity1(f32 x, f32 y, f32 z);

//-
// NOTE(gsp): vertexStrideInBytes = sizeof(Vertex)
function P_MeshConstants P_MeshConstantsFromMesh(f32 density, u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes);

//- Center of mass
function HMM_Vec3 P_MeshCenterOfMass(u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes);

//- Volume
function f32 P_VolumeFromEzmMesh(EzmModel *model, EzmMesh *mesh);
function f32 P_VolumeFromMesh(u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes);
function f32 P_SignedVolumeFromTriangle(HMM_Vec3 p1, HMM_Vec3 p2, HMM_Vec3 p3);


/////////////////////////////////////////////////////////
//~ END OF FILE
#endif //PHYSICS_GEOMETRY_INCLUDE_H
