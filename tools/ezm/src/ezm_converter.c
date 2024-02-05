///////////////////////////////////////////////////////////////////////////
//~ [h] includes
#include <stdio.h>

// NOTE: decent example for cgltf is: https://github.com/raysan5/raylib/blob/master/src/rmodels.c
// line: 4842 (`LoadGLTF`)
#define CGLTF_VALIDATE_ENABLE_ASSERTS 1
#pragma warning( push )
#pragma warning( disable : 4820 4996 4193 )
# define CGLTF_IMPLEMENTATION
# include "cgltf.h"
#pragma warning( pop )

#define LOG_PRINT_RELEASE      1
#define BASE_LOGGER_USE_PRINTF 1
#include "base/base_inc.h"
#define OS_FEATURE_GET_CMD_LINE 1
#include "os/os_inc.h"

#define EZM_WRITE_IMPLEMENTATION
#include "ezm.h"

// Taken and modified from: https://github.com/raysan5/raylib/blob/master/src/rmodels.c
// line: 4870 (originally called `LOAD_ATTRIBUTE`)

// @acessor:           cgltf_accessor ptr
// @componentCount:    number of components of attribute (e.g. vec3 has 3 components)
// @dataType:          destination data type
// @dstPtr:            destination pointer to write to
// @dstStrideType:     essentially destination stride but you don't need to 
//                         pass 'sizeof(type)' just type
// @dstOffsetInBytes:  offset of struct member so that we can write to that struct member
#define CGLTF_LOAD_ATTRIBUTE(accessor, componentCount, dataType, dstPtr, dstStrideType, dstOffsetInBytes) \
{ \
i32 n = 0; \
dataType *buffer =                                      \
(dataType *)((accessor)->buffer_view->buffer->data) +    \
(accessor)->buffer_view->offset/sizeof(dataType)    +    \
(accessor)->offset/sizeof(dataType);                     \
\
u32 dstStride = sizeof(dstStrideType) / sizeof(dataType); \
u32 dstOffset = (dstOffsetInBytes) / sizeof(dataType); \
for (u32 k = 0; k < (accessor)->count; k++) \
{\
for (i32 l = 0; l < (componentCount); l++) \
{\
(dstPtr)[dstStride*k + dstOffset + l] = buffer[n + l];  \
}\
n += (i32)( ((accessor)->stride)/sizeof(dataType) );   \
}\
}

typedef struct EzmFromGLTFResult EzmFromGLTFResult;
struct EzmFromGLTFResult
{
    EzmModel *models;
    String8  *names;
    u32 modelsCount;
    PAD32(1);
};

// @GLTF_to_EZM_Restrictions:
// - Only support primitives: triangle (aka GL_TRIANGLE)
// - Position & Normal attributes must be HMM_Vec3
// - UV Coordinates attributes must be HMM_Vec2
// - Indices are always converted to u16 but we allow u8, u16 and u32 as inputs
// - Each primitive is loaded as a separate EzmMesh
// - If a gltf mesh inside the file is invalid, then we still try get the other meshes
// - If a gltf primitive is invalid, the whole mesh is invalid
function EzmFromGLTFResult
EzmModelsFromGLTF(M_Arena *arena, String8 gltfFilename)
{
    EzmFromGLTFResult result = {0};
    
    LogInfo(EZM_LOG_FILE, "EzmModel from gltf file '%.*s'", S8VArg(gltfFilename));
    //- Reading file and checks
    cgltf_options options = {0};
    cgltf_data*   data = NULL;
    cgltf_result  cgltfResult = cgltf_parse_file(&options, (char*)gltfFilename.str, &data);
    if (cgltf_result_success != cgltfResult)
    {
        LogError(EZM_LOG_FILE, "Unable to read file '%.*s'", S8VArg(gltfFilename));
        return result;
    }
    
    // NOTE: Forces reading data buffers
    cgltfResult = cgltf_load_buffers(&options, data, (char*)gltfFilename.str);
    if (cgltf_result_success != cgltfResult)
    {
        LogError(EZM_LOG_FILE, "Unable to read data buffers from file '%.*s' - probably corrupt", S8VArg(gltfFilename));
        return result;
    }
    
    // NOTE: validates a lot of data (some stuff is still allowed to be 0)
    // importantly, one that we care about is whether attributes have the same count or not,
    // this function checks for that
    cgltfResult = cgltf_validate(data);
    if (cgltf_result_success != cgltfResult)
    {
        LogError(EZM_LOG_FILE, "Data in file '%.*s' does not adhere to gltf spec.", S8VArg(gltfFilename));
        return result;
    }
    
    //- Our own checks and getting counts
    // NOTE: how gltf maps to Ezm
    //       - gltf meshes     = EzmModel s
    //       - gltf primitives = EzmMesh s
    
    if (0 == data->meshes_count)
    {
        LogError(EZM_LOG_FILE, "File '%.*s' does not contain any models.", S8VArg(gltfFilename));
        return result;
    }
    
    M_ArenaTemp scratch = GetScratch(&arena, 1);
    
    // NOTE: if the pointer is 0, then that model was invalid
    // if it is not 0, the model is valid we allocate a temporary
    // 'EzmModel' to it, just with count and name attributes filled in
    EzmModel **validModelPtrs = M_PushArray(scratch.arena, EzmModel *, data->meshes_count);
    u32 validModelsCount = 0;
    for (u32 modelIdx = 0; modelIdx < data->meshes_count; ++modelIdx) 
    {
        b32 meshIsValid = 1;
        cgltf_mesh *gltfMesh = &data->meshes[modelIdx];
        
        u32 totalMeshes   = (u32)gltfMesh->primitives_count;
        u32 totalIndices  = 0;
        u32 totalVertices = 0;
        for (u32 primIdx = 0; primIdx < gltfMesh->primitives_count; ++primIdx)
        {
            cgltf_primitive *primitive = &gltfMesh->primitives[primIdx];
            if (primitive->indices)
            {
                // NOTE: we don't need to check indices type, cgltf_validate does it for us
                totalIndices += (u32)(primitive->indices->count);
            }
            
            // Check if all attributes adhere to out format
            // NOTE: cgltf_validate already does this, but we doing it again
            u32 firstAttributeCount = (u32)primitive->attributes[0].data->count;
            totalVertices += firstAttributeCount;
            for (u32 attribIdx = 0; attribIdx < primitive->attributes_count; ++attribIdx)
            {
                // vertexAttribType might be a position/normal/texture coord/etc..
                cgltf_attribute_type vertexAttribType = primitive->attributes[attribIdx].type;
                
                cgltf_accessor *attribute = primitive->attributes[attribIdx].data;
                
                if (attribute->count != firstAttributeCount)
                {
                    LogError(EZM_LOG_FILE, "Mesh's (in model '%s', file '%.*s') attribute (%s) contains a different ammount of data from other attributes.", gltfMesh->name, S8VArg(gltfFilename), attribute->name);
                    meshIsValid = 0;
                    break;
                }
                
                // attribDataType is i8, u8, u16, u32, f32, etc..
                cgltf_component_type attribDataType = attribute->component_type; 
                
                // attribMathType is scalar, vec2, vec3, vec4, mat4, etc..
                cgltf_type attribMathType = attribute->type;
                
                switch(vertexAttribType)
                {
                    default: break;
                    case cgltf_attribute_type_position:
                    {
                        if (!((attribMathType == cgltf_type_vec3) && (attribDataType == cgltf_component_type_r_32f)))
                        {
                            meshIsValid = 0;
                            LogError(EZM_LOG_FILE, "Mesh's (in model '%s', file '%.*s') position data does not conform to data requirements - it is not HMM_Vec3, we only support this type.", gltfMesh->name, S8VArg(gltfFilename));
                        }
                    }break;
                    case cgltf_attribute_type_normal:
                    {
                        if (!((attribMathType == cgltf_type_vec3) && (attribDataType == cgltf_component_type_r_32f)))
                        {
                            meshIsValid = 0;
                            LogError(EZM_LOG_FILE, "Mesh's (in model '%s', file '%.*s') normal data does not conform to data requirements - it is not HMM_Vec3, we only support this type.", gltfMesh->name, S8VArg(gltfFilename));
                        }
                    }break;
                    case cgltf_attribute_type_texcoord:
                    {
                        if (!((attribMathType == cgltf_type_vec2) && (attribDataType == cgltf_component_type_r_32f)))
                        {
                            meshIsValid = 0;
                            LogError(EZM_LOG_FILE, "Mesh's (in model '%s', file '%.*s') texture coordinate data does not conform to data requirements - it is not HMM_Vec2, we only support this type.", gltfMesh->name, S8VArg(gltfFilename));
                        }
                    }break;
                };
                
                if (!meshIsValid) break;
            }
        }
        
        // Zero checks
        if (meshIsValid && 0 == totalMeshes)
        {
            meshIsValid = 0;
            LogError(EZM_LOG_FILE, "Model '%s' has 0 meshes ('%.*s')", gltfMesh->name, S8VArg(gltfFilename));
        }
        if (meshIsValid && 0 == totalVertices)
        {
            meshIsValid = 0;
            LogError(EZM_LOG_FILE, "Model '%s' has 0 vertices ('%.*s')", gltfMesh->name, S8VArg(gltfFilename));
        }
        
        // U16 Checks
        if (meshIsValid && (totalMeshes > U16Max))
        {
            meshIsValid = 0;
            LogError(EZM_LOG_FILE, "Model '%s' has too many meshes (%d > U16Max) ('%.*s')", gltfMesh->name, totalVertices, S8VArg(gltfFilename));
        }
        if (meshIsValid && (totalVertices > U16Max))
        {
            meshIsValid = 0;
            LogError(EZM_LOG_FILE, "Model '%s' has too many vertices (%d > U16Max) ('%.*s')", gltfMesh->name, totalVertices, S8VArg(gltfFilename));
        }
        if (meshIsValid && totalIndices > U16Max)
        {
            meshIsValid = 0;
            LogError(EZM_LOG_FILE, "Model '%s' contains too many indices (%d > U16Max) ('%.*s')", gltfMesh->name, totalIndices, S8VArg(gltfFilename));
        }
        
        if (meshIsValid)
        {
            validModelsCount++;
            
            EzmModel *model = M_PushStruct(arena, EzmModel);
            validModelPtrs[modelIdx] = model;
            
            model->meshesCount   = (u16)totalMeshes;
            model->verticesCount = (u16)totalVertices;
            model->indicesCount  = (u16)totalIndices;
            
            model->meshes   = M_PushArray(arena, EzmMesh, model->meshesCount);
            model->vertices = M_PushArray(arena, EzmVertex, model->verticesCount);
            model->indices  = M_PushArray(arena, u16, model->indicesCount);
        }
        else
        {
            LogError(EZM_LOG_FILE, "\tWill skip this mesh.");
        }
    }
    
    result.modelsCount = validModelsCount;
    result.models = M_PushArray(arena, EzmModel, result.modelsCount);
    result.names = M_PushArray(arena, String8, result.modelsCount);
    
    //- Read and copy data from buffers
    u32 writtenModelsCount = 0;
    for (u32 modelIdx = 0; modelIdx < data->meshes_count; ++modelIdx) 
    {
        EzmModel *model = &result.models[writtenModelsCount];
        {
            EzmModel *inModel = validModelPtrs[modelIdx];
            if (inModel == 0) continue;
            MemoryCopyStruct(model, inModel);
            
            cgltf_mesh *gltfMesh = &data->meshes[modelIdx];
            result.names[writtenModelsCount] = PushStr8Copy(arena, Str8C(gltfMesh->name));
            
            writtenModelsCount++;
        }
        
        u32 writtenVertices = 0;
        u32 writtenIndices  = 0;
        
        cgltf_mesh *gltfMesh = &data->meshes[modelIdx];
        for (u32 primIdx = 0; primIdx < gltfMesh->primitives_count; ++primIdx)
        {
            cgltf_primitive *primitive = &gltfMesh->primitives[primIdx];
            EzmMesh *mesh = &model->meshes[primIdx];
            
            //- Load Indices
            cgltf_accessor *indicesAttribute = primitive->indices;
            if (indicesAttribute)
            {
                u16 *indicesPtr = model->indices + writtenIndices;
                switch (indicesAttribute->component_type)
                {
                    default: Assert(0); break;
                    case cgltf_component_type_r_8u: 
                    {
                        u8 *tempIndicesBuffer = M_PushArray(scratch.arena, u8, indicesAttribute->count);
                        CGLTF_LOAD_ATTRIBUTE(indicesAttribute, 1, u8, tempIndicesBuffer, u8, 0);
                        for (u32 i = 0; i < indicesAttribute->count; ++i) { indicesPtr[i] = (u16)tempIndicesBuffer[i]; }
                    }
                    case cgltf_component_type_r_32u:
                    {
                        u32 *tempIndicesBuffer = M_PushArray(scratch.arena, u32, indicesAttribute->count);
                        CGLTF_LOAD_ATTRIBUTE(indicesAttribute, 1, u32, tempIndicesBuffer, u32, 0);
                        for (u32 i = 0; i < indicesAttribute->count; ++i) { indicesPtr[i] = (u16)tempIndicesBuffer[i]; }
                    }break;
                    case cgltf_component_type_r_16u: 
                    {
                        CGLTF_LOAD_ATTRIBUTE(indicesAttribute, 1, u16, indicesPtr, u16, 0);
                    }break;
                };
                
                mesh->indicesOffset = writtenIndices;
                mesh->indicesCount  = (u32)indicesAttribute->count;
                writtenIndices     += mesh->indicesCount;
            }
            
            //- Load vertex data
            for (u32 attribIdx = 0; attribIdx < primitive->attributes_count; ++attribIdx)
            {
                // vertexAttribType might be a position/normal/texture coord/etc..
                cgltf_attribute_type vertexAttribType = primitive->attributes[attribIdx].type;
                cgltf_accessor *attribute = primitive->attributes[attribIdx].data;
                
                EzmVertex *verticesPtr = model->vertices + writtenVertices;
                
                switch(vertexAttribType)
                {
                    default: break;
                    case cgltf_attribute_type_position: { CGLTF_LOAD_ATTRIBUTE(attribute, 3, f32, (f32*)verticesPtr, EzmVertex, OffsetOfMember(EzmVertex, p)); }break;
                    case cgltf_attribute_type_normal:
                    { CGLTF_LOAD_ATTRIBUTE(attribute, 3, f32, (f32*)verticesPtr, EzmVertex, OffsetOfMember(EzmVertex, n)); }break;
                    case cgltf_attribute_type_texcoord:
                    { CGLTF_LOAD_ATTRIBUTE(attribute, 2, f32, (f32*)verticesPtr, EzmVertex, OffsetOfMember(EzmVertex, uv)); }break;
                };
            }
            
            mesh->verticesOffset = writtenVertices;
            mesh->verticesCount  = (u32)primitive->attributes[0].data->count;
            writtenVertices     += mesh->verticesCount;
        }
        
        LogInfo(EZM_LOG_FILE, "\tSuccessful read of model '%s' (%.*s)", gltfMesh->name, S8VArg(gltfFilename));
    }
    
    EndScratch(scratch);
    
    return result;
}

///////////////////////////////////////////////////////////////////////////
//~ main
function void
EntryPoint(u32 argc, String8 *argv)
{
    //- Layers init
    T_InitReceipt     threadReceipt    = T_InitMainThreadContext();
    OS_InitReceipt    osInitReceipt    = OS_Init();
    
    if (argc < 2)
    {
        LogError(EZM_LOG_FILE, "Incorrect usage. No input files specified.\n\tExample usage: ezm_converter.exe file1.glb file2.glb ./ezm_files_output_path/");
        return;
    }
    
    String8 outpath = argv[argc-1];
    LogInfo(EZM_LOG_FILE, "Output path: %.*s", S8VArg(outpath));
    
    for (u32 i = 0; i < argc-1; ++i)
    {
        M_Arena *modelsArena = M_ArenaAllocDefault();
        String8 gltfFilename = argv[i];
        EzmFromGLTFResult ezmFromGltfResult = EzmModelsFromGLTF(modelsArena, gltfFilename);
        
        if (ezmFromGltfResult.modelsCount == 0)
        {
            continue;
        }
        
        // Each model will go to its own file
        for (u32 modelIdx = 0; modelIdx < ezmFromGltfResult.modelsCount; ++modelIdx)
        {
            EzmModel *model = &ezmFromGltfResult.models[modelIdx];
            EzmWriteModelToFile(model, outpath, ezmFromGltfResult.names[modelIdx]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//~ [c] includes
#include "base/base_inc.c"
#include "os/os_inc.c"
#include "ezm.c"