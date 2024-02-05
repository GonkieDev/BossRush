#ifndef AUDIO_CORE_H
#define AUDIO_CORE_H

typedef struct AudioHandle AudioHandle; 
struct AudioHandle
{
    u64 a;
};

//////////////////////////////////////////////////////
//~ Defines
#define audio_function no_name_mangle
#define audio_global static

//////////////////////////////////////////////////////
//~ API
audio_function b32 AudioInit(void);

// NOTE(gsp): must be 'wav' format
audio_function AudioHandle AudioFromFile(M_Arena *arena, String8 filepath);

audio_function b32 AudioHandleIsValid(AudioHandle handle);
audio_function void AudioPlay(AudioHandle handle);

////////////////////////////////////////////////////////////////
//~ END OF FILE
#endif //AUDIO_CORE_H
