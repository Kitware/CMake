/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmWin32ProcessExecution.h"

#include "cmSystemTools.h"   

#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>

#if defined(__BORLANDC__)
#  define STRICMP stricmp
#  define TO_INTPTR(x) ((long)(x))
#else // Visual studio
#  if ( _MSC_VER >= 1300 )
#    include <stddef.h>
#    define TO_INTPTR(x) ((intptr_t)(x))
#  else // Visual Studio 6
#    define TO_INTPTR(x) ((long)(x))
#  endif // Visual studio .NET
#  define STRICMP _stricmp
#endif // Borland

#define POPEN_1 1
#define POPEN_2 2
#define POPEN_3 3
#define POPEN_4 4

#define cmMAX(x,y) (((x)<(y))?(y):(x))

#define win32_error(x,y) std::cout << "Win32_Error(" << x << ", " << y << ")" << std::endl, false


void DisplayErrorMessage()
{
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
  // Process any inserts in lpMsgBuf.
  // ... 
  // Display the string.
  MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
  // Free the buffer.
  LocalFree( lpMsgBuf );
}
 
// Code from a Borland web site with the following explaination : 
/* In this article, I will explain how to spawn a console application
 * and redirect its standard input/output using anonymous pipes. An
 * anonymous pipe is a pipe that goes only in one direction (read
 * pipe, write pipe, etc.). Maybe you are asking, "why would I ever
 * need to do this sort of thing?" One example would be a Windows
 * telnet server, where you spawn a shell and listen on a port and
 * send and receive data between the shell and the socket
 * client. (Windows does not really have a built-in remote
 * shell). First, we should talk about pipes. A pipe in Windows is
 * simply a method of communication, often between process. The SDK
 * defines a pipe as "a communication conduit with two ends;
 a process
 * with a handle to one end can communicate with a process having a
 * handle to the other end." In our case, we are using "anonymous"
 * pipes, one-way pipes that "transfer data between a parent process
 * and a child process or between two child processes of the same
 * parent process." It's easiest to imagine a pipe as its namesake. An
 * actual pipe running between processes that can carry data. We are
 * using anonymous pipes because the console app we are spawning is a
 * child process. We use the CreatePipe function which will create an
 * anonymous pipe and return a read handle and a write handle. We will
 * create two pipes, on for stdin and one for stdout. We will then
 * monitor the read end of the stdout pipe to check for display on our
 * child process. Every time there is something availabe for reading,
 * we will display it in our app. Consequently, we check for input in
 * our app and send it off to the write end of the stdin pipe. */ 

inline bool IsWinNT() 
//check if we're running NT 
{
  OSVERSIONINFO osv;
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);
  return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT); 
}

//--------------------------------------------------------------------------- 
bool cmWin32ProcessExecution::BorlandRunCommand(
  const char* command, const char* dir, 
  std::string& output, int& retVal, bool verbose, int /* timeout */) 
{
  //verbose = true;
  //std::cerr << std::endl 
  //        << "WindowsRunCommand(" << command << ")" << std::endl 
  //        << std::flush;
  const int BUFFER_SIZE = 4096;
  char buf[BUFFER_SIZE];
 
//i/o buffer 
  STARTUPINFO si;
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
 
//security information for pipes 
  PROCESS_INFORMATION pi;
  HANDLE newstdin,newstdout,read_stdout,write_stdin;
 
//pipe handles 
  if (IsWinNT()) 
//initialize security descriptor (Windows NT) 
    {
    InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);
    sa.lpSecurityDescriptor = &sd;
 
    }
  else sa.lpSecurityDescriptor = NULL;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = true;
 
//allow inheritable handles 
  if (!CreatePipe(&newstdin,&write_stdin,&sa,0)) 
//create stdin pipe 
    {
    std::cerr << "CreatePipe" << std::endl;
    return false;
 
    }
  if (!CreatePipe(&read_stdout,&newstdout,&sa,0)) 
//create stdout pipe 
    {
    std::cerr << "CreatePipe" << std::endl;
    CloseHandle(newstdin);
    CloseHandle(write_stdin);
    return false;
 
    }
  GetStartupInfo(&si);
 
//set startupinfo for the spawned process 
  /* The dwFlags member tells CreateProcess how to make the
   * process. STARTF_USESTDHANDLES validates the hStd*
   * members. STARTF_USESHOWWINDOW validates the wShowWindow
   * member. */ 
  
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  si.hStdOutput = newstdout;
  si.hStdError = newstdout;
  si.wShowWindow = SW_HIDE;
 
//set the new handles for the child process si.hStdInput = newstdin;
  char* commandAndArgs = strcpy(new char[strlen(command)+1], command);
  if (!CreateProcess(NULL,commandAndArgs,NULL,NULL,TRUE,CREATE_NEW_CONSOLE, 
                     NULL,dir,&si,&pi)) 
    {
    std::cerr << "CreateProcess failed " << commandAndArgs << std::endl;
    CloseHandle(newstdin);
    CloseHandle(newstdout);
    CloseHandle(read_stdout);
    CloseHandle(write_stdin);
    delete [] commandAndArgs;
    return false;
 
    }
  delete [] commandAndArgs;
  unsigned long exit=0;
 
//process exit code unsigned 
  unsigned long bread;
 
//bytes read unsigned 
  unsigned long avail;
 
//bytes available 
  memset(buf, 0, sizeof(buf));
  for(;;) 
//main program loop 
    {
    Sleep(10);
//check to see if there is any data to read from stdout 
    //std::cout << "Peek for data..." << std::endl;
    PeekNamedPipe(read_stdout,buf,1023,&bread,&avail,NULL);
    if (bread != 0) 
      {
      memset(buf, 0, sizeof(buf));
      if (avail > 1023) 
        {
        while (bread >= 1023) 
          {
          //std::cout << "Read data..." << std::endl;
          ReadFile(read_stdout,buf,1023,&bread,NULL);
 
          //read the stdout pipe 
          memset(buf, 0, sizeof(buf));
          output += buf;
          if (verbose)
            {
            std::cout << buf << std::flush; 
            }
          }
        }
      else 
        {
        ReadFile(read_stdout,buf,1023,&bread,NULL);
        output += buf;
        if(verbose) 
          {
          std::cout << buf << std::flush;
          }
 
        }
 
      }
 
    //std::cout << "Check for process..." << std::endl;
    GetExitCodeProcess(pi.hProcess,&exit);
 
//while the process is running 
    if (exit != STILL_ACTIVE) break;
 
    }
  WaitForSingleObject(pi.hProcess, INFINITE);
  GetExitCodeProcess(pi.hProcess,&exit);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  CloseHandle(newstdin);
 
//clean stuff up 
  CloseHandle(newstdout);
  CloseHandle(read_stdout);
  CloseHandle(write_stdin);
  retVal = exit;
  return true;
 
}

bool cmWin32ProcessExecution::StartProcess(
  const char* cmd, const char* path, bool verbose)
{
  this->Initialize();
  this->m_Verbose = verbose;
  return this->PrivateOpen(cmd, path, _O_RDONLY | _O_TEXT, POPEN_3);
}

bool cmWin32ProcessExecution::Wait(int timeout)
{
  return this->PrivateClose(timeout);
}

/*
 * Internal dictionary mapping popen* file pointers to process handles,
 * for use when retrieving the process exit code.  See _PyPclose() below
 * for more information on this dictionary's use.
 */
static void *_PyPopenProcs = NULL;

static BOOL RealPopenCreateProcess(const char *cmdstring,
                                   const char *path, 
                                   const char *szConsoleSpawn,
                                   HANDLE hStdin,
                                   HANDLE hStdout,
                                   HANDLE hStderr,
                                   HANDLE *hProcess)
{
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  char *s1,*s2, *s3 = " /c ";
  int i;
  int x;
  if (i = GetEnvironmentVariable("COMSPEC",NULL,0)) 
    {
    char *comshell;

    s1 = (char *)_alloca(i);
    if (!(x = GetEnvironmentVariable("COMSPEC", s1, i)))
      {
      return x;
      }

    /* Explicitly check if we are using COMMAND.COM.  If we are
     * then use the w9xpopen hack.
     */
    comshell = s1 + x;
    while (comshell >= s1 && *comshell != '\\')
      --comshell;
    ++comshell;

    if (GetVersion() < 0x80000000 &&
        STRICMP(comshell, "command.com") != 0) 
      {
      /* NT/2000 and not using command.com. */
      x = i + (int)strlen(s3) + (int)strlen(cmdstring) + 1;
      s2 = (char *)_alloca(x);
      ZeroMemory(s2, x);
      //sprintf(s2, "%s%s%s", s1, s3, cmdstring);
      sprintf(s2, "%s", cmdstring);
      }
    else 
      {
      /*
       * Oh gag, we're on Win9x or using COMMAND.COM. Use
       * the workaround listed in KB: Q150956
       */
      char modulepath[_MAX_PATH];
      struct stat statinfo;
      GetModuleFileName(NULL, modulepath, sizeof(modulepath));
      for (i = x = 0; modulepath[i]; i++)
        if (modulepath[i] == '\\')
          x = i+1;
      modulepath[x] = '\0';
      /* Create the full-name to w9xpopen, so we can test it exists */
      strncat(modulepath, 
              szConsoleSpawn, 
              (sizeof(modulepath)/sizeof(modulepath[0]))
              -strlen(modulepath));
      if (stat(modulepath, &statinfo) != 0) 
        {
          /* Eeek - file-not-found - possibly an embedding 
             situation - see if we can locate it in sys.prefix 
          */
        strncpy(modulepath, 
                ".", 
                sizeof(modulepath)/sizeof(modulepath[0]));
        if (modulepath[strlen(modulepath)-1] != '\\')
          strcat(modulepath, "\\");
        strncat(modulepath, 
                szConsoleSpawn, 
                (sizeof(modulepath)/sizeof(modulepath[0]))
                -strlen(modulepath));
        /* No where else to look - raise an easily identifiable
           error, rather than leaving Windows to report
           "file not found" - as the user is probably blissfully
           unaware this shim EXE is used, and it will confuse them.
           (well, it confused me for a while ;-)
        */
        if (stat(modulepath, &statinfo) != 0) 
          {
          std::cout 
            << "Can not locate '" << modulepath
            << "' which is needed "
            "for popen to work with your shell "
            "or platform." << std::endl;
          return FALSE;
          }
        }
      x = i + (int)strlen(s3) + (int)strlen(cmdstring) + 1 +
        (int)strlen(modulepath) + 
        (int)strlen(szConsoleSpawn) + 1;

      s2 = (char *)_alloca(x);
      ZeroMemory(s2, x);
      sprintf(
        s2,
        "%s %s%s%s",
        modulepath,
        s1,
        s3,
        cmdstring);
      sprintf(
        s2,
        "%s %s",
        modulepath,
        cmdstring);
      }
    }

  /* Could be an else here to try cmd.exe / command.com in the path
     Now we'll just error out.. */
  else 
    {
    std::cout << "Cannot locate a COMSPEC environment variable to "
              << "use as the shell" << std::endl;
    return FALSE;
    }
  
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  siStartInfo.hStdInput = hStdin;
  siStartInfo.hStdOutput = hStdout;
  siStartInfo.hStdError = hStderr;
  siStartInfo.wShowWindow = SW_HIDE;

  //std::cout << "Create process: " << s2 << std::endl;
  if (CreateProcess(NULL,
                    s2,
                    NULL,
                    NULL,
                    TRUE,
                    CREATE_NEW_CONSOLE,
                    NULL,
                    path,
                    &siStartInfo,
                    &piProcInfo) ) 
    {
    /* Close the handles now so anyone waiting is woken. */
    CloseHandle(piProcInfo.hThread);
    /* Return process handle */
    *hProcess = piProcInfo.hProcess;
    //std::cout << "Process created..." << std::endl;
    return TRUE;
    }
  win32_error("CreateProcess", s2);
  return FALSE;
}

/* The following code is based off of KB: Q190351 */

bool cmWin32ProcessExecution::PrivateOpen(const char *cmdstring, 
                                          const char* path,
                                          int mode, 
                                          int n)
{
  HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr,
    hChildStderrRd, hChildStderrWr, hChildStdinWrDup, hChildStdoutRdDup,
    hChildStderrRdDup, hProcess; /* hChildStdoutWrDup; */

  SECURITY_ATTRIBUTES saAttr;
  BOOL fSuccess;
  int fd1, fd2, fd3;
  //FILE *f1, *f2, *f3;

  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
    {
    return win32_error("CreatePipe", NULL);
    }
    
  /* Create new output read handle and the input write handle. Set
   * the inheritance properties to FALSE. Otherwise, the child inherits
   * the these handles; resulting in non-closeable handles to the pipes
   * being created. */
  fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
                             GetCurrentProcess(), &hChildStdinWrDup, 0,
                             FALSE,
                             DUPLICATE_SAME_ACCESS);
  if (!fSuccess)
    return win32_error("DuplicateHandle", NULL);


  /* Close the inheritable version of ChildStdin
     that we're using. */
  CloseHandle(hChildStdinWr);

  if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
    return win32_error("CreatePipe", NULL);

  fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                             GetCurrentProcess(), &hChildStdoutRdDup, 0,
                             FALSE, DUPLICATE_SAME_ACCESS);
  if (!fSuccess)
    return win32_error("DuplicateHandle", NULL);

  /* Close the inheritable version of ChildStdout
     that we're using. */
  CloseHandle(hChildStdoutRd);

  if (n != POPEN_4) 
    {
    if (!CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0))
      return win32_error("CreatePipe", NULL);
   fSuccess = DuplicateHandle(GetCurrentProcess(),
                               hChildStderrRd,
                               GetCurrentProcess(),
                               &hChildStderrRdDup, 0,
                               FALSE, DUPLICATE_SAME_ACCESS);
    if (!fSuccess)
      return win32_error("DuplicateHandle", NULL);
    /* Close the inheritable version of ChildStdErr that we're using. */
    CloseHandle(hChildStderrRd);

    }
          
  switch (n) 
    {
    case POPEN_1:
      switch (mode & (_O_RDONLY | _O_TEXT | _O_BINARY | _O_WRONLY)) 
        {
        case _O_WRONLY | _O_TEXT:
          /* Case for writing to child Stdin in text mode. */
          fd1 = _open_osfhandle(TO_INTPTR(hChildStdinWrDup), mode);
          //f1 = _fdopen(fd1, "w");
          /* We don't care about these pipes anymore,
             so close them. */
          CloseHandle(hChildStdoutRdDup);
          CloseHandle(hChildStderrRdDup);
          break;

        case _O_RDONLY | _O_TEXT:
          /* Case for reading from child Stdout in text mode. */
          fd1 = _open_osfhandle(TO_INTPTR(hChildStdoutRdDup), mode);
          //f1 = _fdopen(fd1, "r");
          /* We don't care about these pipes anymore,
             so close them. */
          CloseHandle(hChildStdinWrDup);
          CloseHandle(hChildStderrRdDup);
          break;

        case _O_RDONLY | _O_BINARY:
          /* Case for readinig from child Stdout in
             binary mode. */
          fd1 = _open_osfhandle(TO_INTPTR(hChildStdoutRdDup), mode);
          //f1 = _fdopen(fd1, "rb");
          /* We don't care about these pipes anymore,
             so close them. */
          CloseHandle(hChildStdinWrDup);
          CloseHandle(hChildStderrRdDup);
          break;

        case _O_WRONLY | _O_BINARY:
          /* Case for writing to child Stdin in binary mode. */
          fd1 = _open_osfhandle(TO_INTPTR(hChildStdinWrDup), mode);
          //f1 = _fdopen(fd1, "wb");
          /* We don't care about these pipes anymore,
             so close them. */
          CloseHandle(hChildStdoutRdDup);
          CloseHandle(hChildStderrRdDup);
          break;
        }
      break;
        
    case POPEN_2:
    case POPEN_4: 
      if ( 1 ) 
        {
        // Comment this out. Maybe we will need it in the future.
        // file IO access to the process might be cool.
        //char *m1,  *m2;
        
        //if (mode && _O_TEXT) 
        //  {
        //  m1 = "r";
        //  m2 = "w";
        //  } 
        //else 
        //  {
        //  m1 = "rb";
        //  m2 = "wb";
        //  }

        fd1 = _open_osfhandle(TO_INTPTR(hChildStdinWrDup), mode);
        //f1 = _fdopen(fd1, m2);
        fd2 = _open_osfhandle(TO_INTPTR(hChildStdoutRdDup), mode);
        //f2 = _fdopen(fd2, m1);

        if (n != 4)
          {
          CloseHandle(hChildStderrRdDup);
          }

        break;
        }
        
    case POPEN_3:
      if ( 1) 
        {
        // Comment this out. Maybe we will need it in the future.
        // file IO access to the process might be cool.
        //char *m1, *m2;
      
        //if (mode && _O_TEXT) 
        //  {
        //  m1 = "r";
        //  m2 = "w";
        //  } 
        //else 
        //  {
        //  m1 = "rb";
        //  m2 = "wb";
        //  }


        fd1 = _open_osfhandle(TO_INTPTR(hChildStdinWrDup), mode);
        //f1 = _fdopen(fd1, m2);
        fd2 = _open_osfhandle(TO_INTPTR(hChildStdoutRdDup), mode);
        //f2 = _fdopen(fd2, m1);
        fd3 = _open_osfhandle(TO_INTPTR(hChildStderrRdDup), mode);
        //f3 = _fdopen(fd3, m1);        

        break;
        }
    }

  if (n == POPEN_4) 
    {
    if (!RealPopenCreateProcess(cmdstring,
                                path,
                                this->m_ConsoleSpawn.c_str(),
                                hChildStdinRd,
                                hChildStdoutWr,
                                hChildStdoutWr,
                                &hProcess))
      return NULL;
    }
  else 
    {
    if (!RealPopenCreateProcess(cmdstring,
                                path,
                                this->m_ConsoleSpawn.c_str(),
                                hChildStdinRd,
                                hChildStdoutWr,
                                hChildStderrWr,
                                &hProcess))
      return NULL;
    }

  /*
   * Insert the files we've created into the process dictionary
   * all referencing the list with the process handle and the
   * initial number of files (see description below in _PyPclose).
   * Since if _PyPclose later tried to wait on a process when all
   * handles weren't closed, it could create a deadlock with the
   * child, we spend some energy here to try to ensure that we
   * either insert all file handles into the dictionary or none
   * at all.  It's a little clumsy with the various popen modes
   * and variable number of files involved.
   */

  /* Child is launched. Close the parents copy of those pipe
   * handles that only the child should have open.  You need to
   * make sure that no handles to the write end of the output pipe
   * are maintained in this process or else the pipe will not close
   * when the child process exits and the ReadFile will hang. */

  if (!CloseHandle(hChildStdinRd))
    return win32_error("CloseHandle", NULL);
          
  if (!CloseHandle(hChildStdoutWr))
    return win32_error("CloseHandle", NULL);
          
  if ((n != 4) && (!CloseHandle(hChildStderrWr)))
    return win32_error("CloseHandle", NULL);
  
  this->m_ProcessHandle = hProcess;
  if ( fd1 >= 0 )
    {
    //  this->m_StdIn = f1;
    this->m_pStdIn = fd1;
    }
  if ( fd2 >= 0 )
    {
    //  this->m_StdOut = f2;
    this->m_pStdOut = fd2;
    }
  if ( fd3 >= 0 )
    {
    //  this->m_StdErr = f3;
    this->m_pStdErr = fd3;
    }

  return true;
}

/*
 * Wrapper for fclose() to use for popen* files, so we can retrieve the
 * exit code for the child process and return as a result of the close.
 *
 * This function uses the _PyPopenProcs dictionary in order to map the
 * input file pointer to information about the process that was
 * originally created by the popen* call that created the file pointer.
 * The dictionary uses the file pointer as a key (with one entry
 * inserted for each file returned by the original popen* call) and a
 * single list object as the value for all files from a single call.
 * The list object contains the Win32 process handle at [0], and a file
 * count at [1], which is initialized to the total number of file
 * handles using that list.
 *
 * This function closes whichever handle it is passed, and decrements
 * the file count in the dictionary for the process handle pointed to
 * by this file.  On the last close (when the file count reaches zero),
 * this function will wait for the child process and then return its
 * exit code as the result of the close() operation.  This permits the
 * files to be closed in any order - it is always the close() of the
 * final handle that will return the exit code.
 */

 /* RED_FLAG 31-Aug-2000 Tim
  * This is always called (today!) between a pair of
  * Py_BEGIN_ALLOW_THREADS/ Py_END_ALLOW_THREADS
  * macros.  So the thread running this has no valid thread state, as
  * far as Python is concerned.  However, this calls some Python API
  * functions that cannot be called safely without a valid thread
  * state, in particular PyDict_GetItem.
  * As a temporary hack (although it may last for years ...), we
  * *rely* on not having a valid thread state in this function, in
  * order to create our own "from scratch".
  * This will deadlock if _PyPclose is ever called by a thread
  * holding the global lock.
  */

bool cmWin32ProcessExecution::PrivateClose(int /* timeout */)
{
  HANDLE hProcess = this->m_ProcessHandle;

  int result = -1;
  DWORD exit_code;

  std::string output = "";
  bool done = false;
  while(!done)
    {
    Sleep(10);
    bool have_some = false;
    struct _stat fsout;
    struct _stat fserr;
    int rout = _fstat(this->m_pStdOut, &fsout);
    int rerr = _fstat(this->m_pStdErr, &fserr);
    if ( rout && rerr )
      {
      break;
      }
    if (fserr.st_size > 0)
      {
      char buffer[1023];
      int len = read(this->m_pStdErr, buffer, 1023);
      buffer[len] = 0;
      if ( this->m_Verbose )
        {
        std::cout << buffer << std::flush;
        }
      output += buffer;
      have_some = true;
      }
    if (fsout.st_size > 0)
      {
      char buffer[1023];
      int len = read(this->m_pStdOut, buffer, 1023);
      buffer[len] = 0;
      if ( this->m_Verbose )
        {
        std::cout << buffer << std::flush;
        }
      output += buffer;
      have_some = true;
      }
    unsigned long exitCode;
    if ( ! have_some )
      {
      GetExitCodeProcess(hProcess,&exitCode);
      if (exitCode != STILL_ACTIVE)
        {
        break;
        }
      }
    }  

  
  if (WaitForSingleObject(hProcess, INFINITE) != WAIT_FAILED &&
      GetExitCodeProcess(hProcess, &exit_code)) 
    {
    result = exit_code;
    } 
  else 
    {
    /* Indicate failure - this will cause the file object
     * to raise an I/O error and translate the last Win32
     * error code from errno.  We do have a problem with
     * last errors that overlap the normal errno table,
     * but that's a consistent problem with the file object.
     */
    if (result != EOF) 
      {
      /* If the error wasn't from the fclose(), then
       * set errno for the file object error handling.
       */
      errno = GetLastError();
      }
    result = -1;
    }

  /* Free up the native handle at this point */
  CloseHandle(hProcess);
  this->m_ExitValue = result;
  this->m_Output = output;
  if ( result < 0 )
    {
    return false;
    }
  return true;
}

int cmWin32ProcessExecution::Windows9xHack(const char* command)
{
  BOOL bRet;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  DWORD exit_code=0;

  if (!command) 
    {
    cmSystemTools::Error("Windows9xHack: Command not specified");
    return 1;
  }

  /* Make child process use this app's standard files. */
  ZeroMemory(&si, sizeof si);
  si.cb = sizeof si;
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

  
  char * app = 0;
  char* cmd = new char[ strlen(command) + 1 ];
  strcpy(cmd, command);

  bRet = CreateProcess(
    app, cmd,
    NULL, NULL,
    TRUE, 0,
    NULL, NULL,
    &si, &pi
    );
  delete [] cmd;

  if (bRet) 
    {
    if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_FAILED) 
      {
      GetExitCodeProcess(pi.hProcess, &exit_code);
      }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exit_code;
    }

  return 1;
}
