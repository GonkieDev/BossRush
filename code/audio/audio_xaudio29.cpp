//////////////////////////////////////////////////////
//~ [h] includes
#include "audio_xaudio29.hpp"

//////////////////////////////////////////////////////
//~ gsp: Globals
audio_global XAudio29State *xaudio29_state;

//////////////////////////////////////////////////////
//~ gsp: API Hooks

audio_function b32
AudioInit(void)
{
    if (T_IsMainThread() && (xaudio29_state == 0))
    {
        {
            M_Arena *arena = M_ArenaAlloc(KB(256));
            if (!arena)
                return 0;
            xaudio29_state = M_PushStruct(arena, XAudio29State);
            xaudio29_state->arena = arena;
        }
        
        HRESULT hr;
        hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
        if (FAILED(hr))
        {
            return 0;
        }
        
        if ( FAILED(hr = XAudio2Create( &xaudio29_state->pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) )
        {
            return 0;
        }
        
        if ( FAILED(hr = xaudio29_state->pXAudio2->CreateMasteringVoice( &xaudio29_state->pMasterVoice ) ) )
        {
            return 0;
        }
        
    }
    
    return !!(0 != xaudio29_state);
}

audio_function AudioHandle
AudioFromFile(M_Arena *arena, String8 filepath)
{
    AudioHandle handle = XAudioNullHandle();
    
    // NOTE(gsp): used in case we fail to read audio
    u64 arenaInitialPos = M_ArenaPos(arena);
    
    OS_Handle osFileHandle = OS_FileOpen(arena, filepath, OS_FileAccess_Read);
    if (!OS_HandleIsNil(osFileHandle))
    {
        HANDLE hFile = OS_W32_HandleFromOSHandle(osFileHandle);
        Audio *audio = M_PushStruct(arena, Audio);
        if (XAudioPopulateWaveFormatAndBufferFromFile(arena, hFile, &audio->wfx, &audio->buffer))
        {
            HRESULT hr;
            hr = xaudio29_state->pXAudio2->CreateSourceVoice(&audio->sourceVoice, (WAVEFORMATEX*)&audio->wfx);
            if(!FAILED(hr))
            {
                hr = audio->sourceVoice->SubmitSourceBuffer(&audio->buffer);
                if(!FAILED(hr))
                {
                    // NOTE(gsp): success
                    handle = XAudioHandleFromAudio(audio);
                }
            }
        }
    }
    
    if (!AudioHandleIsValid(handle))
    {
        OS_FileClose(osFileHandle);
        M_ArenaPopTo(arena, arenaInitialPos);
    }
    
    return handle;
}

audio_function b32
AudioHandleIsValid(AudioHandle handle)
{
    Audio *audio = XAudioAudioFromHandle(handle);
    return !!(audio != 0);
}

audio_function void
AudioPlay(AudioHandle handle)
{
    if (!AudioHandleIsValid(handle))
    {
        return;
    }
    
    Audio *audio = XAudioAudioFromHandle(handle);
    if (audio->playing)
    {
        // TODO(gsp): restart?
        return;
    }
    
    HRESULT hr = audio->sourceVoice->Start(0);
    if (S_OK == hr)
    {
        LogInfo(0, "Audio is playing");
        audio->playing = 1;
    }
}

//////////////////////////////////////////////////////
//~ Internal
internal_audio_fn AudioHandle 
XAudioNullHandle(void)
{
    AudioHandle handle = {};
    handle.a = 0;
    return handle;
}

internal_audio_fn Audio *
XAudioAudioFromHandle(AudioHandle handle)
{
    return (Audio *)(handle.a);
}

internal_audio_fn AudioHandle
XAudioHandleFromAudio(Audio *audio)
{
    AudioHandle handle = {};
    handle.a = (u64)audio;
    return handle;
}

internal_audio_fn b32
XAudioPopulateWaveFormatAndBufferFromFile(M_Arena *arena, HANDLE hFile, WAVEFORMATEXTENSIBLE *outWfx, XAUDIO2_BUFFER *outBuffer)
{
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return 0;
    }
    
    if(INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ))
    {
        //return HRESULT_FROM_WIN32( GetLastError() );
        return 0;
    }
    
    //-
    HRESULT hr;
    WAVEFORMATEXTENSIBLE wfx = {};
    XAUDIO2_BUFFER buffer = {};
    
    //- Locate 'RIFF' chunk and check file type
    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    
    if (XAudio29FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition) != S_OK)
    {
        return 0;
    }
    
    DWORD filetype;
    hr = XAudioReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
    if (hr != S_OK)
    {
        return 0;
    }
    if (filetype != fourccWAVE)
    {
        return 0;
    }
    
    // Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
    hr = XAudio29FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    if (hr != S_OK)
        return 0;
    hr = XAudioReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
    if (hr != S_OK)
        return 0;
    
    // Locate the 'data' chunk, and read its contents into the buffer
    hr = XAudio29FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    if (hr != S_OK)
        return 0;
    
    BYTE *pDataBuffer = M_PushArray(arena, BYTE, dwChunkSize);
    hr = XAudioReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);
    if (hr != S_OK)
        return 0;
    
    // Fill structs
    buffer.AudioBytes = dwChunkSize;      //size of the audio buffer in bytes
    buffer.pAudioData = pDataBuffer;      //buffer containing audio data
    buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
    
    MemoryCopyStruct(outWfx, &wfx);
    MemoryCopyStruct(outBuffer, &buffer);
    
    return 1;
}

internal_audio_fn HRESULT 
XAudio29FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    
    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;
    
    while (hr == S_OK)
    {
        DWORD dwRead;
        if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        
        if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        
        switch (dwChunkType)
        {
            case fourccRIFF:
            {
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
                {
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                }
            }break;
            
            default:
            {
                if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, (LONG)dwChunkDataSize, NULL, FILE_CURRENT ) )
                {
                    return HRESULT_FROM_WIN32( GetLastError() );            
                }
            }break;
        }
        
        dwOffset += sizeof(DWORD) * 2;
        
        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }
        
        dwOffset += dwChunkDataSize;
        
        if (bytesRead >= dwRIFFDataSize) 
        {
            return S_FALSE;
        }
        
    }
    
    return S_OK;
}

internal_audio_fn HRESULT
XAudioReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, (LONG)bufferoffset, NULL, FILE_BEGIN ) )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    DWORD dwRead;
    if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }
    return hr;
}


//////////////////////////////////////////////////////
//~ [c] includes
#if !OS_WINDOWS
# error "audio_xaudio29.cpp being included but os is not windows!"
#endif

#pragma warning(push)
#pragma warning(disable: 4668 4820 4042 5246 4365 5246 4365 4505 4514)

#include "base/base_inc.c"
#include "os/os_inc.c"

#pragma warning(pop)