/////////////////////////////////////////////////////////////////////////////////
//~ [h] includes
#include "md.h"
#include "md.c"

#define PrintGenNoteCComment(f) fprintf((f), "// generated by %s:%d\n", __FILENAME__, __LINE__)
#undef MD_PrintGenNoteCComment
#define MD_PrintGenNoteCComment PrintGenNoteCComment

#define UnreferencedParameter(param) ((void)param)

//////////////////////////////////////////////////////////////////////
//~ globals
static MD_Arena *arena = 0;

FILE *error_file = 0;

#include <string.h>
#if defined(_WIN32)
# define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
# define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

//////////////////////////////////////////////////////////////////////
//~ main

int
main(int argc, char **argv)
{
    //- Initial Setup
    arena = MD_ArenaAlloc();
    error_file = stderr;
    
    if (argc < 3)
    {
        MD_CodeLoc loc = {0};
        MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error, "Incorrect usage. Usage example:\n\tshader_to_c_string.exe output_file.glsl input_file.glsl [other input files]\n");
    }
    
    MD_String8 output_filename = MD_S8CString(argv[1]);
    FILE *h = fopen(output_filename.str, "wb");
    if (!h)
    {
        MD_CodeLoc loc = {0};
        MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error, "Could not open output file \"%.*s\"\n", MD_S8VArg(output_filename));
        return 0;
    }
    printf("[Shader to C String]\n");
    printf("Outputting to file \"%.*s\"\n", MD_S8VArg(output_filename));
    
    MD_String8 include_guard;
    {
        MD_String8 separator = {0};
        include_guard = MD_PathChopLastPeriod(output_filename);
        include_guard = MD_S8Stylize(arena, include_guard, MD_IdentifierStyle_UpperCase, separator);
    }
    
    fprintf(h, "#ifndef %.*s_INCLUDE_H\n", MD_S8VArg(include_guard));
    fprintf(h, "#define  %.*s_INCLUDE_H\n\n", MD_S8VArg(include_guard));
    
    for (int i = 2; i < argc; i += 1)
    {
        MD_String8 filename = MD_S8CString(argv[i]);
        printf("\t%.*s\n", MD_S8VArg(filename));
        
        MD_String8 file_contents = MD_LoadEntireFile(arena, filename);
        if(file_contents.str == 0)
        {
            MD_CodeLoc loc = {0};
            MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error, "Could not read file \"%.*s\"", MD_S8VArg(filename));
            continue;
        }
        
        MD_String8 variableName;
        {
            MD_String8 splitter = MD_S8Lit(".");
            variableName = MD_S8Stylize(arena, filename, MD_IdentifierStyle_LowerCase, MD_S8(0, 0));
            for (MD_u32 i = 0; i < variableName.size; ++i)
            {
                MD_u8 *c = &variableName.str[i];
                if (*c == '.')
                    *c = '_';
            }
            variableName = MD_S8Fmt(arena, "%.*s_shader", MD_S8VArg(variableName));
        }
        fprintf(h, "global const u8 *%.*s = ", MD_S8VArg(variableName));
        
        MD_String8 splitter = MD_S8Lit("\n");
        MD_String8List lines = MD_S8Split(arena, file_contents, 1, &splitter);
        for (MD_String8Node *node = lines.first; node != 0; node = node->next)
        {
            MD_String8 line = node->string;
            fprintf(h, "\t\"%.*s\\n\"\n", MD_S8VArg(line));
        }
        
        fprintf(h, "\"\\0\";\n\n");
    }
    
    fprintf(h, "\n#endif // %.*s_INCLUDE_H\n", MD_S8VArg(include_guard));
    
    fclose(h);
}