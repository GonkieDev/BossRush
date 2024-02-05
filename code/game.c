//~
global G_State *g_state;

///////////////////////////////////////////////////////////////////////////
//~ @GameUpdate
function void
G_Main(void)
{
    b32 applicationRunning = 1;
    M_ArenaTemp frameScratch = GetScratch(0, 0);
    
    f64 lastFrameTime = OS_TimeSecondsF64();
    
    while (applicationRunning)
    {
        U_BEGIN_TIMED_BLOCK(TOTAL);
        U_BEGIN_TIMED_BLOCK(Input);
        
        f64 currTime = OS_TimeSecondsF64();
        f64 dtF64 = currTime - lastFrameTime;
        lastFrameTime = currTime;
        f32 dt = (f32)dtF64;
        
        g_state->runTime += dt;
        
        I_Swap();
        
        //- Events
        OS_EventList eventList = OS_GetEvents(g_state->osWindowHandle, frameScratch.arena);
        for (OS_Event *event = eventList.first; event != 0; event = event->next)
        {
            switch (event->kind)
            {
                case OS_EventKind_WindowClose:
                {
                    applicationRunning = 0;
                    OS_WindowClose(event->window);
                }break;
                
                case OS_EventKind_Press:
                {
                    OS_Key key = event->key;
                    switch (key)
                    {
                        case OS_Key_Esc: { applicationRunning = 0; } break;
                        case OS_Key_F11: OS_WindowToggleFullscreen(g_state->osWindowHandle); break;
                        case OS_Key_Space: 
                        {
                            g_state->clipMouse = !g_state->clipMouse;
                            OS_ToggleCursorVisibility();
                            
                            if (g_state->clipMouse)
                            {
                                HMM_Vec2 client = OS_ClientDimsFromWindow(g_state->osWindowHandle);
                                HMM_Vec2 quarterClient = HMM_DivV2F(client, 4.f);
                                Rng2F32  clipRect = R2F32(quarterClient,
                                                          HMM_Sub(client, quarterClient));
                                OS_MouseClipToWindow(g_state->osWindowHandle, clipRect);
                            }
                            else
                            {
                                OS_MouseUnclip();
                            }
                        }break;
                        
#if BUILD_DEBUG
                        case OS_Key_F3: { g_state->drawDebugInfo = !g_state->drawDebugInfo; } break;
#endif
                    };
                    
                    I_KeyUpdate(key, 1, event->keyWasDown, 0.f);
                    OS_EatEvent(&eventList, event);
                }break;
                case OS_EventKind_Release:
                {
                    OS_Key key = event->key;
                    I_KeyUpdate(key, 0, event->keyWasDown, 0.f);
                    OS_EatEvent(&eventList, event);
                }break;
                
                case OS_EventKind_MouseMove:
                {
                    I_MouseUpdate(event);
                    OS_EatEvent(&eventList, event);
                }break;
                
                case OS_EventKind_WindowLoseFocus:
                {
                    g_state->windowIsFocused = 0;
                }break;
                case OS_EventKind_WindowGainFocus:
                {
                    g_state->windowIsFocused = 1;
                    I_ClearCurrentInput();
                }break;
                
            }
        }
        
        I_ProcessActions();
        
        U_END_TIMED_BLOCK(Input);
        U_BEGIN_TIMED_BLOCK(Update);
        
        Input *input = g_state->input;
        
        if (g_state->windowIsFocused)
        {
            {
                HMM_Vec2 m = OS_MouseFromWindow(g_state->osWindowHandle);
                //LogInfo(0, "Mouse pos %f %f", m.x, m.y);
            }
            
            {
                f32 verSense, horSense;
                verSense = horSense = 40.f;
                f32 moveSpd  = 5.f;
                f32 clampRotY = 85.f;
                
                if (input->spdUp)
                    moveSpd *= 2.f;
                
                Camera *cam = &g_state->mainCam;
                
                cam->rotY -= input->moveCamera.y * verSense * dt;
                cam->rotY  = Clamp(-clampRotY, cam->rotY, clampRotY);
                cam->rotX  += input->moveCamera.x * horSense * dt;
                //LogInfo(0, "Rot x: %f\ty: %f", cam->rotX, cam->rotY);
                
                HMM_Vec3 fwd;
                {
                    f32 pitch = RadFromDegF32(cam->rotY);
                    f32 yaw   = RadFromDegF32(cam->rotX);
                    f32 cosPitch = cosf(pitch);
                    fwd.x = cosf(yaw) * cosPitch;
                    fwd.y = sinf(pitch);
                    fwd.z = sinf(yaw) * cosPitch;
                    fwd   = HMM_Norm(fwd);
                }
                
                HMM_Vec3 up = HMM_V3(0.f, 1.f, 0.f);
                HMM_Vec3 right = HMM_Norm(HMM_Cross(up, fwd));
                
                HMM_Vec3 camDisplacement = {0};
                camDisplacement = HMM_Mul(fwd, input->move.z);
                camDisplacement = HMM_Sub(camDisplacement, HMM_Mul(right, input->move.x));
                camDisplacement.y = 0.f;
                camDisplacement = HMM_Add(camDisplacement, HMM_Mul(up, input->move.y));
                camDisplacement = HMM_Mul(camDisplacement, moveSpd * dt);
                
                //LogInfo(0, "Move: %f %f %f", V3Args(input->move));
                //LogInfo(0, "Cam: rotX: %f\trotY:%f", cam->rotX, cam->rotY);
                
                cam->pos = HMM_Add(cam->pos, camDisplacement);
                
                HMM_Mat4 camMat = HMM_LookAt_RH(cam->pos, HMM_Add(cam->pos, fwd), up);
                R_CamMatrixUpdate(&camMat);
            }
            
            //- Physics
            P_Update(&g_state->physicsState, dtF64);
            
            U_END_TIMED_BLOCK(Update);
            
            //- Render
            U_BEGIN_TIMED_BLOCK(Render);
            
            R_CmdList cmdList = {0};
            R_Frame(&cmdList)
            {
#if 0
                D_Mesh(.mesh = g_state->mvMachine, .texture = g_state->mvMachineTex, .pos = HMM_V3(5.f, 0.f, 0.f));
                D_Mesh(.mesh = g_state->mvMachine, .texture = g_state->mvMachineTex, .pos = HMM_V3(-5.f, 0.f, 0.f));
                D_Mesh(.mesh = g_state->mvMachine, .texture = g_state->mvMachineTex, .pos = HMM_V3(0.f, 0.f, -5.f));
                D_Mesh(.mesh = g_state->mvMachine, .texture = g_state->mvMachineTex, .pos = HMM_V3(0.f, 0.f, 5.f), .color = HMM_V4(1.f, 0.f, 0.f, 1.f));
                
                D_Mesh(.mesh = g_state->mvMachine, .pos.y = 8.f, .size = HMM_V3(0.1f, 0.1f, 0.1f));
                D_Mesh(.mesh = g_state->mvMachine, .pos = HMM_V3(0.f, 8.f, 2.f), .size.x = 0.1f);
#endif
                
                D_Mesh(.mesh = g_state->plane, .size = HMM_V3(40.f, 0.f, 40.f));
                
                P_PhysicsState *p_state = &g_state->physicsState;
                for (u32 i = 0; i < p_state->rbsCount; ++i)
                {
                    P_RigidBody *rb = &p_state->rbs[i];
                    D_Mesh(.mesh = g_state->cube, .pos = rb->x, .size = rb->aabb, .color = HMM_V4(0.1f, 0.5f, 0.1f, 1.f));
                }
                
                D_DebugArrow(HMM_V3(0.f, 0.f, 0.f), HMM_V3(1.f, 0.f, 0.f), HMM_V4(1.f, 0.f, 0.f, 1.f));
                D_DebugArrow(HMM_V3(0.f, 0.f, 0.f), HMM_V3(0.f, 1.f, 0.f), HMM_V4(0.f, 1.f, 0.f, 1.f));
                D_DebugArrow(HMM_V3(0.f, 0.f, 0.f), HMM_V3(0.f, 0.f, 1.f), HMM_V4(0.f, 0.f, 1.f, 1.f));
                
                
                //- Draw Debug info
#if BUILD_DEBUG
                if (g_state->drawDebugInfo)
                {
                    D_DrawTextQ(HMM_V2(0.f, 0.f), "s/f: %f", dt);
                    D_DrawTextQ(HMM_V2(0.f, g_state->uiFontSize + 5.5f), "run time: %f", g_state->runTime);
                    
                    HMM_Vec2 perfCounterDrawPos = HMM_V2(0.f, 30.f);
                    for (u32 i = 0; i < U_DebugPerfCounter_Count; ++i)
                    {
                        U_DebugPerfCounter perfCounter = u_debug_perf_counters_old[i];
                        HMM_Vec2 adv = D_DrawTextQ(perfCounterDrawPos, "%.*s: %llu(us) %llu(K cycles)", S8VArg(u_debug_perf_counter_string[i]), perfCounter.time, perfCounter.cycleCount / 1000);
                        
                        perfCounterDrawPos.y += adv.y + 5.5f;
                    }
                }
#endif
            }
            
            U_END_TIMED_BLOCK(Render);
        }
        
        //-
        ArenaTempEnd(frameScratch);
        U_END_TIMED_BLOCK(TOTAL);
        U_TIME_BLOCK_END_ALL();
    }
}

///////////////////////////////////////////////////////////////////////////
//~ @GameInit
function b32
G_Init(OS_Handle osWindowHandle)
{
    {
        M_Arena *arena = M_ArenaAllocDefault();
        if (0 == arena)
        {
            return 0;
        }
        g_state = M_PushStruct(arena, G_State);
        g_state->arena = arena;
    }
    
    g_state->osWindowHandle = osWindowHandle;
    {
        Camera *mainCam = &g_state->mainCam;
        mainCam->pos = HMM_V3(0.f, 0.f, 0.f);
    }
    
    g_state->windowIsFocused = 1;
    
    g_state->oldInput = &g_state->inputs[0];
    g_state->input    = &g_state->inputs[1];
    
    //-
    {
        g_state->uiFontSize = 12.f;
        g_state->uiFont = FP_LoadFont(S8Lit("assets/fonts/cour.ttf"));
        if (!FP_IsHandleValid(g_state->uiFont))
        {
            LogError(0, "Failed to load ui font.");
            return 0;
        }
    }
    
    //- Load assets
    {
        g_state->planeModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/plane.ezm"));
        g_state->plane = R_EzmMeshAlloc(g_state->planeModel, &g_state->planeModel->meshes[0]);
        
        g_state->cubeModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/cube.ezm"));
        g_state->cube = R_EzmMeshAlloc(g_state->cubeModel, &g_state->cubeModel->meshes[0]);
        LogInfo(0, "Volume of 2x2x2cube: %f", P_VolumeFromEzmMesh(g_state->cubeModel, &g_state->cubeModel->meshes[0]));
        
        // TODO(gsp): remove
        {
            EzmModel *model = g_state->cubeModel;
            EzmMesh  *mesh  = &g_state->cubeModel->meshes[0];
            P_MeshConstants consts = P_MeshConstantsFromMesh(1.f, model->indices + mesh->indicesOffset, mesh->indicesCount, &model->vertices[0].p,
                                                             sizeof(EzmVertex));
            LogInfo(0, "det(I) = %f", HMM_Determinant(HMM_M3D(1.f)));
            HMM_Mat3 I2 = HMM_Add(HMM_M3D(1.f), HMM_M3D(1.f));
            LogInfo(0, "det(I2) = %f", HMM_Determinant(HMM_M3D(2.f)));
        }
        
        g_state->uvsphereModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/uvsphere.ezm"));
        g_state->uvsphere = R_EzmMeshAlloc(g_state->uvsphereModel, &g_state->uvsphereModel->meshes[0]);
        LogInfo(0, "Volume of uvsphere: %f", P_VolumeFromEzmMesh(g_state->uvsphereModel, &g_state->uvsphereModel->meshes[0]));
        
        
        g_state->mvMachineModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/MV_Machine.ezm"));
        g_state->mvMachine = R_EzmMeshAlloc(g_state->mvMachineModel, &g_state->mvMachineModel->meshes[0]);
        
#if BUILD_DEBUG
        g_state->arrowModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/debug/arrow.ezm"));
        g_state->arrow      = R_EzmMeshAlloc(g_state->arrowModel, &g_state->arrowModel->meshes[0]);
#endif // #if BUILD_DEBUG
        
        ScratchArenaScope(scratch, 0, 0)
        {
            A_Image testImage = A_ImageFromFile(scratch.arena, 4, S8Lit("assets/textures/steampunk.jpg"));
            Assert(testImage.channels == 4);
            g_state->mvMachineTex = R_Texture2DAlloc(testImage.size, R_Texture2DFormat_R8_G8_B8_A8, R_Texture2DSampling_Billinear, testImage.data);
            
            Vec2S32 prevSize = testImage.size;
            testImage = A_ImageFromFile(scratch.arena, 4, S8Lit("assets/textures/test_fill.png"));
            R_Texture2DFillRegion(g_state->mvMachineTex, R2S32(V2S32(0, 0), V2S32(prevSize.x / 2, prevSize.y / 2)), testImage.data);
        }
        
        P_RigidBodyAdd(&g_state->physicsState, HMM_V3(2.f, 3.f, 5.f), 1.f, HMM_V3(0.f, 0.f, 0.f));
        P_RigidBodyAdd(&g_state->physicsState, HMM_V3(1.f, 1.5f, 1.f), 1.f, HMM_V3(5.f, 0.f, 5.f));
    }
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////
//~ @GameShutdown
function void
G_Shutdown(void)
{
    
}

///////////////////////////////////////////////////////////////////////////
//~ @EntryPoint
function void
EntryPoint(u32 argc, String8 *argv)
{
    UnreferencedParameter(argc);
    UnreferencedParameter(argv);
    
    //- Layers init
    T_InitReceipt     threadReceipt    = T_InitMainThreadContext();
    OS_InitReceipt    osInitReceipt    = OS_Init();
    OS_InitGfxReceipt osInitGfxReceipt = OS_InitGfx(osInitReceipt);
    
    //- Create window
    const char *windowTitle = "Game"
#if BUILD_DEBUG
    "(DEBUG)"
#endif
    ;
    OS_Handle osWindowHandle = OS_WindowCreate(V2S64(1280, 720), Str8C(windowTitle), 0);
    if (OS_HandleIsNil(osWindowHandle))
    { 
        LogError(GAME_LOG_FILE, "Failed to create main window."); 
        return; 
    }
    if (!R_Init(osWindowHandle))
    {
        LogError(GAME_LOG_FILE, "Failed to initialise render."); 
        return;
    }
    
    //- Secondary layers init
    if (!FP_Init())
    {
        LogFatal(GAME_LOG_FILE, "Failed to initialise font provider.");
        return;
    }
    
    if (!FC_Init(V2S32(2000, 2000)))
    {
        LogFatal(GAME_LOG_FILE, "Failed to initialise font cache.");
        return;
    }
    
    
    //- Game
    OS_WindowShow(osWindowHandle);
    
    if (!G_Init(osWindowHandle))
    {
        LogFatal(GAME_LOG_FILE, "Game failed to initialised.");
        return;
    }
    G_Main();
    G_Shutdown();
    
    //- Cleaup
    FC_Shutdown();
    R_Shutdown();
}
