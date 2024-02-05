/* date = July 9th 2023 5:56 pm */

#ifndef OS_CORE_WIN32_INCLUDE_H
#define OS_CORE_WIN32_INCLUDE_H

//////////////////////
//~ gsp: Pre windows header 
#pragma push_macro("function")
#undef function

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
# include <timeapi.h> // timeBeginPeriod
# include <shellapi.h> // DragQueryFileW & DragQueryPoint

//- gsp: Post windows headers cleanup 
//#undef WIN32_LEAN_AND_MEAN
#pragma pop_macro("function")

//////////////////////
//~ End of windows headers

/////////////////////
//~ gsp: Types

//- gsp: Thread types 
typedef struct OS_W32_Thread OS_W32_Thread;
struct OS_W32_Thread
{
    OS_W32_Thread *next;
    HANDLE handle;
    DWORD threadID;
    u32 _pad;
    OS_ThreadFunction *func;
    void *params;
};

//- gsp: Synchronisation types
typedef struct OS_W32_SRWLockNode OS_W32_SRWLockNode;
struct OS_W32_SRWLockNode
{
    OS_W32_SRWLockNode *next;
    SRWLOCK lock;
};

//- 
typedef struct OS_W32_State OS_W32_State;
struct OS_W32_State
{
    // arena
    M_Arena *arena;
    
    // threads
    SRWLOCK threadSRWLock;
    M_Arena *threadArena;
    OS_W32_Thread *firstFreeThread;
    
    // mutexes
    M_Arena *mutexArena;
    SRWLOCK mutexesSRWLock;
    OS_W32_SRWLockNode *firstFreeMutex;
    
    SRWLOCK currentDirSRWLock;
    
    // info - gsp: initialised and kept constant after OS_Init
    LARGE_INTEGER counterFrequency;
    b32 granularSleepEnabled;
    u32 logicalProcessorCount;
    String8 exePathWithName;
    String8 exeDirPath;
    String8 exeName;
    String8 currentUserHomeDir;
    String8 currentUserTmpPath;
};

typedef UINT OS_W32_GetDpiForWindowType(HWND hwnd);

/////////////////////
//~ gsp: Functions

#define OS_OutputDebugString(u8str) OutputDebugStringA((LPSTR)u8str)

//- gsp: Helpers 
inline_function void OS_W32_InitReceiptSetValid(OS_InitReceipt *osInitReceipt);

#define OS_W32_HandleFromOSHandle(osHandle) ((HANDLE)osHandle.a[0])
#define OS_W32_ThreadPtrFromOSHandle(osHandle) ((OS_W32_Thread *)osHandle.a[0])
#define OS_W32_SRWLockNodeFromOSHandle(osHandle) ((OS_W32_SRWLockNode*)osHandle.a[0])

#define OS_W32_HModuleFromOSSharedLib(osSharedLib) ((HMODULE)osSharedLib.a[0])
inline_function b32 OS_W32_IsValidHModule(HMODULE hmodule);

//- gsp: Thread functions 
function DWORD WINAPI OS_W32_ThreadEntry(LPVOID lpParameter);
function OS_W32_Thread *OS_W32_ThreadAlloc(void);
function void OS_W32_ThreadRelease(OS_W32_Thread *thread);

//- gsp: File functions 
function b32 OS_W32_FileExists(String16 path);

//- gsp: File search functions 
function void OS_W32_FillFileInfo(M_Arena *arena, WIN32_FIND_DATAW *ffd, OS_FileInfo *outFileInfo);

//////////////////////
//~ End of file
#endif //OS_CORE_WIN32_INCLUDE_H
