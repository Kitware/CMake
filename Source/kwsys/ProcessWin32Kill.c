/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#define KWSYS_IN_PROCESS_C
#include "kwsysPrivate.h"
#include KWSYS_HEADER(ProcessWin32Kill.h)

/* The following process tree kill implementation is taken from
   http://www.alexfedotov.com/articles/killproc.asp
   It will work only on some versions of windows.  Hopefully
   I will eventually get some time to do a real implementation of this
   for all windows versions.  */

#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdarg.h>
#include <tlhelp32.h>

//---------------------------------------------------------------------------
// KillProcess
//
//  Terminates the specified process.
//
//  Parameters:
//        dwProcessId - identifier of the process to terminate
//
//  Returns:
//        TRUE, if successful, FALSE - otherwise.
//
static BOOL
WINAPI
KillProcess(
  IN DWORD dwProcessId
  )
{
  HANDLE hProcess;
  DWORD dwError;

  // first try to obtain handle to the process without the use of any
  // additional privileges
  hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
  if (hProcess == NULL)
    {
    OSVERSIONINFO osvi;
    TOKEN_PRIVILEGES Priv, PrivOld;
    DWORD cbPriv;
    HANDLE hToken;

    if (GetLastError() != ERROR_ACCESS_DENIED)
      return FALSE;

    // determine operating system version
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    // we cannot do anything else if this is not Windows NT
    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
      return SetLastError(ERROR_ACCESS_DENIED), FALSE;

    // enable SE_DEBUG_NAME privilege and try again

    cbPriv = sizeof(PrivOld);

    // obtain the token of the current thread 
    if (!OpenThreadToken(GetCurrentThread(), 
                         TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
                         FALSE, &hToken))
      {
      if (GetLastError() != ERROR_NO_TOKEN)
        return FALSE;

      // revert to the process token
      if (!OpenProcessToken(GetCurrentProcess(),
                            TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
                            &hToken))
        return FALSE;
      }

    if(!(ANYSIZE_ARRAY > 0))
      {
      return 0;
      }

    Priv.PrivilegeCount = 1;
    Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Priv.Privileges[0].Luid);

    // try to enable the privilege
    if (!AdjustTokenPrivileges(hToken, FALSE, &Priv, sizeof(Priv),
                               &PrivOld, &cbPriv))
      {
      dwError = GetLastError();
      CloseHandle(hToken);
      return SetLastError(dwError), FALSE;
      }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
      {
      // the SE_DEBUG_NAME privilege is not present in the caller's
      // token
      CloseHandle(hToken);
      return SetLastError(ERROR_ACCESS_DENIED), FALSE;
      }

    // try to open process handle again
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    dwError = GetLastError();
                
    // restore the original state of the privilege
    AdjustTokenPrivileges(hToken, FALSE, &PrivOld, sizeof(PrivOld),
                          NULL, NULL);
    CloseHandle(hToken);

    if (hProcess == NULL)
      {
      return SetLastError(FALSE), 0;
      }
    
    }

  // terminate the process
  if (!TerminateProcess(hProcess, (UINT)-1))
    {
    dwError = GetLastError();
    CloseHandle(hProcess);
    return SetLastError(dwError), FALSE;
    }

  CloseHandle(hProcess);

  // completed successfully
  return TRUE;
}

typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID {
  DWORD           UniqueProcess;
  DWORD           UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
  USHORT          Length;
  USHORT          MaximumLength;
  PWSTR           Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
  SIZE_T          PeakVirtualSize;
  SIZE_T          VirtualSize;
  ULONG           PageFaultCount;
  SIZE_T          PeakWorkingSetSize;
  SIZE_T          WorkingSetSize;
  SIZE_T          QuotaPeakPagedPoolUsage;
  SIZE_T          QuotaPagedPoolUsage;
  SIZE_T          QuotaPeakNonPagedPoolUsage;
  SIZE_T          QuotaNonPagedPoolUsage;
  SIZE_T          PagefileUsage;
  SIZE_T          PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
  LARGE_INTEGER   KernelTime;
  LARGE_INTEGER   UserTime;
  LARGE_INTEGER   CreateTime;
  ULONG                       WaitTime;
  PVOID                       StartAddress;
  CLIENT_ID       ClientId;
  KPRIORITY       Priority;
  KPRIORITY       BasePriority;
  ULONG                       ContextSwitchCount;
  LONG                        State;
  LONG                        WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

// Note that the size of the SYSTEM_PROCESSES structure is different on
// NT 4 and Win2K, but we don't care about it, since we don't access neither
// IoCounters member nor Threads array

typedef struct _SYSTEM_PROCESSES {
  ULONG                       NextEntryDelta;
  ULONG                       ThreadCount;
  ULONG                       Reserved1[6];
  LARGE_INTEGER   CreateTime;
  LARGE_INTEGER   UserTime;
  LARGE_INTEGER   KernelTime;
  UNICODE_STRING  ProcessName;
  KPRIORITY       BasePriority;
  ULONG                       ProcessId;
  ULONG                       InheritedFromProcessId;
  ULONG                       HandleCount;
  ULONG                       Reserved2[2];
  VM_COUNTERS     VmCounters;
#if _WIN32_WINNT >= 0x500
  IO_COUNTERS     IoCounters;
#endif
  SYSTEM_THREADS  Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;

//---------------------------------------------------------------------------
// KillProcessTreeNtHelper
//
//  This is a recursive helper function that terminates all the processes
//  started by the specified process and them terminates the process itself
//
//  Parameters:
//        pInfo       - processes information
//        dwProcessId - identifier of the process to terminate
//
//  Returns:
//        Win32 error code.
//
static
BOOL
WINAPI
KillProcessTreeNtHelper(
  IN PSYSTEM_PROCESSES pInfo,
  IN DWORD dwProcessId
  )
{
  PSYSTEM_PROCESSES p;
  if(!pInfo)
    {
    return 0;
    }

  p = pInfo;

  // kill all children first
  for (;;)
    {
    if (p->InheritedFromProcessId == dwProcessId)
      KillProcessTreeNtHelper(pInfo, p->ProcessId);

    if (p->NextEntryDelta == 0)
      break;

    // find the address of the next process structure
    p = (PSYSTEM_PROCESSES)(((LPBYTE)p) + p->NextEntryDelta);
    }

  // kill the process itself
  if (!KillProcess(dwProcessId))
    return GetLastError();

  return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// KillProcessTreeWinHelper
//
//  This is a recursive helper function that terminates all the processes
//  started by the specified process and them terminates the process itself
//
//  Parameters:
//        dwProcessId - identifier of the process to terminate
//
//  Returns:
//        Win32 error code.
//
static
BOOL
WINAPI
KillProcessTreeWinHelper(
  IN DWORD dwProcessId
  )
{
  HINSTANCE hKernel; 
  HANDLE hSnapshot;
  PROCESSENTRY32 Entry;

  HANDLE (WINAPI * _CreateToolhelp32Snapshot)(DWORD, DWORD);
  BOOL (WINAPI * _Process32First)(HANDLE, PROCESSENTRY32 *);
  BOOL (WINAPI * _Process32Next)(HANDLE, PROCESSENTRY32 *);

  // get handle to KERNEL32.DLL
  hKernel = GetModuleHandle(_T("kernel32.dll"));
  if(!hKernel)
    {
    return 0;
    }

  // locate necessary functions in KERNEL32.DLL
  *(FARPROC *)&_CreateToolhelp32Snapshot =
    GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
  *(FARPROC *)&_Process32First =
    GetProcAddress(hKernel, "Process32First");
  *(FARPROC *)&_Process32Next =
    GetProcAddress(hKernel, "Process32Next");

  if (_CreateToolhelp32Snapshot == NULL ||
      _Process32First == NULL ||
      _Process32Next == NULL)
    return ERROR_PROC_NOT_FOUND;


  // create a snapshot
  hSnapshot = _CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return GetLastError();

  Entry.dwSize = sizeof(Entry);
  if (!_Process32First(hSnapshot, &Entry))
    {
    DWORD dwError = GetLastError();
    CloseHandle(hSnapshot);
    return dwError;
    }

  // kill all children first
  do
    {
    if (Entry.th32ParentProcessID == dwProcessId)
      KillProcessTreeWinHelper(Entry.th32ProcessID);

    Entry.dwSize = sizeof(Entry);
    }
  while (_Process32Next(hSnapshot, &Entry));

  CloseHandle(hSnapshot);

  // kill the process itself
  if (!KillProcess(dwProcessId))
    return GetLastError();

  return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// KillProcessEx
//
//  Terminates the specified process and, optionally, all processes started
//      from the specified process (the so-called process tree).
//
//  Parameters:
//        dwProcessId - identifier of the process to terminate
//        bTree           - specifies whether the entire process tree should be
//                                      terminated
//
//  Returns:
//        TRUE, if successful, FALSE - otherwise.
//
static BOOL
WINAPI
KillProcessEx(
  IN DWORD dwProcessId,
  IN BOOL bTree
  )
{
  OSVERSIONINFO osvi;
  DWORD dwError;
  HANDLE hHeap;
  NTSTATUS Status;
  ULONG cbBuffer;
  PVOID pBuffer = NULL;

  if (!bTree)
    return KillProcess(dwProcessId);


  // determine operating system version
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  GetVersionEx(&osvi);

  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
      osvi.dwMajorVersion < 5)
    {
    HINSTANCE hNtDll;
    NTSTATUS (WINAPI * _ZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);

    // get handle to NTDLL.DLL
    hNtDll = GetModuleHandle(_T("ntdll.dll"));
    if(!hNtDll)
      {
      return 0;
      }

    // find the address of ZwQuerySystemInformation
    *(FARPROC *)&_ZwQuerySystemInformation =
      GetProcAddress(hNtDll, "ZwQuerySystemInformation");
    if (_ZwQuerySystemInformation == NULL)
      return SetLastError(ERROR_PROC_NOT_FOUND), 0;

    // obtain a handle to the default process heap
    hHeap = GetProcessHeap();
    
    cbBuffer = 0x8000;

    // it is difficult to say a priory which size of the buffer 
    // will be enough to retrieve all information, so we start
    // with 32K buffer and increase its size until we get the
    // information successfully
    do
      {
      pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
      if (pBuffer == NULL)
        return SetLastError(ERROR_NOT_ENOUGH_MEMORY), FALSE;

      Status = _ZwQuerySystemInformation(
        SystemProcessesAndThreadsInformation,
        pBuffer, cbBuffer, NULL);

      if (Status == STATUS_INFO_LENGTH_MISMATCH)
        {
        HeapFree(hHeap, 0, pBuffer);
        cbBuffer *= 2;
        }
      else if (!NT_SUCCESS(Status))
        {
        HeapFree(hHeap, 0, pBuffer);
        return SetLastError(Status), 0;
        }
      }
    while (Status == STATUS_INFO_LENGTH_MISMATCH);

    // call the helper function
    dwError = KillProcessTreeNtHelper((PSYSTEM_PROCESSES)pBuffer, 
                                      dwProcessId);
                
    HeapFree(hHeap, 0, pBuffer);
    }
  else
    {
    // call the helper function
    dwError = KillProcessTreeWinHelper(dwProcessId);
    }

  SetLastError(dwError);
  return dwError == ERROR_SUCCESS;
}

int kwsysProcessWin32Kill(int pid)
{
  return KillProcessEx(pid, 1)? 1:0;
}
