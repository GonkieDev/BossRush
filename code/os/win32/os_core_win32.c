#ifndef OS_CORE_WIN32_INCLUDE_C
#define OS_CORE_WIN32_INCLUDE_C

//////////////////////
//~ gsp: Globals
global HINSTANCE os_w32_hInstance = 0; // Init by wWinMain
global OS_W32_State *os_w32_state = 0;

//////////////////////
//~ gsp: General API functions 

function OS_InitReceipt
OS_Init(void)
{
 OS_InitReceipt receipt = {0};
 
 AssertMsg(0 == os_w32_state, "OS_Init should only be called once!");
 if (0 == os_w32_state)
 {
  M_Arena *arena = M_ArenaAlloc(GB(1));
  if (arena)
  {
   os_w32_state = M_PushStruct(arena, OS_W32_State);
   os_w32_state->arena = arena;
   
   os_w32_state->threadArena = M_ArenaAlloc(KB(500));
   os_w32_state->mutexArena  = M_ArenaAlloc(KB(500));
   
   // Init SRW locks
   InitializeSRWLock(&os_w32_state->threadSRWLock);
   InitializeSRWLock(&os_w32_state->currentDirSRWLock);
   InitializeSRWLock(&os_w32_state->mutexesSRWLock);
   
   // gsp: System info
   QueryPerformanceFrequency(&os_w32_state->counterFrequency);
   // gsp: Enable tight granularity sleeps
   os_w32_state->granularSleepEnabled = timeBeginPeriod(1);
   {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    os_w32_state->logicalProcessorCount = systemInfo.dwNumberOfProcessors;
   }
   
   // Exe path
   {
    wchar_t exePathWithName16[MAX_PATH];
    GetModuleFileNameW(0, exePathWithName16, ArrayCount(exePathWithName16));
    os_w32_state->exePathWithName = String8From16(os_w32_state->arena, Str16C((u16*)exePathWithName16));
    Str8PathCurateBackslashes(os_w32_state->exePathWithName);
    os_w32_state->exeDirPath = PushStr8Copy(arena, Str8PathChopPastLastSlash(os_w32_state->exePathWithName));
    os_w32_state->exeName = Str8PathSkipLastSlash(os_w32_state->exePathWithName);
   }
   
   // Tmp path
   {
    wchar_t tmpDirPath[MAX_PATH];
    u32 tmpDirPathLen = (u32)GetTempPathW(ArrayCount(tmpDirPath), tmpDirPath);
    if (0 == tmpDirPathLen)
    {
     // TODO(gsp): handle failure to get tmpDirPath
     LogPrint(LogLevel_Error, "Temporary directory path was unable to be found!", 0);
    }
    
    os_w32_state->currentUserTmpPath = String8From16(arena, Str16((u16*)tmpDirPath, tmpDirPathLen));
    Str8PathCurateBackslashes(os_w32_state->currentUserTmpPath);
   }
   
   // Set receipt to valid
   OS_W32_InitReceiptSetValid(&receipt);
  }
 }
 
 return receipt;
}

function b32
OS_InitWasSuccessful(OS_InitReceipt receipt)
{
 return (receipt.a[0] == 1);
}

function void
OS_Shutdown(void)
{
}

//////////////////////
//~ gsp: Helpers
inline_function void 
OS_W32_InitReceiptSetValid(OS_InitReceipt *osInitReceipt)
{
 osInitReceipt->a[0] = 1;
}

inline_function b32
OS_W32_IsHandleValid(HANDLE handle)
{
 return (INVALID_HANDLE_VALUE != handle);
}

inline_function b32
OS_W32_IsValidHModule(HMODULE hmodule)
{
 return (0 != hmodule);
}

//////////////////////
//~ gsp: Memory functions
function u64
OS_PageSize(void)
{
 SYSTEM_INFO info;
 GetSystemInfo(&info);
 return info.dwPageSize;
}

function void *
OS_Reserve(u64 size)
{
 u64 gbSnappedSize = size;
 gbSnappedSize += GB(1) - 1;
 gbSnappedSize -= gbSnappedSize % GB(1);
 void *ptr = VirtualAlloc(0, gbSnappedSize, MEM_RESERVE, PAGE_NOACCESS);
 return ptr;
}

function void
OS_Release(void *ptr, u64 size)
{
 (void)size;
 VirtualFree(ptr, 0, MEM_RELEASE);
}

function void
OS_Commit(void *ptr, u64 size)
{
 u64 page_snapped_size = size;
 page_snapped_size += OS_PageSize() - 1;
 page_snapped_size -= page_snapped_size%OS_PageSize();
 VirtualAlloc(ptr, page_snapped_size, MEM_COMMIT, PAGE_READWRITE);
}

function void
OS_Decommit(void *ptr, u64 size)
{
 VirtualFree(ptr, size, MEM_DECOMMIT);
}

//////////////////////
//~ gsp: Thread stuff 

function u64
OS_ThreadCurrentID(void)
{
 u64 result = 0;
 result = (u64)GetCurrentThreadId();
 return result;
}

function OS_Handle
OS_ThreadStart(void *params, OS_ThreadFunction *func)
{
 OS_W32_Thread *thread = OS_W32_ThreadAlloc();
 if (thread)
 {
  thread->func = func;
  thread->params = params;
  {
   // gsp: CreateThread params
   LPSECURITY_ATTRIBUTES lpThreadAttributes = 0;
   SIZE_T dwStackSize = 0;
   DWORD dwCreationFlags = 0;
   thread->handle = CreateThread(lpThreadAttributes, dwStackSize, OS_W32_ThreadEntry, thread, dwCreationFlags, &thread->threadID);
  }
 }
 OS_Handle result  = {0};
 result.a[0] = (u64)thread;
 return result;
};

function void
OS_ThreadJoin(OS_Handle threadHandle)
{
 OS_W32_Thread *thread = OS_W32_ThreadPtrFromOSHandle(threadHandle);
 if (thread && thread->handle)
 {
  WaitForSingleObject(thread->handle, INFINITE);
  OS_W32_ThreadRelease(thread);
 }
}

function void
OS_ThreadDetach(OS_Handle threadHandle)
{
 OS_W32_Thread *thread = OS_W32_ThreadPtrFromOSHandle(threadHandle);
 if (thread)
 {
  if (thread->handle)
  {
   CloseHandle(thread->handle);
  }
  OS_W32_ThreadRelease(thread);
 }
}

///////////////////////
//~ gsp: Thread Alloc and Release 

function OS_W32_Thread *
OS_W32_ThreadAlloc(void)
{
 OS_W32_Thread *thread = 0;
 AcquireSRWLockExclusive(&os_w32_state->threadSRWLock);
 {
  thread = os_w32_state->firstFreeThread;
  if (0 != thread)
  {
   StackPop(os_w32_state->firstFreeThread);
  }
  else
  {
   thread = M_PushStructNoZero(os_w32_state->threadArena, OS_W32_Thread);
  }
  MemoryZero(thread, sizeof(OS_W32_Thread));
 }
 ReleaseSRWLockExclusive(&os_w32_state->threadSRWLock);
 return thread;
}

function void
OS_W32_ThreadRelease(OS_W32_Thread *thread)
{
 AcquireSRWLockExclusive(&os_w32_state->threadSRWLock);
 {
  StackPush(os_w32_state->firstFreeThread, thread);
 }
 ReleaseSRWLockExclusive(&os_w32_state->threadSRWLock);
}

///////////////////////
//~ gsp: OS_W32_ThreadEntry
function DWORD WINAPI
OS_W32_ThreadEntry(LPVOID lpParameter)
{
 OS_W32_Thread *thread = (OS_W32_Thread *)lpParameter;
 thread->func(thread->params);
 
 return (0);
}

//////////////////////
//~ gsp: Mutexes

function OS_Handle
OS_SRWMutexAlloc(void)
{
 OS_Handle result = {0};
 AcquireSRWLockExclusive(&os_w32_state->mutexesSRWLock);
 {
  OS_W32_SRWLockNode *mutex = os_w32_state->firstFreeMutex;
  if (0 != mutex)
  {
   StackPop(mutex);
  }
  else
  {
   mutex = M_PushStructNoZero(os_w32_state->mutexArena, OS_W32_SRWLockNode);
  }
  MemoryZeroStruct(mutex);
  result.a[0] = (u64)mutex;
 }
 ReleaseSRWLockExclusive(&os_w32_state->mutexesSRWLock);
 return result;
}

function void
OS_SRWMutexRelease(OS_Handle mutex)
{
 AcquireSRWLockExclusive(&os_w32_state->mutexesSRWLock);
 {
  OS_W32_SRWLockNode *lock = OS_W32_SRWLockNodeFromOSHandle(mutex);
  if (0 != lock)
  {
   StackPush(os_w32_state->firstFreeMutex, lock);
  }
 }
 ReleaseSRWLockExclusive(&os_w32_state->mutexesSRWLock);
}

function void
OS_SRWMutexWriterBlockEnter(OS_Handle mutex)
{
 OS_W32_SRWLockNode *lock = OS_W32_SRWLockNodeFromOSHandle(mutex);
 AcquireSRWLockExclusive(&lock->lock);
}

function void
OS_SRWMutexWriterBlockLeave(OS_Handle mutex)
{
 OS_W32_SRWLockNode *lock = OS_W32_SRWLockNodeFromOSHandle(mutex);
 ReleaseSRWLockExclusive(&lock->lock);
}

function void
OS_SRWMutexReaderBlockEnter(OS_Handle mutex)
{
 OS_W32_SRWLockNode *lock = OS_W32_SRWLockNodeFromOSHandle(mutex);
 AcquireSRWLockShared(&lock->lock);
}

function void
OS_SRWMutexReaderBlockLeave(OS_Handle mutex)
{
 OS_W32_SRWLockNode *lock = OS_W32_SRWLockNodeFromOSHandle(mutex);
 ReleaseSRWLockShared(&lock->lock);
}


//////////////////////
//~ gsp: File functions

function OS_Handle
OS_FileOpen(M_Arena *arena, String8 path, OS_FileAccessFlags flags)
{
 // gsp: get path16
 String16 path16 = String16From8(arena, path);
 
 // gsp: map flags to W32 flags
 DWORD dwDesiredAccess = 0;
 DWORD dwShareMode = 0;
 if (OS_FileAccess_Read & flags)
 {
  dwDesiredAccess |= GENERIC_READ;
 }
 if (OS_FileAccess_Read == flags)
 {
  dwShareMode |= FILE_SHARE_READ;
 }
 if (OS_FileAccess_Write & flags)
 {
  dwDesiredAccess |= GENERIC_WRITE;
 }
 if (OS_FileAccess_Execute & flags)
 {
  dwDesiredAccess |= GENERIC_EXECUTE;
 }
 
 DWORD dwCreationDisposition = 0;
 if (OS_FileAccess_Overwrite & flags)
 {
  dwCreationDisposition |= CREATE_ALWAYS;
 }
 else
 {
  if ((OS_FileAccess_CreateIfNotExists & flags) && !OS_W32_FileExists(path16))
   dwCreationDisposition |= CREATE_NEW;
 }
 if (OS_FileAccess_Read == flags)
 {
  dwCreationDisposition = OPEN_EXISTING;
 }
 
 // gsp: misc
 LPSECURITY_ATTRIBUTES securityAttributes = 0;
 DWORD dwFlagsAndAttributes = 0;
 HANDLE hTemplateFile = 0;
 
 HANDLE createFileHandle = CreateFileW((LPCWSTR)path16.str, dwDesiredAccess, dwShareMode, securityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
 
 if (INVALID_HANDLE_VALUE == createFileHandle)
 {
#if 1
  DWORD lastError = GetLastError();
  LogError(0, "Failed to open file: %d", lastError);
#endif
 }
 
 OS_Handle result = OS_HandleZero();
 result.a[0] = (u64)createFileHandle;
 
 return result;
}

function void
OS_FileClose(OS_Handle fileHandle)
{
 HANDLE handle = OS_W32_HandleFromOSHandle(fileHandle);
 if (OS_W32_IsHandleValid(handle))
 {
  CloseHandle(handle);
 }
}

inline_function b32
OS_FileIsValid(OS_Handle fileHandle)
{
 HANDLE handle = OS_W32_HandleFromOSHandle(fileHandle);
 return (OS_W32_IsHandleValid(handle));
}

function String8
OS_FileReadEntire(M_Arena *arena, OS_Handle fileHandle)
{
 String8 result = {0};
 HANDLE handle = OS_W32_HandleFromOSHandle(fileHandle);
 if (OS_W32_IsHandleValid(handle))
 {
  LARGE_INTEGER sizeLargeInt = {0};
  if (GetFileSizeEx(handle, &sizeLargeInt))
  {
   u32 size = U64SafeTruncateToU32(sizeLargeInt.QuadPart);
   u8 *buffer = M_PushArrayNoZero(arena, u8, size);
   u32 numberOfBytesRead = 0;
   if (ReadFile(handle, buffer, size, (LPDWORD)&numberOfBytesRead, 0) && (numberOfBytesRead == size))
   {
    result.str = buffer;
    result.size = size;
   }
  }
 }
 return result;
}

function u64
OS_FileWrite(OS_Handle fileHandle, String8 buffer)
{
 u32 numberOfBytesWritten = 0;
 HANDLE handle = OS_W32_HandleFromOSHandle(fileHandle);
 if (OS_W32_IsHandleValid(handle))
 {
  if (!WriteFile(handle, buffer.str, U64SafeTruncateToU32(buffer.size), (LPDWORD)&numberOfBytesWritten, 0))
  {
   numberOfBytesWritten = 0;
  }
 }
 u64 numberOfBytesWrittenU64 = (u32)numberOfBytesWritten;
 return numberOfBytesWrittenU64;
}

function u64
OS_FileGetLastWriteTime(OS_Handle fileHandle)
{
 u64 lastWriteTime = 0;
 HANDLE handle = OS_W32_HandleFromOSHandle(fileHandle);
 FILETIME lastWriteTimeFiletime = {0};
 LPFILETIME lpCreationTime = 0;
 LPFILETIME lpLastAccessTime = 0;
 if (GetFileTime(handle, lpCreationTime, lpLastAccessTime, &lastWriteTimeFiletime))
 {
  lastWriteTime = ((u64)lastWriteTimeFiletime.dwHighDateTime << 32) & (u64)lastWriteTimeFiletime.dwLowDateTime;
  StaticAssert(sizeof(DWORD) == sizeof(u32), "DWORD must have 32 bits for the above line to work");
 }
 return lastWriteTime;
}

function b32
OS_W32_FileExists(String16 path)
{
 b32 result = 0;
 HANDLE fileHandle = CreateFileW((LPCWSTR)path.str, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
 if (INVALID_HANDLE_VALUE != fileHandle)
 {
  result = 1;
 }
 CloseHandle(fileHandle);
 return result;
}

//////////////////////
//~ gsp: File search functions

function b32
OS_FileIterStart(M_Arena *arena, String8 searchDir, OS_Handle *outHandle, OS_FileInfo *outFileInfo)
{
 M_ArenaTemp scratch = GetScratch(&arena, 1);
 // gsp: Format path
 String8 formattedSearchDir = PushStr8Fmt(scratch.arena, "%s\\*", searchDir.str);
 String16 formattedSearchDir16 = String16From8(scratch.arena, formattedSearchDir);
 
 WIN32_FIND_DATAW ffd;
 HANDLE handle = FindFirstFileW((LPCWSTR)formattedSearchDir16.str, &ffd);
 OS_W32_FillFileInfo(arena, &ffd, outFileInfo);
 
 outHandle->a[0] = (u64)handle;
 b32 result = (handle != INVALID_HANDLE_VALUE) && ((u64)ERROR_FILE_NOT_FOUND != (u64)handle);
 
 EndScratch(scratch);
 return result;
}

// NOTE(gsp): returns non-zero if file was found
function b32
OS_FileIterGetNextFile(M_Arena *arena, OS_Handle osHandle, OS_FileInfo *outFileInfo)
{
 b32 result = 0;
 HANDLE handle = OS_W32_HandleFromOSHandle(osHandle);
 if (OS_W32_IsHandleValid(handle))
 {
  WIN32_FIND_DATAW ffd  = {0};
  result = (b32)FindNextFileW(handle, &ffd);
  OS_W32_FillFileInfo(arena, &ffd, outFileInfo);
 }
 return result;
}

function void
OS_FileIterEnd(OS_Handle osHandle)
{
 HANDLE handle = OS_W32_HandleFromOSHandle(osHandle);
 if (OS_W32_IsHandleValid(handle))
 {
  FindClose(handle);
 }
}

function void
OS_W32_FillFileInfo(M_Arena *arena, WIN32_FIND_DATAW *ffd, OS_FileInfo *outFileInfo)
{
 MemoryZeroStruct(outFileInfo);
 outFileInfo->creationTime = ((u64)ffd->ftCreationTime.dwHighDateTime << 32) | ((u64)ffd->ftCreationTime.dwLowDateTime);
 outFileInfo->lastAccessTime = ((u64)ffd->ftLastAccessTime.dwHighDateTime << 32) | ((u64)ffd->ftLastAccessTime.dwLowDateTime);
 outFileInfo->lastWriteTime = ((u64)ffd->ftLastWriteTime.dwHighDateTime << 32) | ((u64)ffd->ftLastWriteTime.dwLowDateTime);
 outFileInfo->size = ((u64)ffd->nFileSizeHigh << 32) | ((u64)ffd->nFileSizeLow);
 outFileInfo->filename = String8From16(arena, Str16C((u16*)ffd->cFileName));
 if (FILE_ATTRIBUTE_DIRECTORY & ffd->dwFileAttributes)
  outFileInfo->flags |= OS_FileInfo_IsDir;
}

//////////////////////
//~ gsp: Time functions

inline_function DateTime
OS_DateTimeCurrent(void)
{
 SYSTEMTIME systemTime;
 GetSystemTime(&systemTime);
 // gsp: map to DateTime
 DateTime result     = {0};
 result.year         = systemTime.wYear;
 result.month        = (u8)systemTime.wMonth;
 result.dayOfWeek    = (u8)systemTime.wDayOfWeek;
 result.day          = (u8)systemTime.wDay;
 result.hours        = (u8)systemTime.wHour;
 result.minutes      = (u8)systemTime.wMinute;
 result.seconds      = (u8)systemTime.wSecond;
 result.milliseconds = systemTime.wMilliseconds;
 return result;
}

inline_function f64 
OS_TimeSecondsF64(void)
{
 LARGE_INTEGER currentTime;
 QueryPerformanceCounter(&currentTime);
 f64 timeInSeconds = ((f64)currentTime.QuadPart)/((f64)os_w32_state->counterFrequency.QuadPart);
 return timeInSeconds;
}

inline_function f32
OS_TimeSecondsF32(void)
{
 return (f32)OS_TimeSecondsF64();
}

inline_function u64
OS_TimeMicroseconds(void)
{
 f64 timeInSeconds = OS_TimeSecondsF64();
 u64 timeInMicroseconds = (u64)(timeInSeconds * Million(1));
 return timeInMicroseconds;
}

inline_function u64
OS_TimeCycles(void)
{
 u64 result = __rdtsc();
 return result;
}

inline_function void
OS_Sleep(u64 milliseconds)
{
 Sleep(U64SafeTruncateToU32(milliseconds));
}

function void
OS_Wait(u64 endTimeMicroseconds)
{
 u64 currentTimeMicroseconds = OS_TimeMicroseconds();
 if (currentTimeMicroseconds < endTimeMicroseconds)
 {
  u64 timeToWaitMicroseconds = endTimeMicroseconds - currentTimeMicroseconds;
  if(os_w32_state->granularSleepEnabled)
  {
   OS_Sleep(timeToWaitMicroseconds/1000);
  }
  else
  {
   OS_Sleep(timeToWaitMicroseconds/15000);
  }
  for(;OS_TimeMicroseconds() < endTimeMicroseconds;);
 }
}

//////////////////////
//~ gsp: System info

function f32
OS_CaretBlinkTimeSeconds(void)
{
 f32 result = InfinityF32;
 UINT caretBlinkTimeMS = GetCaretBlinkTime();
 if (INFINITE != caretBlinkTimeMS)
 {
  result = (f32)caretBlinkTimeMS / 1000.f;
 }
 return result;
}

function f32
OS_DoubleClickTimeSeconds(void)
{
 f32 result = 0.5f;
 UINT doubleClickTimeMS = GetDoubleClickTime();
 if (5000 > doubleClickTimeMS)
 {
  result = (f32)doubleClickTimeMS / 1000.f;
 }
 return result;
}

function u32
OS_LogicalProcessorCount(void)
{
 return os_w32_state->logicalProcessorCount;
}


///////////////////////////
//~ gsp: @per_os Dynamic Libraries 

function OS_SharedLib
OS_LibLoad(M_Arena *arena, String8 path)
{
 String16 path16 = String16From8(arena, path);
 // NOTE(gsp): returns null on failure
 HMODULE hmodule = LoadLibraryW((LPCWSTR)path16.str);
 OS_SharedLib result;
 result.a[0] = (u64)hmodule;
 return result;
}

function VoidFunc *
OS_LibGetProc(OS_SharedLib lib, String8 name)
{
 VoidFunc *result = 0;
 HMODULE hModule = OS_W32_HModuleFromOSSharedLib(lib);
 if (OS_W32_IsValidHModule(hModule))
 {
  result = (VoidFunc *)GetProcAddress(hModule, (LPCSTR)name.str);
 }
 return result;
}

function void
OS_LibRelease(OS_SharedLib lib)
{
 HMODULE hModule = OS_W32_HModuleFromOSSharedLib(lib);
 if (OS_W32_IsValidHModule(hModule))
 {
  FreeLibrary(hModule);
 }
}


///////////////////////////
//~ gsp: Entropy

///////////////////////////
//~ gsp: Command line args

///////////////////////////
//~ gsp: Path stuff
function String8
OS_GetExeDirPath(void)
{
 return os_w32_state->exeDirPath;
}

function String8
OS_GetExeName(void)
{
 return os_w32_state->exeName;
}

function String8
OS_GetExePathWithName(void)
{
 return os_w32_state->exePathWithName;
}

function String8
OS_GetCurrentWorkingDir(M_Arena *arena)
{
 wchar_t dir[MAX_PATH];
 AcquireSRWLockExclusive(&os_w32_state->threadSRWLock);
 {
  GetCurrentDirectoryW(MAX_PATH, dir);
 }
 ReleaseSRWLockExclusive(&os_w32_state->threadSRWLock);
 
 String8 result = String8From16(arena, Str16C((u16*)dir));
 return result;
}

function b32
OS_SetCurrentWorkingDir(String8 path)
{
 M_ArenaTemp scratch = GetScratch(0, 0);
 b32 result = (b32)SetCurrentDirectoryW((LPCWSTR)String16From8(scratch.arena, path).str);
 EndScratch(scratch);
 return result;
}

function String8
OS_GetCurrentUserTmpPath(void)
{
 return os_w32_state->currentUserTmpPath;
}

///////////////////////////
//~ BOTTOM
#endif // #ifndef OS_CORE_WIN32_INCLUDE_C