
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4005 4090 )
#endif // #if defined(_MSC_VER)

#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#if defined(_MSC_VER)
# pragma warning( pop )
#endif // #if defined(_MSC_VER)

//~ @Build_Params
#define ROOT_DIR ".." PATH_SEP ".." PATH_SEP
#define CODE_FOLDER    "code"
#define VENDOR_FOLDER  "vendor"
#define TOOLS_FOLDER   "tools"
#define GAME_MAIN_FILE "build.c"
#define GAME_EXE_NAME  "game"
#define ASSETS_FOLDER  "assets"

#define BIN_DIR PATH_SEP "bin" PATH_SEP
#define BIN_DIR_W32 BIN_DIR "win32" PATH_SEP

#if defined(_WIN32)
# define BIN BIN_DIR_W32
#else
# error "Build for this OS not implemented yet."
#endif

# define DEBUG_STR "debug"
# define RELEASE_STR "release"

//~
#define CL_COMMON        "/nologo /std:c11 /FC"
#define CL_COMMON_LIBS   "user32.lib gdi32.lib shell32.lib ole32.lib Winmm.lib"
#define CL_COMMON_LINKER "/INCREMENTAL:NO " CL_COMMON_LIBS " "

//~
enum
{
    Build_Debug,
    Build_Release,
};
int build_kind = Build_Debug;
Cstr build_str = DEBUG_STR;

//- paths
Cstr root;
Cstr code_dir;
Cstr vendor_dir;
Cstr tools_dir;
Cstr assets_dir;
Cstr ezm_tool_dir;
Cstr ezm_src_dir;
Cstr ezm_bin_dir;
Cstr ezm_convert_dst_dir;
Cstr ezm_convert_dst_dir_debug;
Cstr codegen_dir;
Cstr gl_to_cstr_dir;

Cstr target_dir;
Cstr target_assets_dir;

//~ fwd
void mkdir_recursive(Cstr path);
void mkdir_if_not_exists(Cstr path);
void copy_recursive(Cstr dst, Cstr src, Cstr wildcard);

void LineBreak(void)
{
    //printf("\n");
    fprintf(stderr, "\n");
}

//~
void
setup_paths(void)
{
    path_set_current_dir(ROOT_DIR);
    
    root           = PATH(path_get_current_dir());
    code_dir       = PATH(root, CODE_FOLDER);
    vendor_dir     = PATH(root, VENDOR_FOLDER);
    tools_dir      = PATH(root, TOOLS_FOLDER);
    ezm_tool_dir   = PATH(tools_dir, "ezm");
    ezm_src_dir    = PATH(ezm_tool_dir, "src");
    ezm_bin_dir    = PATH(ezm_tool_dir, "bin", "");
    assets_dir     = PATH(root, ASSETS_FOLDER);
    ezm_convert_dst_dir = PATH(assets_dir, "ezm");
    ezm_convert_dst_dir_debug = PATH(ezm_convert_dst_dir, "debug", "");
    codegen_dir    = PATH(tools_dir, "gl_loader_codegen");
    gl_to_cstr_dir = PATH(tools_dir, "shader_to_c_string");
    
    target_dir     = PATH(root, BIN DEBUG_STR);
    if (build_kind == Build_Release)
        target_dir = PATH(root, BIN RELEASE_STR);
    target_assets_dir = PATH(target_dir, ASSETS_FOLDER);
    
    mkdir_if_not_exists(target_dir);
    mkdir_if_not_exists(target_assets_dir);
}

//~
void
copy_assets(void)
{
    copy_recursive(target_assets_dir, assets_dir, "*");
}

void
build_gl_loader_codegen(void)
{
    INFO("Building GL Loader Codegen");
    
    Cstr codegen_main_file = "gl_func_loader_codegen.c";
    
    Cstr prev_dir = path_get_current_dir();
    path_set_current_dir(codegen_dir);
    
#if defined (_MSC_VER)
    CMD("cl", 
        codegen_main_file,
        CL_COMMON,
        
        "/Fegl_func_loader_codegen.exe",
        CONCAT("/I", PATH(vendor_dir, "metadesk"), " "),
        
        "/O2",
        "/wd5105",
        
        "/link "
        CL_COMMON_LINKER
        );
#else
# error Compiler not supported.
#endif
    
    path_set_current_dir(prev_dir);
    
    LineBreak();
}

//~
void
build_gl_shader_to_c_string(void)
{
    INFO("Building GL Shader to Cstr");
    
    Cstr main_file = "shader_to_c_string.c";
    Cstr prev_dir = path_get_current_dir();
    path_set_current_dir(gl_to_cstr_dir);
    
#if defined (_MSC_VER)
    CMD("cl", 
        main_file,
        CL_COMMON,
        "/Feshader_to_c_string.exe",
        CONCAT("/I", PATH(vendor_dir, "metadesk"), " "),
        "/wd5105",
        "/O2",
        
        "/link "
        CL_COMMON_LINKER
        );
#else
# error Compiler not supported.
#endif
    
    path_set_current_dir(prev_dir);
    
    LineBreak();
}

//~
void
build_ezm_converter(void)
{
    INFO("Building EZM converter.");
    
    Cstr main_file = "ezm_converter.c";
    Cstr prev_dir = path_get_current_dir();
    
    INFO("ezm bin: %s", ezm_bin_dir);
    path_rm(ezm_bin_dir);
    mkdir_recursive(ezm_bin_dir);
    path_set_current_dir(ezm_bin_dir);
    
#if 0
    // debug build
    Cstr extra_compiler_flags = "/Od /Zi /Fd /Fm /Fo";
#else
    // optimised build
    Cstr extra_compiler_flags = "/O2";
#endif
    Cstr warns = "/wd4201 /wd5105 /wd5045 /wd4101 /wd4090 /wd4127 /wd4061 /wd4047 /wd4024";
    
#if defined (_MSC_VER)
    CMD("cl", 
        PATH(ezm_src_dir, main_file),
        CL_COMMON,
        "/Feezm_converter.exe",
        
        CONCAT("/I", code_dir, " "),
        CONCAT("/I", vendor_dir, " "),
        
        extra_compiler_flags,
        warns,
        
        "/link "
        CL_COMMON_LINKER
        
        );
#else
# error Compiler not supported.
#endif
    
    
    path_set_current_dir(prev_dir);
    LineBreak();
}

//~
void
convert_all_meshes(void)
{
    Cstr ezm_files = 0;
    
    FOREACH_FILE_IN_DIR(file, assets_dir, {
                            if (strcmp(file, ".") == 0 || strcmp(file, "..") == 0)
                            {
                                continue;
                            }
                            
                            INFO("Convert file: %s", file);
                            
                            if (ENDS_WITH(file, ".glb") || ENDS_WITH(file, ".gltf"))
                            {
                                ezm_files = CONCAT(PATH(assets_dir, file), " ", ezm_files);
                            }
                        });
    
    Cstr ezmConverterExe = PATH(ezm_bin_dir, "ezm_converter.exe");
    
    if (ezm_files)
    {
        mkdir_if_not_exists(ezm_convert_dst_dir);
        CMD(ezmConverterExe, 
            ezm_files,
            ezm_convert_dst_dir);
    }
    else
    {
        WARN("No EZM files found to convert.");
    }
    
    if (build_kind == Build_Debug)
    {
        Cstr ezm_files_debug_path = PATH(assets_dir, "debug");
        Cstr ezm_files_debug = 0;
        FOREACH_FILE_IN_DIR(file, ezm_files_debug_path, {
                                if (strcmp(file, ".") == 0 || strcmp(file, "..") == 0)
                                {
                                    continue;
                                }
                                
                                INFO("Convert debug file: %s", file);
                                
                                if (ENDS_WITH(file, ".glb") || ENDS_WITH(file, ".gltf"))
                                {
                                    ezm_files_debug = CONCAT(PATH(ezm_files_debug_path, file), " ", ezm_files_debug);
                                }
                            });
        
        if (ezm_files_debug)
        {
            INFO("\n");
            mkdir_if_not_exists(ezm_convert_dst_dir_debug);
            CMD(ezmConverterExe, 
                ezm_files_debug,
                ezm_convert_dst_dir_debug);
        }
        else
        {
            WARN("No EZM Debug files found to convert.");
        }
    }
    
    LineBreak();
}

//~
void
build_game_main(void)
{
    INFO("Building game.");
    
    // gl func loader codegen
    {
        path_set_current_dir(PATH(code_dir, "render", "gl"));
        CMD(PATH(codegen_dir, "gl_func_loader_codegen.exe"), "render_gl_funcs.mdesk");
        LineBreak();
    }
    
    // gl shader to c string
    {
        path_set_current_dir(PATH(code_dir, "render", "gl", "builtin_shaders"));
        
        CMD(PATH(gl_to_cstr_dir, "shader_to_c_string.exe"), 
            "render_gl_generated_shaders.h",
            
            "triangle.vert.glsl",
            "triangle.frag.glsl", 
            
            "ezm_simple.vert.glsl",
            "ezm_simple.frag.glsl",
            
            "ui_shader.vert.glsl",
            "ui_shader.frag.glsl",
            
            "sprite3D_shader.vert.glsl",
            "sprite3D_shader.frag.glsl"
            );
        
        LineBreak();
    }
    
    // ezm converter
    convert_all_meshes();
    
    // pushd
    path_set_current_dir(target_dir);
    
    //- Compile game
    {
        Cstr main_file = PATH(code_dir, GAME_MAIN_FILE);
        
#if defined(_MSC_VER)
        
        Cstr extra_compiler_flags = "";
        Cstr extra_linker_flags = "";
        Cstr warns = "/wd4201 /wd5105 /wd5045 /wd4101 /wd4090 /wd4127 /wd4061 /wd4047 /wd4024 /wd4191 /wd4577";
        
        if (build_kind == Build_Debug)
        {
            extra_compiler_flags = "/Od /Zi /DBUILD_DEBUG ";
            extra_compiler_flags = 
                CONCAT(extra_compiler_flags,
                       //CONCAT("/Fd", GAME_EXE_NAME, ".pdb "),
                       //CONCAT("/Fm", GAME_EXE_NAME, ".map "),
                       //CONCAT("/Fo", GAME_EXE_NAME, ".obj ")
                       "/Fd /Fm /Fo "
                       );
        }
        else if (build_kind == Build_Release)
        {
            extra_compiler_flags = "/O2";
            warns = CONCAT(warns, " /wd4711 ");
        }
        
        CMD("cl", 
            
            main_file,
            
            // Extra CPP files
            PATH(code_dir, "font_provider", "dwrite", "font_provider_core_dwrite.cpp"),
            PATH(code_dir, "audio", "audio_xaudio29.cpp"),
            
            CL_COMMON,
            "/Wall",
            
            CONCAT("/Fe", GAME_EXE_NAME ".exe "),
            CONCAT("/I", code_dir, " "),
            CONCAT("/I", vendor_dir, " "),
            CONCAT("/I", ezm_src_dir, " "),
            extra_compiler_flags,
            warns,
            
            "/link "
            CL_COMMON_LINKER,
            "opengl32.lib",
            extra_linker_flags
            
            );
        
#else
# error "Compiler not supported."
#endif
    }
    
    LineBreak();
    
    copy_assets();
    
    // popd
    path_set_current_dir(root);
}

//~
int
main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);
    
    int build_tools = 1;
    
    if (argc >= 2)
    {
        for (int i = 1; i < argc; ++i)
        {
            char *arg = argv[i];
            if (strcmp(arg, RELEASE_STR) == 0)
            {
                build_kind = Build_Release;
            }
            else if (strcmp(arg, "not_full") == 0)
            {
                build_tools = 0;
            }
        }
    }
    
    if (argc == 2 && (strcmp(argv[1], RELEASE_STR) == 0))
    {
        INFO("Building RELEASE mode.");
        build_kind = Build_Release;
        build_str = RELEASE_STR;
    }
    else
    {
        INFO("Building DEBUG mode.");
    }
    
    setup_paths();
    if (build_tools)
    {
        build_gl_loader_codegen();
        build_gl_shader_to_c_string();
        build_ezm_converter();
    }
    build_game_main();
    
    INFO("Success.");
    
    return 0;
}

//~ Helpers

void
mkdir_recursive(Cstr path)
{
    char tmp[256] = {0};
    
    for (int i = 0;; ++i)
    {
        char c = path[i];
        if (!c)
        {
            break;
        }
        
        if (c == '\\' || c == '/')
        {
            memcpy(tmp, path, i);
            tmp[i+1] = 0;
            
            if (!path_exists(tmp))
            {
                Cstr_Array dir_to_make = cstr_array_make((Cstr)tmp, 0);
                path_mkdirs(dir_to_make);
            }
        }
    }
}

void
mkdir_if_not_exists(Cstr path)
{
    if (!path_exists(path))
    {
        INFO("Making dir: %s", path);
        mkdir_recursive(path);
    }
}

void
copy_recursive(Cstr dst, Cstr src, Cstr wildcard)
{
#if defined(_WIN32)
    // xcopy <src> <dst>
    // xcopy "%assets_dir%"\ezm\*.ezm .\assets /h /i /c /k /e /r /y
    CMD("xcopy", PATH(src, wildcard), dst, "/h /i /c /k /e /r /y");
#else
# error "copy_recursive not implemented for this OS"
#endif
}