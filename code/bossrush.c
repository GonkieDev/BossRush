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
    
    D_SetPtrs(frameScratch.arena, &g_state->RCmdList);
    
    while (applicationRunning)
    {
        U_BEGIN_TIMED_BLOCK(TOTAL);
        U_BEGIN_TIMED_BLOCK(Input);
        
        f64 currTime = OS_TimeSecondsF64();
        f64 dtF64 = currTime - lastFrameTime;
        lastFrameTime = currTime;
        f32 dt = (f32)dtF64;
        
        g_state->runTime += dt;
        
        MemoryZeroStruct(&g_state->RCmdList);
        
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
                        
                        case OS_Key_G:
                        {
                            g_state->invGrav = !g_state->invGrav;
                        }break;
                        
#if BUILD_DEBUG
                        case OS_Key_O:
                        {
                            Camera *freeCam = &g_state->freeCam;
                            g_state->activeCam = (freeCam == g_state->activeCam) ? &g_state->mainCam : freeCam;
                        }break;
                        case OS_Key_F9:
                        {
                            g_state->showHitBoxes = !g_state->showHitBoxes;
                        }break;
#endif
                        
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
            // NOTE(gsp): frame update
            g_state->frameCount++;
            
            //OS_SetCursor(OS_CursorKind_Arrow);
            
#if BUILD_DEBUG
            if (g_state->activeCam == &g_state->freeCam)
            {
                HMM_Vec2 sensitivity = HMM_V2(40.f, 40.f);
                f32 moveSpd  = 20.f;
                if (input->freeCameraSpdUp)
                    moveSpd *= 2.f;
                HMM_Vec2 rotDelta = HMM_Mul(input->cameraRotate, sensitivity);
                FreeCameraUpdate(&g_state->freeCam, input->move, rotDelta, moveSpd, dt);
                
                input->move = V3Zero();
            }
            else
            {
                f32 fov = g_state->freeCam.fov;
                MemoryCopyStruct(&g_state->freeCam, &g_state->mainCam);
                g_state->freeCam.fov = fov;
                input->move.y = 0.f;
            }
#endif
            
            // cam update
            {
                Camera *mainCam = &g_state->mainCam;
                HMM_Vec3 playerPos = G_EntityGetPos3D(G_PlayerEntityGet());
                *mainCam = CameraMake(HMM_Add(g_state->playerCamOffset, playerPos), playerPos, mainCam->fov);
            }
            
            Camera *cam = g_state->activeCam;
            HMM_Mat4 camMat = HMM_LookAt_RH(cam->pos, HMM_Add(cam->pos, cam->fwd), HMM_V3(0.f, 1.f, 0.f));
            R_CamMatrixUpdate(&camMat);
            
            g_state->p2d_state.g = g_state->invGrav ? HMM_V2(0.f, -9.81f) : HMM_V2(0.f, 0.f);
            f32 physAlpha = P2D_Update(dt);
            
            UnusedVariable(physAlpha); // TODO(gsp): @physAlpha use phys alpha in renderer
            G_UpdateEntities(dt);
            
            D_DebugArrow(HMM_V3(0.f, 5.f, 0.f), HMM_V3(0.f, 0.f, 1.f), HMM_V4(0.f, 1.f, 1.f, 1.f));
            D_DebugArrow(HMM_V3(0.f, 5.f, 0.f), HMM_V3(1.f, 0.f, 0.f), HMM_V4(1.f, 1.f, 1.f, 1.f));
            D_DebugArrow(HMM_V3(0.f, 5.f, 0.f), HMM_V3(0.f, 1.f, 0.f), HMM_V4(1.f, 0.f, 0.f, 1.f));
            
            U_END_TIMED_BLOCK(Update);
            
            //- Render
            U_BEGIN_TIMED_BLOCK(Render);
            
            R_Frame(g_state->mainCam.fov, &g_state->RCmdList)
            {
                // Floor
                D_Mesh(.mesh = g_state->plane, .size = HMM_V3(40.f, 0.f, 40.f));
                
                //D_Mesh(.mesh = g_state->wall, .size = HMM_V3(5.f, 3.5f, 0.f), .pos = HMM_V3(0.f, 0.f, 7.f));
                
                G_RenderEntities();
                
                G_DrawDebugInfo(dt);
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
        M_ArenaSetAutoAlignment(arena, 16);
        if (0 == arena)
        {
            return 0;
        }
        g_state = M_PushStruct(arena, G_State);
        g_state->arena = arena;
    }
    
    //- Window
    g_state->osWindowHandle = osWindowHandle;
    g_state->windowIsFocused = 1;
    
    //- Cams init
    g_state->playerCamOffset = HMM_V3(-40.f, 80.17f, 0.f);
    
    g_state->mainCam = CameraMake(g_state->playerCamOffset, V3Zero(), RadFromDegF32(40.f));
#if BUILD_DEBUG
    g_state->freeCam = CameraMake(HMM_V3(0.f, 0.f, 0.f), V3Zero(), RadFromDegF32(30.f));
#endif
    g_state->activeCam = &g_state->mainCam;
    
    //- Input
    g_state->oldInput = &g_state->inputs[0];
    g_state->input    = &g_state->inputs[1];
    
    // NOTE(gsp): 1 so that 0 is reserved for player
    g_state->entitiesCount = 1;
    
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
    
    //- Physics init
    P2D_Init(g_state->arena, &g_state->p2d_state, G_MAX_ENTITIES);
    
    //- Sprite Atlas init
    g_state->sa_state = SA_Init(g_state->arena, V2S32(2048, 2048));
    
    //- Load assets
    {
        g_state->tango = AudioFromFile(g_state->arena, S8Lit("assets/audio/tango.wav"));
        g_state->water = AudioFromFile(g_state->arena, S8Lit("assets/audio/water.wav"));
        g_state->kahoot_wurls = AudioFromFile(g_state->arena, S8Lit("assets/audio/kahoot_wurls.wav"));
        
        //AudioPlay(g_state->tango);
        //AudioPlay(g_state->water);
        //AudioPlay(g_state->kahoot_wurls);
        
        g_state->planeModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/plane.ezm"));
        g_state->plane = R_EzmMeshAlloc(g_state->planeModel, &g_state->planeModel->meshes[0]);
        
        g_state->cubeModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/cube.ezm"));
        g_state->cube = R_EzmMeshAlloc(g_state->cubeModel, &g_state->cubeModel->meshes[0]);
        
        g_state->uvsphereModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/uvsphere.ezm"));
        g_state->uvsphere = R_EzmMeshAlloc(g_state->uvsphereModel, &g_state->uvsphereModel->meshes[0]);
        
        g_state->filledCircleModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/filled_circle.ezm"));
        g_state->filledCircle = R_EzmMeshAlloc(g_state->filledCircleModel, &g_state->filledCircleModel->meshes[0]);
        
        g_state->wallModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/wall.ezm"));
        g_state->wall = R_EzmMeshAlloc(g_state->wallModel, &g_state->wallModel->meshes[0]);
        
        
#if BUILD_DEBUG
        g_state->arrowModel = EzmModelFromFile(g_state->arena, S8Lit("assets/ezm/debug/arrow.ezm"));
        g_state->arrow      = R_EzmMeshAlloc(g_state->arrowModel, &g_state->arrowModel->meshes[0]);
#endif // #if BUILD_DEBUG
        
        SpriteAnimParams mageAnimDirTreeParams[Dir2D_COUNT] = SAP_DIR_ARRAY(mage_walk);
        g_state->mageAnimDirTree = SpriteAnimTreeDirMakeFromParams(g_state->arena, g_state->sa_state, mageAnimDirTreeParams);
        
        g_state->fireballSpr = SA_SpriteFromFile(g_state->sa_state, S8Lit("assets/sprites/fireball.png"));
    }
    
    //- Entities init
    G_PlayerEntityInit(HMM_V2(0.f, -3.f), &g_state->mageAnimDirTree);
    G_EnemyEntityInit(HMM_V2(0.f, 3.f), &g_state->mageAnimDirTree);
    G_WallEntityAdd(HMM_V2(0.f, 8.f), HMM_V2(5.f, 0.f), 3.f);
    G_WallEntityAdd(HMM_V2(8.f, 0.f), HMM_V2(0.f, 5.f), 3.f);
    G_WallEntityAdd(HMM_V2(0.f, -8.f), HMM_V2(10.f, 0.f), 3.f);
    
    G_ProjectEntityAdd(g_state->fireballSpr, HMM_V2(20.f, 20.f), HMM_V2(-1.f, 0.f), 5.f, 2.f);
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////
//~ @GameShutdown
function void
G_Shutdown(void)
{
    
}

///////////////////////////////////////////////////////////////////////////
//~
function void
G_DrawDebugInfo(f32 dt)
{
#if BUILD_DEBUG
    
    FP_Handle font = g_state->uiFont;
    f32 fontSize   = g_state->uiFontSize;
    HMM_Vec4 color = HMM_V4(1.f, 1.f, 1.f, 1.f);
    
    D_DrawText(font, fontSize, HMM_V2(0.f, 0.f), color, "s/f: %f (FPS: %f)", dt, 1.f/dt);
    
    if (g_state->drawDebugInfo)
    {
        D_DrawText(font, fontSize, HMM_V2(0.f, g_state->uiFontSize + 5.5f), color, "run time: %f", g_state->runTime);
        
        HMM_Vec2 perfCounterDrawPos = HMM_V2(0.f, 30.f);
        for (u32 i = 0; i < U_DebugPerfCounter_Count; ++i)
        {
            U_DebugPerfCounter perfCounter = u_debug_perf_counters_old[i];
            HMM_Vec2 adv = D_DrawText(font, fontSize, perfCounterDrawPos, color, "%.*s: %llu(us) %llu(K cycles)", S8VArg(u_debug_perf_counter_string[i]), perfCounter.time, perfCounter.cycleCount / 1000);
            
            perfCounterDrawPos.y += adv.y + 5.5f;
        }
    }
    
#endif
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
    const char *windowTitle = "Boss Rush"
#if BUILD_DEBUG
    " (DEBUG)"
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
    
    if (!FC_Init(V2S32(2048, 2048)))
    {
        LogFatal(GAME_LOG_FILE, "Failed to initialise font cache.");
        return;
    }
    
    if (!AudioInit())
    {
        LogFatal(GAME_LOG_FILE, "Failed to initialise audio.");
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
