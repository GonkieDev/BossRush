
function HMM_Mat3
P_InertiaTensorFromAABBDensity1(f32 x, f32 y, f32 z)
{
    f32 Vover12 = (x*y*z) / 12.f;
    HMM_Mat3 m = {
        .elements[0][0] = (y*y + z*z) * Vover12,
        .elements[1][1] = (x*x + z*z) * Vover12,
        .elements[2][2] = (x*x + y*y) * Vover12,
    };
    return m;
}

////////////////////////////////////////////////////////////////////////////////////////////
//~ P_MeshConstants
function P_MeshConstants
P_MeshConstantsFromMesh(f32 density, u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes)
{
    // http://number-none.com/blow/inertia/index.html
    
    P_MeshConstants consts = {0};
    
    // NOTE(gsp): covariance matrix of a canonical tetrahedron
    HMM_Mat3 C_canonical = {
        .elements[0][0] = 1.f/60.f,
        .elements[1][1] = 1.f/60.f,
        .elements[2][2] = 1.f/60.f,
        
        .elements[0][1] = 1.f/120.f,
        .elements[0][2] = 1.f/120.f,
        .elements[1][0] = 1.f/120.f,
        .elements[1][2] = 1.f/120.f,
        .elements[2][0] = 1.f/120.f,
        .elements[2][1] = 1.f/120.f,
    };
    
    // NOTE(gsp): w0 is the reference point
    HMM_Vec3 w0 = HMM_V3(0.f, 0.f, 0.f);
    
    Assert(indicesCount % 3 == 0);
    for (u32 i = 0; i < indicesCount; i += 3)
    {
        u32 i1 = indices[i+0];
        u32 i2 = indices[i+1];
        u32 i3 = indices[i+2];
        
        HMM_Vec3 w1 = *((HMM_Vec3*) ((u8*)vertices + (i1*vertexStrideInBytes)) );
        HMM_Vec3 w2 = *((HMM_Vec3*) ((u8*)vertices + (i2*vertexStrideInBytes)) );
        HMM_Vec3 w3 = *((HMM_Vec3*) ((u8*)vertices + (i3*vertexStrideInBytes)) );
        
        HMM_Vec3 w10 = HMM_Sub(w1, w0);
        HMM_Vec3 w20 = HMM_Sub(w2, w0);
        HMM_Vec3 w30 = HMM_Sub(w3, w0);
        
        HMM_Mat3 A = {
            .elements[0][0] = w10.x, .elements[0][1] = w20.x, .elements[0][2] = w30.x,
            .elements[1][0] = w10.y, .elements[1][1] = w20.y, .elements[1][2] = w30.y,
            .elements[2][0] = w10.z, .elements[2][1] = w20.z, .elements[2][2] = w30.z,
        };
        
        f32 detA = HMM_Determinant(A);
        HMM_Mat3 A_trans = HMM_Transpose(A);
        
        // Calculate C
        HMM_Mat3 C; 
        C = HMM_Mul(C_canonical, A_trans);
        C = HMM_Mul(A, C);
        C = HMM_Mul(C, detA);
#if 0
        HMM_Mat3 C = HMM_Mul(HMM_Mul(A, HMM_Mul(C_canonical, A_trans)),
                             detA);
#endif
        
        // Calculate other mass properties
        f32 V = detA / 6.f;
        f32 m = density * V;
        HMM_Vec3 cm = w0;
        cm = HMM_Add(cm, w1);
        cm = HMM_Add(cm, w2);
        cm = HMM_Add(cm, w3);
        cm = HMM_Mul(cm, 1.f / 4.f);
        
        // Accumulate
        consts.C  = HMM_Add(consts.C, C);
        consts.cm = HMM_Mul(HMM_Add(HMM_Mul(consts.cm, consts.m), HMM_Mul(cm, m)), 1.f / (consts.m + m));
        consts.m  += m;
        consts.V  += V;
    }
    
    // NOTE(gsp): if this check is removed, we have to translate consts.C
    // w0 -> center of mass
    // C = Translate(C, w0 - cm)
    {
        HMM_Vec3 cm = consts.cm;
        Assert((cm.x == 0.f && cm.y == 0.f && cm.z == 0.f) || 
               (HMM_Len(cm) < 0.00001f));
        cm = HMM_V3(0.f, 0.f, 0.f);
    }
    
    f32 trC = HMM_TraceM3(consts.C);
    HMM_Mat3 I = HMM_M3D(trC);
    I = HMM_Sub(I, consts.C);
    consts.Ibody = I;
    
    return consts;
}

function HMM_Vec3
P_MeshCenterOfMass(u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes)
{
    HMM_Vec3 result = {0};
    for (u32 i = 0; i < indicesCount; ++i)
    {
        HMM_Vec3 p1 = *((HMM_Vec3*) ((u8*)vertices + (indices[i]*vertexStrideInBytes)) );
        result = HMM_Add(result, p1);
    }
    result = HMM_Mul(result, 1.f / (f32)indicesCount);
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////
//~ Volume
function f32
P_VolumeFromEzmMesh(EzmModel *model, EzmMesh *mesh)
{
    return P_VolumeFromMesh(model->indices + mesh->indicesOffset, mesh->indicesCount, &model->vertices[0].p, sizeof(EzmVertex));
}

function f32
P_VolumeFromMesh(u16 *indices, u32 indicesCount, HMM_Vec3 *vertices, u32 vertexStrideInBytes)
{
    f32 result = 0.f;
    Assert(indicesCount % 3 == 0);
    Assert(indices);
    for (u32 i = 0; i < indicesCount; i += 3)
    {
        u32 i1 = indices[i+0];
        u32 i2 = indices[i+1];
        u32 i3 = indices[i+2];
        
        HMM_Vec3 p1 = *((HMM_Vec3*) ((u8*)vertices + (i1*vertexStrideInBytes)) );
        HMM_Vec3 p2 = *((HMM_Vec3*) ((u8*)vertices + (i2*vertexStrideInBytes)) );
        HMM_Vec3 p3 = *((HMM_Vec3*) ((u8*)vertices + (i3*vertexStrideInBytes)) );
        
        result += P_SignedVolumeFromTriangle(p1, p2, p3);
    }
    return AbsoluteValueF32(result);
}

inline_function f32
P_SignedVolumeFromTriangle(HMM_Vec3 p1, HMM_Vec3 p2, HMM_Vec3 p3)
{
    // Volume of tetrahedron
    f32 result = (1.f/6.f) * (HMM_Dot(p1, HMM_Cross(p2, p3)));
    return result;
}