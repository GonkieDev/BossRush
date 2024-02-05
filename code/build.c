///////////////////////////////////////////////////////////////////////////
//~ [h] base includes
#include "base/base_inc.h"

StaticAssert(sizeof(HMM_Vec3) == sizeof(f32)*3,    "");
StaticAssert(sizeof(HMM_Vec4) == sizeof(f32)*4,    "");
StaticAssert(sizeof(HMM_Quat) == sizeof(HMM_Vec4), "");
StaticAssert(sizeof(HMM_Mat3) == sizeof(f32)*9,    "");
StaticAssert(sizeof(HMM_Mat4) == sizeof(f32)*16,   "");

#define OS_FEATURE_GFX 1
#include "os/os_inc.h"

#define GAME_LOG_FILE 0
#define EZM_LOG_FILE GAME_LOG_FILE
#define FC_LOG_FILE  GAME_LOG_FILE

#include "ezm.h"

#define R_BACKEND_GL 1
#include "render/render_inc.h"

#include "assets/assets_inc.h"
#include "font_provider/font_provider_inc.h"
#include "font_cache/font_cache.h"
#include "camera.h"
#include "sprite/sprite_atlas.h"
#include "draw/draw_core.h"
#include "utils/utils_debug_perf.h"
#include "audio/audio_core.h"
//#include "physics/physics_inc.h"
#include "input.h"

//~ [h] game includes
//#include "game.h"

#include "physics2_5d/physics25d.h"
#include "sprite/sprite_animation.h"
#include "game_entity.h"
#include "bossrush.h"

//~ [c] game includes
//#include "game.c"

#include "bossrush.c"
#include "game_entity.c"
#include "sprite/sprite_animation.c"
#include "physics2_5d/physics25d.c"

///////////////////////////////////////////////////////////////////////////
//~ [c] includes
#include "base/base_inc.c"
#include "os/os_inc.c"
#include "render/render_inc.c"
#include "assets/assets_inc.c"
#include "input.c"
#include "ezm.c"
#include "camera.c"
#include "font_cache/font_cache.c"
#include "font_provider/font_provider_inc.c"
#include "draw/draw_core.c"
#include "utils/utils_debug_perf.c"
#include "sprite/sprite_atlas.c"
//#include "physics/physics_inc.c"