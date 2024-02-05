/* date = July 9th 2023 2:20 am */

#ifndef OS_CORE_INCLUDE_H
#define OS_CORE_INCLUDE_H

//////////////////////
//~ Types

typedef struct OS_InitReceipt OS_InitReceipt;
struct OS_InitReceipt
{
    u64 a[1];
};

//- gsp: Thread types 
typedef void OS_ThreadFunction(void *params);

//- gsp: File types
typedef struct OS_Handle OS_Handle;
struct OS_Handle
{
    u64 a[1];
};

typedef u32 OS_FileAccessFlags;
enum
{
    OS_FileAccess_Read              = (1 << 1),
    OS_FileAccess_Write             = (1 << 2),
    OS_FileAccess_Execute           = (1 << 3),
    OS_FileAccess_CreateIfNotExists = (1 << 4),
    OS_FileAccess_Overwrite         = (1 << 5),
};

//- gsp: File iteration types 
typedef u32 OS_FileInfoFlags;
enum
{
    OS_FileInfo_IsDir = (1 << 1),
};

typedef struct OS_FileInfo OS_FileInfo;
struct OS_FileInfo
{
    u64 creationTime;
    u64 lastAccessTime;
    u64 lastWriteTime;
    u64 size;
    String8 filename;     // allocated in arena
    OS_FileInfoFlags flags;
    u32 _pad;
};

//- gsp: Shared library types
typedef struct OS_SharedLib OS_SharedLib;
struct OS_SharedLib
{
    u64 a[2];
};

//////////////////////////////////////////////////////
//~ gsp: EntryPoint function 

function void EntryPoint(u32 argc, String8 *argv);

//////////////////////////////////////////////////////
//~ gsp: @per_os General API functions 

function OS_InitReceipt OS_Init(void);
function b32 OS_InitWasSuccessful(OS_InitReceipt receipt);
function void OS_Shutdown(void);

//- gsp: Helpers
inline_function OS_Handle OS_HandleZero(void);
inline_function b32 OS_CompareHandles(OS_Handle a, OS_Handle b);
inline_function b32 OS_HandleIsNil(OS_Handle handle);

//////////////////////////////////////////////////////
//~ gsp: @per_os Memory functions 

function u64 OS_PageSize(void);
function void *OS_Reserve(u64 size);
function void OS_Release(void *ptr, u64 size);
function void OS_Commit(void *ptr, u64 size);
function void OS_Decommit(void *ptr, u64 size);

//////////////////////////////////////////////////////
//~ gsp: @per_os Threads & synchronisation primitives 

//- gsp: Thread control
function u64 OS_ThreadCurrentID(void);
function OS_Handle OS_ThreadStart(void *params, OS_ThreadFunction *func);
// NOTE(gsp): Waits for this thread to finish
function void OS_ThreadJoin(OS_Handle threadHandle);
function void OS_ThreadDetach(OS_Handle threadHandle);

//- gsp: mutexes
function OS_Handle OS_SRWMutexAlloc(void);
function void OS_SRWMutexRelease(OS_Handle mutex);
function void OS_SRWMutexWriterBlockEnter(OS_Handle mutex);
function void OS_SRWMutexWriterBlockLeave(OS_Handle mutex);
function void OS_SRWMutexReaderBlockEnter(OS_Handle mutex);
function void OS_SRWMutexReaderBlockLeave(OS_Handle mutex);
#define OS_SRWMutexWriterBlock(m) DeferLoop(OS_SRWMutexWriterBlockEnter(m), OS_SRWMutexWriterBlockLeave(m))
#define OS_SRWMutexReaderBlock(m) DeferLoop(OS_SRWMutexReaderBlockEnter(m), OS_SRWMutexReaderBlockLeave(m))


//////////////////////////////////////////////////////
//~ gsp: @per_os File functions

function OS_Handle  OS_FileOpen(M_Arena *arena, String8 path, OS_FileAccessFlags flags);
function void       OS_FileClose(OS_Handle fileHandle);
inline_function b32 OS_FileIsValid(OS_Handle fileHandle);
function String8    OS_FileReadEntire(M_Arena *arena, OS_Handle fileHandle);
// NOTE(gsp): returns number of bytes written
function u64        OS_FileWrite(OS_Handle fileHandle, String8 buffer);
// NOTE(gsp): the number returned is not necessarily the epoch time, it's different on windows!
function u64        OS_FileGetLastWriteTime(OS_Handle fileHandle);

//////////////////////////////////////////////////////
//~ gsp: @per_os File search functions 

// NOTE(gsp): returns non-zero on success
// NOTE(gsp): outHandle and outFileInfo get filled by OS_FileIterStart
function b32  OS_FileIterStart(M_Arena *arena, String8 searchDir, OS_Handle *outHandle, OS_FileInfo *outFileInfo);
// NOTE(gsp): returns non-zero if file was found
function b32  OS_FileIterGetNextFile(M_Arena *arena, OS_Handle handle, OS_FileInfo *outFileInfo);
function void OS_FileIterEnd(OS_Handle handle);

//////////////////////////////////////////////////////
//~ gsp: @per_os Time functions

inline_function DateTime OS_DateTimeCurrent(void);
inline_function f64 OS_TimeSecondsF64(void);
inline_function f32 OS_TimeSecondsF32(void);
inline_function u64 OS_TimeMicroseconds(void);
inline_function u64 OS_TimeCycles(void);
inline_function void OS_Sleep(u64 milliseconds);
function void OS_Wait(u64 endTimeMicroseconds);

//////////////////////////////////////////////////////
//~ gsp: @per_os System Info 
// NOTE(gsp): If it does not blink, it must return InfinityF32
function f32 OS_CaretBlinkTimeSeconds(void);
function f32 OS_DoubleClickTimeSeconds(void);
function u32 OS_LogicalProcessorCount(void);

//////////////////////////////////////////////////////
//~ gsp: @per_os Dynamic Libraries 
function OS_SharedLib OS_LibLoad(M_Arena *arena, String8 path);
function VoidFunc    *OS_LibGetProc(OS_SharedLib lib, String8 name);
function void         OS_LibRelease(OS_SharedLib lib);


//////////////////////////////////////////////////////
//~ gsp: @per_os Entropy 
// TODO(gsp): OS_Entropy stuff
//function void OS_GetEntropy(String8 buffer);

//////////////////////////////////////////////////////
//~ gsp: @per_os Command line args 
// TODO(gsp): command line args ^

//////////////////////////////////////////////////////
//~ gsp: @per_os Path stuff 
function String8 OS_GetExeDirPath(void);
function String8 OS_GetExeName(void);
function String8 OS_GetExePathWithName(void);
function String8 OS_GetCurrentWorkingDir(M_Arena *arena);
function b32     OS_SetCurrentWorkingDir(String8 path);
// TODO(gsp): OS_GetCurrentUserHomeDir
function String8 OS_GetCurrentUserHomeDir(void);
function String8 OS_GetCurrentUserTmpPath(void);

//////////////////////////////////////////////////////
//~ END OF FILE
#endif //OS_CORE_INCLUDE_H