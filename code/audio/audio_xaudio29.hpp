#ifndef AUDIO_XAUDIO29_INCLUDE_HPP
#define AUDIO_XAUDIO29_INCLUDE_HPP

//////////////////////////////////////////////////////
//~ [h] includes
#pragma warning(push)
#pragma warning(disable: 4668 4820 4042 5246 4365 5246 4365 4505 4514 4514)

#define BUILD_EXTERNAL_USAGE 1
#include "base/base_inc.h"

#include <windows.h>
#include <xaudio2.h>

#pragma warning(pop)

#include "os/os_inc.h"
#include "audio_core.h"


//////////////////////////////////////////////////////
//~ Defines
#define internal_audio_fn static

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

//////////////////////////////////////////////////////
//~ Types
typedef struct Audio Audio;
struct Audio
{
    OS_Handle osFileHandle;
    IXAudio2SourceVoice  *sourceVoice;
    WAVEFORMATEXTENSIBLE wfx;
    XAUDIO2_BUFFER       buffer;
    b32                  playing;
};

typedef struct XAudio29State XAudio29State;
struct XAudio29State
{
    M_Arena *arena;
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasterVoice;
};

//////////////////////////////////////////////////////
//~ Internal

// Handles
internal_audio_fn AudioHandle XAudioNullHandle(void);
internal_audio_fn Audio *XAudioAudioFromHandle(AudioHandle handle);
internal_audio_fn AudioHandle XAudioHandleFromAudio(Audio *audio);

internal_audio_fn b32 XAudioPopulateWaveFormatAndBufferFromFile(M_Arena *arena, 
                                                                HANDLE hFile, 
                                                                WAVEFORMATEXTENSIBLE *outWfx, 
                                                                XAUDIO2_BUFFER *outBuffer);

internal_audio_fn HRESULT XAudio29FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition);
internal_audio_fn HRESULT XAudioReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);


//////////////////////////////////////////////////////
//~ END OF FILE
#endif //AUDIO_XAUDIO29_INCLUDE_HPP