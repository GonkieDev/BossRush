////////////////////////////////////////
//~ [h] includes

#include "render_gl_types.h"
#include "render_gl.h"

#if OS_WINDOWS
# include "gl_win32/render_gl_win32.h"
#else
# error "GL code for this platform has not been written yet!"
#endif

////////////////////////////////////////
//~ gsp: globals
global R_GL_State *r_gl_state = 0;

////////////////////////////////////////
//~
function b32
R_Init(OS_Handle osWindowHandle)
{
    if (OS_HandleIsNil(osWindowHandle))
        return 0;
    
    if (!R_GL_OS_Init(osWindowHandle))
    {
        return 0;
    }
    
    //- Load functions required to check extensions
    glGetStringi = (GLGETSTRINGI *)R_GL_GetGLFuncAddress("glGetStringi");
    if (!glGetStringi)
    {
        LogFatal(GL_LOG_FILE, "Failed to get glGetStringi address.");
        return 0;
    }
    
    {
        const GLubyte *glVersion = glGetString(GL_VERSION);
        LogInfo(GL_LOG_FILE, "[GL] GL Version: %s", glVersion);
    }
    
    //- @GL_Extensions_Check
    {
        b32 extensionsSupported[ArrayCount(r_gl_extensions)] = {0};
        
        i32 extensionsCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extensionsCount);
        for (i32 i = 0; i < extensionsCount; ++i)
        {
            const GLubyte *extensionStr = glGetStringi(GL_EXTENSIONS, i);
            String8 extension = Str8C(extensionStr);
            
            for (u32 rIdx = 0; rIdx < ArrayCount(r_gl_extensions); ++rIdx)
            {
                if (!extensionsSupported[rIdx] && Str8StrictMatch(extension, r_gl_extensions[rIdx]))
                {
                    LogInfo(GL_LOG_FILE, "[GL] Extension: %.*s", S8VArg(extension));
                    extensionsSupported[rIdx] = 1;
                }
            }
        }
        
        for (u32 rIdx = 0; rIdx < ArrayCount(r_gl_extensions); ++rIdx)
        {
            if (!extensionsSupported[rIdx])
            {
                LogError(GL_LOG_FILE, "OpenGL Extension '%.*s' not supported. Aborting.", S8VArg(r_gl_extensions[rIdx]));
                return 0;
            }
        }
        
    }
    
    //- Init func ptrs
    if (!R_GL_InitFunctionPointers())
    {
        return 0;
    }
    
#if BUILD_DEBUG
    {
        // Check if we obtained a debug context
        int flags; 
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            // Initialize debug output 
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
            glDebugMessageCallback(R_GL_DebugOutputCallback, 0);
            // Disable notifications
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
            
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0,                       
                                 GL_DEBUG_SEVERITY_MEDIUM, -1, "GL Render test error message - ignore this."); 
        }
        else
        {
            AssertMsg(0, "[R,  GL] Unable to obtain a debug context for OpenGL");
            LogError(GL_LOG_FILE, "[R,  GL] Unable to obtain a debug context for OpenGL");
        }
    }
#endif // #if BUILD_DEBUG
    
    // Init state
    {
        M_Arena *arena = M_ArenaAlloc(sizeof(r_gl_state));
        if (0 == arena)
        {
            return 0;
        }
        r_gl_state = M_PushStruct(arena, R_GL_State);
        r_gl_state->arena = arena;
        
        r_gl_state->eqp.os_window = osWindowHandle;
    }
    
    //- Query values
    {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &r_gl_state->gl_maxTextureSize);
        if (r_gl_state->gl_maxTextureSize == 0)
        {
            LogFatal(GL_LOG_FILE, "[GL] Failed to get max texture size;");
            return 0;
        }
        
#if BUILD_DEBUG
        // Get maxLabelLen
        glGetIntegerv(GL_MAX_LABEL_LENGTH, &r_gl_state->maxLabelLen);
        // https://www.khronos.org/opengl/wiki/Debug_Output#Object_names:~:text=which%20will%20be%20no%20less%20than%20256
        Assert(r_gl_state->maxLabelLen >= 256); 
#endif // #if BUILD_DEBUG
        
    }
    
    //- @GL_Enable_Options
    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    //-
    if (!R_GL_InitResources())
    {
        return 0;
    }
    
    return 1;
}

function void          
R_Shutdown(void)
{
    // TODO(gsp): R_Shutdown
}

////////////////////////////////////////
//~ @R_Frame_Hooks
function b32
R_FrameBegin(f32 fov)
{
    // Check if client size has changed
    {
        R_GL_WindowEquip *eqp = &r_gl_state->eqp;
        Vec2S32 clientDims = Vec2S32FromVec(OS_ClientDimsFromWindow(eqp->os_window));
        if ((clientDims.x != eqp->clientDims.x) || (clientDims.y != eqp->clientDims.y))
        {
            glViewport(0, 0, clientDims.x, clientDims.y);
            
            R_GL_ProjViewport projViewport = {
                .proj = HMM_Perspective_RH_NO(fov, (f32)clientDims.x / (f32)clientDims.y, 0.1f, 1000.f),
                .viewport = HMM_V2((f32)clientDims.x, (f32)clientDims.y),
            };
            
            glNamedBufferSubData(r_gl_state->globalUBO.n, 
                                 (void*)OffsetOfMember(R_GL_GlobalUBO, projViewport),
                                 SizeOfMember(R_GL_GlobalUBO, projViewport),
                                 &projViewport);
            
            LogInfo(GL_LOG_FILE, "[GL] Framebuffer size updated: %d %d (fov: %f)", clientDims.x, clientDims.y, fov);
        }
        eqp->clientDims = clientDims;
    }
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    return 1;
}

function void
R_FrameEnd(R_CmdList *cmdList)
{
    M_ArenaTemp scratch = GetScratch(0, 0);
    
    u32 uiVerticesCount = 0;
    u32 uiVerticesMax   = R_MAX_UI_VERTICES;
    R_GL_UIVertex *uiVertices = M_PushArray(scratch.arena, R_GL_UIVertex, uiVerticesMax);
    
    u32 sprite3DVerticesCount = 0;
    u32 sprite3DVerticesMax   = R_MAX_SPRITE3D_VERTICES;
    R_GL_Sprite3DVertex *sprite3DVertices = M_PushArray(scratch.arena, R_GL_Sprite3DVertex, sprite3DVerticesMax);
    
    u32 sprite3DInstancesCount = 0;
    R_GL_Sprite3DInstanceData *sprite3DInstanceData = M_PushArray(scratch.arena, R_GL_Sprite3DInstanceData, R_MAX_SPRITE3D_RECTS);
    
    // NOTE(gsp): 
#if 0
    u32 texturesToBindCount = 0;
    R_GL_Texture **texturesToBind = M_PushArray(scratch.arena, R_GL_Texture *, R_MAX_TEXTURES);
#endif
    
    U_BEGIN_TIMED_BLOCK(Render_3D);
    
    //- Global binds
    // TODO(gsp): @gl funcs for ubo binding base
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, r_gl_state->globalUBO.n);
    R_GL_BindVAO(r_gl_state->nullVAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r_gl_state->texssbo.n);
    
    glEnable(GL_DEPTH_TEST);
    
    //-
    for (R_CmdNode *node = cmdList->first;
         node != 0;
         node = node->next)
    {
        R_Cmd *cmd = node->cmd;
        switch(cmd->kind)
        {
            default: AssertMsg(0, "Invalid R_Cmd kind."); break;
            
            //- Ezm Mesh
            case R_CmdKind_Mesh:
            {
                // TODO(gsp): @gl_render batch render ezm meshes
                R_GL_EzmMesh *mesh = R_GL_EzmMeshFromMeshKey(cmd->meshKey);
                if (mesh)
                {                
                    // Binds
                    R_GL_BindSP(r_gl_state->ezmShaderProgram);
                    
                    // Bind ssbos
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->indexBuffer.n);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->vertexBuffer.n);
                    
                    glUniformMatrix4fv(r_gl_state->ezmShaderProgramModelLoc, 1, GL_FALSE, &cmd->modelMat.elements);
                    glUniform4f(r_gl_state->ezmShaderProgramShadeColorLoc, cmd->color.x, cmd->color.y, cmd->color.z, cmd->color.w);
                    
                    if (R_TexHandleIsValid(cmd->texture))
                    {
                        R_GL_Texture *tex = R_GL_TextureFromTexHandle(cmd->texture);
                        glUniform1ui(r_gl_state->ezmShaderProgramAlbedoLoc, tex->texIndex);
                    }
                    else
                    {
                        glUniform1ui(r_gl_state->ezmShaderProgramAlbedoLoc, r_gl_state->defaultTex->texIndex);
                    }
                    
                    glDrawArrays(GL_TRIANGLES, 0, mesh->indicesCount);
                }
            }break;
            
            case R_CmdKind_UI_Rect2D:
            {
                i32 start = (uiVerticesCount != 0 ? -1 : 0);
                for (i32 corner = start; corner < 5; ++corner)
                {
                    uiVertices[uiVerticesCount++] = R_GL_UIVertexFromCmd(cmd, (corner == -1) ? 0 : (u32)corner);
                    Assert(uiVerticesCount < uiVerticesMax);
                }
            }break;
        }
    }
    
    for (R_CmdNodeSprite3D *node = cmdList->sprite3DFirst;
         node != 0;
         node = node->next)
    {
        R_CmdSprite3D *cmd = node->cmd;
        for (u32 corner = 0; corner < 6; ++corner)
        {
            sprite3DVertices[sprite3DVerticesCount++] = R_GL_Sprite3DVertexFromCmd(cmd, corner);
            Assert(sprite3DVerticesCount < sprite3DVerticesMax);
        }
        R_GL_Sprite3DInstanceData *instanceData = &sprite3DInstanceData[sprite3DInstancesCount++];
        instanceData->modelMat = cmd->modelMat;
    }
    
    //- Render 3D Sprites
    if (sprite3DInstancesCount > 0)
    {
        //glClear(GL_DEPTH_BUFFER_BIT);
        
        // Binds
        R_GL_BindSP(r_gl_state->sprite3DShaderProgram);
        
        // Upload vertices
        glNamedBufferSubData(r_gl_state->sprite3Dvertexssbo.n, 0,
                             sizeof(R_GL_Sprite3DVertex)*sprite3DVerticesCount,
                             sprite3DVertices);
        
        // Upload instance data
        glNamedBufferSubData(r_gl_state->sprite3DinstanceSSBO.n, 0,
                             sizeof(R_GL_Sprite3DInstanceData)*sprite3DInstancesCount,
                             sprite3DInstanceData);
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r_gl_state->sprite3Dvertexssbo.n);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, r_gl_state->sprite3DinstanceSSBO.n);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, sprite3DVerticesCount, sprite3DInstancesCount);
    }
    U_END_TIMED_BLOCK(Render_3D);
    
    U_BEGIN_TIMED_BLOCK(Render_UI);
    //- Render UI Rects
    if (uiVerticesCount > 0)
    {
        glDisable(GL_DEPTH_TEST);
        // Binds
        R_GL_BindSP(r_gl_state->uiShaderProgram);
        
        // Upload vertices
        glNamedBufferSubData(r_gl_state->uissbo.n, 0,
                             sizeof(R_GL_UIVertex)*uiVerticesCount,
                             uiVertices);
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r_gl_state->uissbo.n);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, uiVerticesCount);
    }
    U_END_TIMED_BLOCK(Render_UI);
    
    //- End
    EndScratch(scratch);
    R_GL_OS_FinishFrame(r_gl_state->eqp.os_window);
}

////////////////////////////////////////
//~ @R_Textures_Hooks
function R_TexHandle
R_Texture2DAlloc(Vec2S32 size, R_Texture2DFormat fmt, R_Texture2DSampling sampling, void *initialData)
{
    Assert(r_gl_state->texturesCount < R_MAX_TEXTURES);
    R_TexHandle texHandle = {0};
    
    if (size.x > r_gl_state->gl_maxTextureSize)
    {
        LogWarn(GL_LOG_FILE, "Texture's width (%d) is bigger than max supported texture size. Truncating it to %d.", size.x, r_gl_state->gl_maxTextureSize);
        size.x = r_gl_state->gl_maxTextureSize;
    }
    if (size.y > r_gl_state->gl_maxTextureSize)
    {
        LogWarn(GL_LOG_FILE, "Texture's height (%d) is bigger than max supported texture size. Truncating it to %d.", size.y, r_gl_state->gl_maxTextureSize);
        size.y = r_gl_state->gl_maxTextureSize;
    }
    
    R_GL_Texture *tex = 0;
    u32 glTexId = U32Max;
    glCreateTextures(GL_TEXTURE_2D, 1, &glTexId);
    if (glTexId  == U32Max)
    {
        LogError(GL_LOG_FILE, "[GL] glGenTextures failed.");
        return texHandle;
    }
    
    tex = &r_gl_state->textures[r_gl_state->texturesCount];
    tex->id       = glTexId;
    tex->size     = size;
    tex->fmt      = fmt;
    tex->sampling = sampling;
    tex->levels   = 0;
    tex->texIndex = r_gl_state->texturesCount;
    
    u32 glSampling = R_GL_Sampling(sampling);
    glTextureParameteri(tex->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex->id, GL_TEXTURE_MIN_FILTER, glSampling);
    glTextureParameteri(tex->id, GL_TEXTURE_MAG_FILTER, glSampling);
    
    u32 glSizedFmt = R_GL_SizedFormat(fmt);
    glTextureStorage2D(tex->id, 1, glSizedFmt, size.x, size.y);
    
    tex->handle = glGetTextureHandleARB(tex->id);
    if (0 == tex->handle)
    {
        LogError(GL_LOG_FILE, "[GL] glGetTextureHandleARB returned null handle.");
        return texHandle;
    }
    glMakeTextureHandleResidentARB(tex->handle);
    
    // Add texture to textures ssbo
    glNamedBufferSubData(r_gl_state->texssbo.n, 
                         (void*)(sizeof(u64)*r_gl_state->texturesCount),
                         sizeof(u64), &tex->handle);
    
    if (initialData)
    {
        u32 glDataFmt  = R_GL_DataFormat(fmt);
        glTextureSubImage2D(tex->id, 0, 0, 0, size.x, size.y, glDataFmt, GL_UNSIGNED_BYTE, initialData);
    }
    
    r_gl_state->texturesCount++;
    texHandle = R_GL_TexHandleFromTexture(tex);
    return texHandle;
}

function void
R_Texture2DRelease(R_TexHandle handle)
{
    if (R_TexHandleIsValid(handle))
    {
        R_GL_Texture *tex  = R_GL_TextureFromTexHandle(handle);
        glDeleteTextures(1, &tex->id);
    }
}

function void
R_Texture2DFillRegion(R_TexHandle handle, Rng2S32 region, void *data)
{
    Assert(data);
    Assert(R_TexHandleIsValid(handle));
    if (!(data && R_TexHandleIsValid(handle)))
        return;
    
    R_GL_Texture *tex  = R_GL_TextureFromTexHandle(handle);
    u32 glDataFormat   = R_GL_DataFormat(tex->fmt);
    Vec2S32 regionSize = Dim2S32(region);
    glTextureSubImage2D(tex->id, 0,
                        region.p0.x, region.p0.y,
                        regionSize.x, regionSize.y,
                        glDataFormat, GL_UNSIGNED_BYTE, data);
}

function Vec2S32
R_SizeFromTexture2D(R_TexHandle handle)
{
    R_GL_Texture *tex = R_GL_TextureFromTexHandle(handle);
    if (!tex) return V2S32(0, 0);
    return tex->size;
}

function R_Texture2DFormat 
R_FormatFromTexture2D(R_TexHandle handle)
{
    R_GL_Texture *tex = R_GL_TextureFromTexHandle(handle);
    if (!tex) return R_Texture2DFormat_Null;
    return tex->fmt;
}

function b32
R_TexHandleIsValid(R_TexHandle handle)
{
    return (0 != handle.a);
}

inline_function R_GL_Texture *
R_GL_TextureFromTexHandle(R_TexHandle handle)
{
    R_GL_Texture *tex = (R_GL_Texture *)handle.a;
    return tex;
}

inline_function R_TexHandle
R_GL_TexHandleFromTexture(R_GL_Texture *tex)
{
    R_TexHandle handle = { .a = (u64)tex };
    return handle;
}

inline_function u32
R_GL_Sampling(R_Texture2DSampling sampling)
{
    u32 glSampling = GL_NEAREST;
    switch (sampling)
    {
        default: AssertMsg(0, "Invalid texture sampling.");
        case R_Texture2DSampling_Nearest: glSampling = GL_NEAREST; break;
        case R_Texture2DSampling_Billinear: glSampling = GL_LINEAR; break;
    }
    return glSampling;
}

inline_function u32
R_GL_DataFormat(R_Texture2DFormat fmt)
{
    u32 glFormat = GL_RGBA;
    switch (fmt)
    {
        default: AssertMsg(0, "Invalid texture format.");
        case R_Texture2DFormat_R8_G8_B8: glFormat = GL_RGB; break;
        case R_Texture2DFormat_R8_G8_B8_A8: glFormat = GL_RGBA; break;
        // TODO: other r_gl_formats
    }
    return glFormat;
}

inline_function u32
R_GL_SizedFormat(R_Texture2DFormat fmt)
{
    u32 glFormat = E_GL_SizedFormat_RGBA8;
    switch (fmt)
    {
        default: AssertMsg(0, "Invalid texture format.");
        case R_Texture2DFormat_R8_G8_B8: glFormat = GL_RGB8; break;
        case R_Texture2DFormat_R8_G8_B8_A8: glFormat = GL_RGBA8; break;
        // TODO: other r_gl_formats
    }
    return glFormat;
}


//- gsp: meshes 
function R_MeshKey
R_EzmMeshAlloc(EzmModel *ezmModel, EzmMesh *unusedEzmMesh)
{
    // TODO(gsp): @ezm_render allocate all meshes properly instead
    // TODO(gsp): @alloc handle too many meshes
    Assert(r_gl_state->ezmMeshesCount < R_EZM_MAX_MESHES);
    UnreferencedParameter(unusedEzmMesh);
    
    R_GL_EzmMesh *glEzmMesh = &r_gl_state->ezmMeshes[r_gl_state->ezmMeshesCount++];
    glEzmMesh->indicesCount = ezmModel->indicesCount;
    
    // TODO: use u16 instead
    M_ArenaTemp scratch = GetScratch(0, 0);
    u32 *u32indices = M_PushArray(scratch.arena, u32, ezmModel->indicesCount);
    for (u32 i = 0; i < ezmModel->indicesCount; ++i)
    {
        u32indices[i] = ezmModel->indices[i];
    }
    
    {
        u32 buffers[2] = { 0 };
        glCreateBuffers(ArrayCount(buffers), buffers);
        glEzmMesh->vertexBuffer.n = buffers[0];
        glEzmMesh->indexBuffer.n  = buffers[1];
    }
    
    glNamedBufferStorage(glEzmMesh->vertexBuffer.n, 
                         sizeof(EzmVertex)*ezmModel->verticesCount, 
                         ezmModel->vertices,  GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(glEzmMesh->indexBuffer.n, 
                         sizeof(u32)*ezmModel->indicesCount, 
                         u32indices, GL_DYNAMIC_STORAGE_BIT);
    
    R_GL_NameObject(GL_BUFFER, glEzmMesh->vertexBuffer.n, "(SSBO) Vertex Buffer Ezm Mesh");
    R_GL_NameObject(GL_BUFFER, glEzmMesh->indexBuffer.n,  "(SSBO) Index Buffer  Ezm Mesh");
    
    EndScratch(scratch);
    return R_MeshKeyFromGLEzmMesh(glEzmMesh);
}

inline_function b32
R_MeshKeyIsValid(R_MeshKey meshKey)
{
    return (meshKey.a != 0);
}

inline_function R_GL_EzmMesh *
R_GL_EzmMeshFromMeshKey(R_MeshKey meshKey)
{
    R_GL_EzmMesh *mesh = (R_GL_EzmMesh *)meshKey.a;
    return mesh;
}

inline_function R_MeshKey
R_MeshKeyFromGLEzmMesh(R_GL_EzmMesh *glEzmMesh)
{
    R_MeshKey meshKey = { .a = (u64)glEzmMesh };
    return meshKey;
}

//- gsp: R_Cmd helpers
function void
R_CmdPush(M_Arena *arena, R_CmdList *list, R_Cmd cmd)
{
    R_CmdNode *node = M_PushStruct(arena, R_CmdNode);
    node->cmd = M_PushStruct(arena, R_Cmd);
    MemoryCopy(node->cmd, &cmd, sizeof(R_Cmd));
    QueuePush(list->first, list->last, node);
    ++list->count;
}

function void
R_CmdSprite3DPush(M_Arena *arena, R_CmdList *list, R_CmdSprite3D cmd)
{
    R_CmdNodeSprite3D *nodeAlloc = M_PushStruct(arena, R_CmdNodeSprite3D);
    QueuePush(list->sprite3DFirst, list->sprite3DLast, nodeAlloc);
    
    nodeAlloc->cmd = M_PushStructNoZero(arena, R_CmdSprite3D);
    MemoryCopy(nodeAlloc->cmd, &cmd, sizeof(R_CmdSprite3D));
    ++list->sprite3DCount;
}

//- Camera
function void
R_CamMatrixUpdate(HMM_Mat4 *camMatrix)
{
    glNamedBufferSubData(r_gl_state->globalUBO.n, 
                         (void*)OffsetOfMember(R_GL_GlobalUBO, cam),
                         sizeof(*camMatrix),
                         camMatrix);
}

//~ End of backend hooks


////////////////////////////////////////
//~ @GL_functions
function b32
R_GL_InitResources(void)
{
    //- Ezm Shader
    r_gl_state->ezmShaderProgram = R_GL_SP(&ezmsimple_vert_glsl_shader, &ezmsimple_frag_glsl_shader);
    R_GL_NameObject(GL_PROGRAM, r_gl_state->ezmShaderProgram.n, "Ezm SP");
    if (!r_gl_state->ezmShaderProgram.n)
    {
        LogError(GL_LOG_FILE, "Failed to compile ezm mesh shader program.");
        return 0;
    }
    
    //- Ezm Shader (uniforms)
    R_GL_BindSP(r_gl_state->ezmShaderProgram);
    r_gl_state->ezmShaderProgramModelLoc = glGetUniformLocation(r_gl_state->ezmShaderProgram.n, "model");
    if (r_gl_state->ezmShaderProgramModelLoc == U32Max)
    {
        LogError(GL_LOG_FILE, "Failed to get 'model' uniform location (ezm shader program).");
        return 0;
    }
    
    r_gl_state->ezmShaderProgramShadeColorLoc = glGetUniformLocation(r_gl_state->ezmShaderProgram.n, "shadeColor");
    if (r_gl_state->ezmShaderProgramShadeColorLoc== U32Max)
    {
        LogError(GL_LOG_FILE, "Failed to get 'shadeColor' uniform location (ezm shader program).");
        return 0;
    }
    
    r_gl_state->ezmShaderProgramAlbedoLoc = glGetUniformLocation(r_gl_state->ezmShaderProgram.n, "albedoTexId");
    if (r_gl_state->ezmShaderProgramAlbedoLoc == U32Max)
    {
        LogError(GL_LOG_FILE, "Failed to get 'albedoTexId' uniform location (ezm shader program).");
        //return 0;
    }
    
    //- UI Shader
    r_gl_state->uiShaderProgram = R_GL_SP(&uishader_vert_glsl_shader, &uishader_frag_glsl_shader);
    R_GL_NameObject(GL_PROGRAM, r_gl_state->uiShaderProgram.n, "UI SP");
    if (!r_gl_state->uiShaderProgram.n)
    {
        LogError(GL_LOG_FILE, "Failed to compile UI shader program.");
        return 0;
    }
    
    //- Sprite3D Shader
    r_gl_state->sprite3DShaderProgram = R_GL_SP(&sprite3dshader_vert_glsl_shader, &sprite3dshader_frag_glsl_shader);
    R_GL_NameObject(GL_PROGRAM, r_gl_state->ezmShaderProgram.n, "Sprite3D Shader Program");
    if (!r_gl_state->sprite3DShaderProgram.n)
    {
        LogError(GL_LOG_FILE, "Failed to sprite3D shader program.");
        return 0;
    }
    
    
    //- Buffer creation
    enum
    {
        R_GL_Buffer_GlobalUBO,
        R_GL_Buffer_TexturesSSBO,
        R_GL_Buffer_UI_SSBO,
        R_GL_Buffer_Sprite3D_Vertex_SSBO,
        R_GL_Buffer_Sprite3D_Instance_SSBO,
        R_GL_Buffer_COUNT
    };
    u32 glBuffers[R_GL_Buffer_COUNT];
    glCreateBuffers(R_GL_Buffer_COUNT, glBuffers);
    r_gl_state->globalUBO.n = glBuffers[R_GL_Buffer_GlobalUBO];
    r_gl_state->texssbo.n = glBuffers[R_GL_Buffer_TexturesSSBO];
    r_gl_state->uissbo.n = glBuffers[R_GL_Buffer_UI_SSBO];
    r_gl_state->sprite3Dvertexssbo.n = glBuffers[R_GL_Buffer_Sprite3D_Vertex_SSBO];
    r_gl_state->sprite3DinstanceSSBO.n = glBuffers[R_GL_Buffer_Sprite3D_Instance_SSBO];
    
    //- Global UBO setup
    {
        glNamedBufferStorage(r_gl_state->globalUBO.n, sizeof(R_GL_GlobalUBO), 0, GL_DYNAMIC_STORAGE_BIT);
        R_GL_NameObject(GL_BUFFER, r_gl_state->globalUBO.n, "GlobalUBO");
    }
    
    //- Textures ssbo
    {
        glNamedBufferStorage(r_gl_state->texssbo.n, sizeof(GLuint64)*R_MAX_TEXTURES, 0, GL_DYNAMIC_STORAGE_BIT);
        R_GL_NameObject(GL_BUFFER, r_gl_state->texssbo.n, "(SSBO) Textures bindless handles");
    }
    
    //- UI SSBO
    {
        glNamedBufferStorage(r_gl_state->uissbo.n, sizeof(R_GL_UIVertex)*R_MAX_UI_VERTICES, 0, GL_DYNAMIC_STORAGE_BIT);
        R_GL_NameObject(GL_BUFFER, r_gl_state->uissbo.n, "(SSBO) UI Vertex data");
    }
    
    //- Sprite3D SSBOs
    {
        glNamedBufferStorage(r_gl_state->sprite3Dvertexssbo.n, sizeof(R_GL_Sprite3DVertex)*R_MAX_SPRITE3D_VERTICES, 0, GL_DYNAMIC_STORAGE_BIT);
        R_GL_NameObject(GL_BUFFER, r_gl_state->sprite3Dvertexssbo.n, "(SSBO) Sprite3D Vertex Data");
        
        glNamedBufferStorage(r_gl_state->sprite3DinstanceSSBO.n, sizeof(R_GL_Sprite3DInstanceData)*R_MAX_SPRITE3D_RECTS, 0, GL_DYNAMIC_STORAGE_BIT);
        R_GL_NameObject(GL_BUFFER, r_gl_state->sprite3DinstanceSSBO.n, "(SSBO) Sprite3D Instance Data");
    }
    
    
    //- Null VAO
    {
        glGenVertexArrays(1, &r_gl_state->nullVAO.n);
        R_GL_BindVAO(r_gl_state->nullVAO); // binding just creates it
        R_GL_NameObject(GL_VERTEX_ARRAY, r_gl_state->nullVAO.n, "Null VAO");
        R_GL_UnbindVAO(r_gl_state->nullVAO);
    }
    
    //- Init default textures
    { // UI Default Texture
        u8 pixel[4] = {255,255,255,255};
        R_TexHandle texHandle = R_Texture2DAlloc(V2S32(1, 1), R_Texture2DFormat_R8_G8_B8_A8, R_Texture2DSampling_Nearest, &pixel);
        r_gl_state->defaultTex = R_GL_TextureFromTexHandle(texHandle);
        R_GL_NameTexture(texHandle, "defaultTex");
    }
    
    return 1;
}

//- @Shader_functions
inline_function u32 
R_GL_ShaderCompile(u8 **source, GLenum shaderType)
{
    u32 shader = 0;
    
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar**)source, 0);
    glCompileShader(shader);
    
#if BUILD_DEBUG
    {
        int  success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, ArrayCount(infoLog), 0, infoLog);
            LogError(GL_LOG_FILE, "Failed to compile shader. Error log:\n%s", infoLog);
            //AssertMsg(0, "shader failed to compile.");
        }
    }
#endif
    
    return shader;
}

inline_function GL_SP
R_GL_SP(u8 **vertSource, u8 **fragSource)
{
    u32 shaderProgram = 0;
    
    u32 vertShader = R_GL_ShaderCompile(vertSource, GL_VERTEX_SHADER);
    u32 fragShader = R_GL_ShaderCompile(fragSource, GL_FRAGMENT_SHADER);
    if (vertShader && fragShader)
    {
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertShader);
        glAttachShader(shaderProgram, fragShader);
        glLinkProgram(shaderProgram);
        
#if BUILD_DEBUG
        {
            int  success;
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if(!success)
            {
                char infoLog[512];
                glGetProgramInfoLog(shaderProgram, ArrayCount(infoLog), 0, infoLog);
                LogError(GL_LOG_FILE, "Failed to link shader program. Error log:\n%s", infoLog);
            }
        }
#endif
    }
    
    // NOTE(gsp): passing 0 is valid, 'silently ignore'
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    GL_SP sp = { .n = shaderProgram };
    return sp;
}

////////////////////////////////////////
//~ @R_GL_BoundState

inline_function void
R_GL_BindSP(GL_SP sp)
{
    if (sp.n != r_gl_state->bs.sp.n)
    {
        glUseProgram(sp.n);
        r_gl_state->bs.sp = sp;
    }
}

inline_function void
R_GL_BindVAO(GL_VAO vao)
{
    if (vao.n != r_gl_state->bs.vao.n)
    {
        glBindVertexArray(vao.n);
        r_gl_state->bs.vao = vao;
    }
}

inline_function void
R_GL_UnbindVAO(GL_VAO vao)
{
    if (vao.n == r_gl_state->bs.vao.n)
    {
        glBindVertexArray(0);
        GL_VAO zeroVAO = {0};
        r_gl_state->bs.vao = zeroVAO;
    }
}


////////////////////////////////////////
//~ @GL_Misc
inline_function R_GL_UIVertex
R_GL_UIVertexFromCmd(R_Cmd *cmd, u32 corner)
{
    corner %= 4;
    
    u32 texId = R_TexHandleIsValid(cmd->texture) ? 
        R_GL_TextureFromTexHandle(cmd->texture)->texIndex : r_gl_state->defaultTex->texIndex;
    
    R_GL_UIVertex v = {
        .rect  = cmd->rect,
        .uv    = cmd->uv,
        .color = cmd->color,
        .texId = texId,
    };
    
    return v;
}

inline_function R_GL_Sprite3DVertex
R_GL_Sprite3DVertexFromCmd(R_CmdSprite3D *cmd, u32 corner)
{
    corner %= 4;
    
    u32 texId = R_TexHandleIsValid(cmd->tex) ? 
        R_GL_TextureFromTexHandle(cmd->tex)->texIndex : r_gl_state->defaultTex->texIndex;
    
    R_GL_Sprite3DVertex v = {
        .uv    = cmd->uv,
        .color = cmd->color,
        .texId = texId,
    };
    
    return v;
}


////////////////////////////////////////
//~ @GL_DebugFunctions
#if BUILD_DEBUG
function void APIENTRY
R_GL_DebugOutputCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
                         GLsizei length, const char *message, const void *userParam)
{
    UnreferencedParameter(length); UnreferencedParameter(userParam);
    
#if R_GL_ENABLE_IGNORE_WARNS
    for (u32 idx = 0; idx < ArrayCount(R_GL_IgnoreErrors); ++idx)
    {
        if (R_GL_IgnoreErrors[idx] == id)
            return;
    }
#endif // #if R_GL_ENABLE_IGNORE_WARNS
    
    String8 sourceStr = {0};
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             sourceStr = S8Lit("API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = S8Lit("Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = S8Lit("Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = S8Lit("Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = S8Lit("Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           sourceStr = S8Lit("Other"); break;
    }
    
    LogLevel loglevel = LogLevel_Error;
    String8 severityStr = {0};
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severityStr = S8Lit("HIGH"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = S8Lit("MEDIUM"); loglevel = LogLevel_Warn; break;
        case GL_DEBUG_SEVERITY_LOW:          severityStr = S8Lit("LOW"); loglevel = LogLevel_Info; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = S8Lit("NOTIF"); loglevel = LogLevel_Info; break;
    };
    
    String8 typeStr = {0};
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               typeStr = S8Lit("Error"); loglevel = LogLevel_Error; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = S8Lit("Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = S8Lit("Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = S8Lit("Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = S8Lit("Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = S8Lit("Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = S8Lit("Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = S8Lit("Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = S8Lit("Other"); break;
    };
    
    LogPrint(loglevel, "--- GL Debug Callback (Type: %.*s) (Source: %.*s) (Severity: %.*s) (%d)---\n\n%s", S8VArg(typeStr), S8VArg(sourceStr), S8VArg(severityStr), id, message);
    
    Assert(LogLevel_Error != loglevel);
}

function void 
R_GL_LabelObject(GLenum type, u32 id, String8 label)
{
    AssertMsg(r_gl_state->maxLabelLen <= label.str, "Label string length is too long.");
#if 0
    switch (type)
    {
        case GL_BUFFER: 
        case GL_SHADER: 
        case GL_PROGRAM: 
        case GL_VERTEX_ARRAY: 
        case GL_QUERY: 
        case GL_PROGRAM_PIPELINE: 
        case GL_TRANSFORM_FEEDBACK: 
        case GL_SAMPLER: 
        case GL_TEXTURE: 
        case GL_RENDERBUFFER: 
        case GL_FRAMEBUFFER:
        {
            glObjectLabel(type, id, (u32)label.size, (char*)label.str);
        }break;
    }
#endif
    glObjectLabel(type, id, (u32)label.size, (char*)label.str);
}

//-
#endif // #if BULID_DEBUG

////////////////////////////////////////
//~ gsp: Checks
#if !R_BACKEND_GL
# error "[R] R_BACKEND_GL is not true, but render_gl.c is being included!"
#endif

////////////////////////////////////////
//~ [c] includes

#if OS_WINDOWS
# include "gl_win32/render_gl_win32.c"
#else
# error "GL code for this platform has not been written yet!"
#endif


////////////////////////////////////////
//~ END OF FILE