/*
    ninja's subprocess.h
*/

// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NINJA_SUBPROCESS_H_
#define NINJA_SUBPROCESS_H_

#include <string>
#include <vector>
#include <queue>
#include <cstdio>
#include <algorithm>


#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif


#if defined(_WIN64)
typedef unsigned __int64 cmULONG_PTR;
#else
typedef unsigned long cmULONG_PTR;
#endif

//#include "exit_status.h"
enum ExitStatus {
  ExitSuccess,
  ExitFailure,
  ExitInterrupted
};

/// Subprocess wraps a single async subprocess.  It is entirely
/// passive: it expects the caller to notify it when its fds are ready
/// for reading, as well as call Finish() to reap the child once done()
/// is true.
struct Subprocess {
  ~Subprocess();

  /// Returns ExitSuccess on successful process exit, ExitInterrupted if
  /// the process was interrupted, ExitFailure if it otherwise failed.
  ExitStatus Finish();

  bool Done() const;

  const std::string& GetOutput() const;

  int ExitCode() const { return exit_code_; }

 private:
  Subprocess();
  bool Start(struct SubprocessSet* set, const std::string& command,
                                        const std::string& dir);
  void OnPipeReady();

  std::string buf_;

#ifdef _WIN32
  /// Set up pipe_ as the parent-side pipe of the subprocess; return the
  /// other end of the pipe, usable in the child process.
  HANDLE SetupPipe(HANDLE ioport);

  PROCESS_INFORMATION child_;
  HANDLE pipe_;
  OVERLAPPED overlapped_;
  char overlapped_buf_[4 << 10];
  bool is_reading_;
  int exit_code_;
#else
  int fd_;
  pid_t pid_;
#endif

  friend struct SubprocessSet;
};

/// SubprocessSet runs a ppoll/pselect() loop around a set of Subprocesses.
/// DoWork() waits for any state change in subprocesses; finished_
/// is a queue of subprocesses as they finish.
struct SubprocessSet {
  SubprocessSet();
  ~SubprocessSet();

  Subprocess* Add(const std::string& command, const std::string& dir);
  bool DoWork();
  Subprocess* NextFinished();
  void Clear();

  std::vector<Subprocess*> running_;
  std::queue<Subprocess*> finished_;

#ifdef _WIN32
  static BOOL WINAPI NotifyInterrupted(DWORD dwCtrlType);
  static HANDLE ioport_;
#else
  static void SetInterruptedFlag(int signum);
  static bool interrupted_;

  struct sigaction old_act_;
  sigset_t old_mask_;
#endif
};

#endif // NINJA_SUBPROCESS_H_


/*
    ninja's util functions
*/


static void Fatal(const char* msg, ...) {
  va_list ap;
  fprintf(stderr, "ninja: FATAL: ");
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
#ifdef _WIN32
  // On Windows, some tools may inject extra threads.
  // exit() may block on locks held by those threads, so forcibly exit.
  fflush(stderr);
  fflush(stdout);
  ExitProcess(1);
#else
  exit(1);
#endif
}


#ifdef _WIN32
std::string GetLastErrorString() {
  DWORD err = GetLastError();

  char* msg_buf;
  FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&msg_buf,
        0,
        NULL);
  std::string msg = msg_buf;
  LocalFree(msg_buf);
  return msg;
}
#endif

#define snprintf _snprintf


/*
    ninja's subprocess-win32.cc
*/

// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//#include "subprocess.h"

#include <stdio.h>

#include <algorithm>

//#include "util.h"

namespace {

void Win32Fatal(const char* function) {
  Fatal("%s: %s", function, GetLastErrorString().c_str());
}

}  // anonymous namespace

Subprocess::Subprocess() : overlapped_(), is_reading_(false),
                           exit_code_(1) {
  child_.hProcess = NULL;
}

Subprocess::~Subprocess() {
  if (pipe_) {
    if (!CloseHandle(pipe_))
      Win32Fatal("CloseHandle");
  }
  // Reap child if forgotten.
  if (child_.hProcess)
    Finish();
}

HANDLE Subprocess::SetupPipe(HANDLE ioport) {
  char pipe_name[100];
  snprintf(pipe_name, sizeof(pipe_name),
           "\\\\.\\pipe\\ninja_pid%u_sp%p", GetCurrentProcessId(), this);

  pipe_ = ::CreateNamedPipeA(pipe_name,
                             PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                             PIPE_TYPE_BYTE,
                             PIPE_UNLIMITED_INSTANCES,
                             0, 0, INFINITE, NULL);
  if (pipe_ == INVALID_HANDLE_VALUE)
    Win32Fatal("CreateNamedPipe");

  if (!CreateIoCompletionPort(pipe_, ioport, (cmULONG_PTR)this, 0))
    Win32Fatal("CreateIoCompletionPort");

  memset(&overlapped_, 0, sizeof(overlapped_));
  if (!ConnectNamedPipe(pipe_, &overlapped_) &&
      GetLastError() != ERROR_IO_PENDING) {
    Win32Fatal("ConnectNamedPipe");
  }

  // Get the write end of the pipe as a handle inheritable across processes.
  HANDLE output_write_handle = CreateFile(pipe_name, GENERIC_WRITE, 0,
                                          NULL, OPEN_EXISTING, 0, NULL);
  HANDLE output_write_child;
  if (!DuplicateHandle(GetCurrentProcess(), output_write_handle,
                       GetCurrentProcess(), &output_write_child,
                       0, TRUE, DUPLICATE_SAME_ACCESS)) {
    Win32Fatal("DuplicateHandle");
  }
  CloseHandle(output_write_handle);

  return output_write_child;
}

bool Subprocess::Start(SubprocessSet* set, const std::string& command,
                                           const std::string& dir) {
  HANDLE child_pipe = SetupPipe(set->ioport_);

  SECURITY_ATTRIBUTES security_attributes;
  memset(&security_attributes, 0, sizeof(SECURITY_ATTRIBUTES));
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  security_attributes.bInheritHandle = TRUE;
  // Must be inheritable so subprocesses can dup to children.
  HANDLE nul = CreateFile("NUL", GENERIC_READ,
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
          &security_attributes, OPEN_EXISTING, 0, NULL);
  if (nul == INVALID_HANDLE_VALUE)
    Fatal("couldn't open nul");

  STARTUPINFOA startup_info;
  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(STARTUPINFO);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = nul;
  startup_info.hStdOutput = child_pipe;
  startup_info.hStdError = child_pipe;

  PROCESS_INFORMATION process_info;
  memset(&process_info, 0, sizeof(process_info));

  // Do not prepend 'cmd /c' on Windows, this breaks command
  // lines greater than 8,191 chars.
  if (!CreateProcessA(NULL, (char*)command.c_str(), NULL, NULL,
                      /* inherit handles */ TRUE, CREATE_NEW_PROCESS_GROUP,
                      NULL, (dir.empty() ? NULL : dir.c_str()),
                      &startup_info, &process_info)) {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) {
      // file (program) not found error is treated
      // as a normal build action failure
      if (child_pipe)
        CloseHandle(child_pipe);
      CloseHandle(pipe_);
      CloseHandle(nul);
      pipe_ = NULL;
      // child_ is already NULL;
      buf_ =
       "CreateProcess failed: The system cannot find the file specified.\n";
      return true;
    } else {
      Win32Fatal("CreateProcess");    // pass all other errors to Win32Fatal
    }
  }

  // Close pipe channel only used by the child.
  if (child_pipe)
    CloseHandle(child_pipe);
  CloseHandle(nul);

  CloseHandle(process_info.hThread);
  child_ = process_info;

  return true;
}

void Subprocess::OnPipeReady() {
  DWORD bytes;
  if (!GetOverlappedResult(pipe_, &overlapped_, &bytes, TRUE)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) {
      CloseHandle(pipe_);
      pipe_ = NULL;
      return;
    }
    Win32Fatal("GetOverlappedResult");
  }

  if (is_reading_ && bytes)
    buf_.append(overlapped_buf_, bytes);

  memset(&overlapped_, 0, sizeof(overlapped_));
  is_reading_ = true;
  if (!::ReadFile(pipe_, overlapped_buf_, sizeof(overlapped_buf_),
                  &bytes, &overlapped_)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) {
      CloseHandle(pipe_);
      pipe_ = NULL;
      return;
    }
    if (GetLastError() != ERROR_IO_PENDING)
      Win32Fatal("ReadFile");
  }

  // Even if we read any bytes in the readfile call, we'll enter this
  // function again later and get them at that point.
}

ExitStatus Subprocess::Finish() {
  if (!child_.hProcess)
    return ExitFailure;

  // TODO: add error handling for all of these.
  WaitForSingleObject(child_.hProcess, INFINITE);

  DWORD exit_code = 0;
  GetExitCodeProcess(child_.hProcess, &exit_code);

  CloseHandle(child_.hProcess);
  child_.hProcess = NULL;
  exit_code_ = exit_code;
  return exit_code == 0              ? ExitSuccess :
         exit_code == CONTROL_C_EXIT ? ExitInterrupted :
                                       ExitFailure;
}

bool Subprocess::Done() const {
  return pipe_ == NULL;
}

const std::string& Subprocess::GetOutput() const {
  return buf_;
}

HANDLE SubprocessSet::ioport_;

SubprocessSet::SubprocessSet() {
  ioport_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  if (!ioport_)
    Win32Fatal("CreateIoCompletionPort");
  if (!SetConsoleCtrlHandler(NotifyInterrupted, TRUE))
    Win32Fatal("SetConsoleCtrlHandler");
}

SubprocessSet::~SubprocessSet() {
  Clear();

  SetConsoleCtrlHandler(NotifyInterrupted, FALSE);
  CloseHandle(ioport_);
}

BOOL WINAPI SubprocessSet::NotifyInterrupted(DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
    if (!PostQueuedCompletionStatus(ioport_, 0, 0, NULL))
      Win32Fatal("PostQueuedCompletionStatus");
    return TRUE;
  }

  return FALSE;
}

Subprocess *SubprocessSet::Add(const std::string& command,
                               const std::string& dir) {
  Subprocess *subprocess = new Subprocess;
  if (!subprocess->Start(this, command, dir)) {
    delete subprocess;
    return 0;
  }
  if (subprocess->child_.hProcess)
    running_.push_back(subprocess);
  else
    finished_.push(subprocess);
  return subprocess;
}

bool SubprocessSet::DoWork() {
  DWORD bytes_read;
  Subprocess* subproc;
  OVERLAPPED* overlapped;

  if (!GetQueuedCompletionStatus(ioport_, &bytes_read, (cmULONG_PTR*)&subproc,
                                 &overlapped, INFINITE)) {
    if (GetLastError() != ERROR_BROKEN_PIPE)
      Win32Fatal("GetQueuedCompletionStatus");
  }

  if (!subproc) // A NULL subproc indicates that we were interrupted and is
                // delivered by NotifyInterrupted above.
    return true;

  subproc->OnPipeReady();

  if (subproc->Done()) {
    std::vector<Subprocess*>::iterator end =
        std::remove(running_.begin(), running_.end(), subproc);
    if (running_.end() != end) {
      finished_.push(subproc);
      running_.resize(end - running_.begin());
    }
  }

  return false;
}

Subprocess* SubprocessSet::NextFinished() {
  if (finished_.empty())
    return NULL;
  Subprocess* subproc = finished_.front();
  finished_.pop();
  return subproc;
}

void SubprocessSet::Clear() {
  std::vector<Subprocess*>::iterator it = running_.begin();
  for (; it != running_.end(); ++it) {
    if ((*it)->child_.hProcess) {
      if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,
                                    (*it)->child_.dwProcessId))
        Win32Fatal("GenerateConsoleCtrlEvent");
      }
  }
  it = running_.begin();
  for (; it != running_.end(); ++it)
    delete *it;
  running_.clear();
}


// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// Wrapper around cl that adds /showIncludes to command line, and uses that to
// generate .d files that match the style from gcc -MD.
//
// /showIncludes is equivalent to -MD, not -MMD, that is, system headers are
// included.


#include <windows.h>
#include <sstream>
//#include "subprocess.h"
//#include "util.h"

// We don't want any wildcard expansion.
// See http://msdn.microsoft.com/en-us/library/zay8tzh6(v=vs.85).aspx
void _setargv() {}

static void usage(const char* msg) {
  Fatal("%s\n\nusage:\n    "
          "cmcldeps "
          "<language C, CXX or RC>  "
          "<source file path>  "
          "<output path for *.d file>  "
          "<output path for *.obj file>  "
          "<prefix of /showIncludes>  "
          "<path to cl.exe>  "
          "<path to tool (cl or rc)>  "
          "<rest of command ...>\n", msg);
}

static std::string trimLeadingSpace(const std::string& cmdline) {
  int i = 0;
  for (; cmdline[i] == ' '; ++i)
    ;
  return cmdline.substr(i);
}

static void doEscape(std::string& str, const std::string& search,
                                       const std::string& repl) {
  std::string::size_type pos = 0;
  while ((pos = str.find(search, pos)) != std::string::npos) {
    str.replace(pos, search.size(), repl);
    pos += repl.size();
  }
}

// Strips one argument from the cmdline and returns it. "surrounding quotes"
// are removed from the argument if there were any.
static std::string getArg(std::string& cmdline) {
  std::string ret;
  bool in_quoted = false;
  unsigned int i = 0;

  cmdline = trimLeadingSpace(cmdline);

  for (;; ++i) {
    if (i >= cmdline.size())
      usage("Couldn't parse arguments.");
    if (!in_quoted && cmdline[i] == ' ')
      break; // "a b" "x y"
    if (cmdline[i] == '"')
      in_quoted = !in_quoted;
  }

  ret = cmdline.substr(0, i);
  if (ret[0] == '"' && ret[i - 1] == '"')
    ret = ret.substr(1, ret.size() - 2);
  cmdline = cmdline.substr(i);
  return ret;
}

static void parseCommandLine(LPTSTR wincmdline,
                             std::string& lang,
                             std::string& srcfile,
                             std::string& dfile,
                             std::string& objfile,
                             std::string& prefix,
                             std::string& clpath,
                             std::string& binpath,
                             std::string& rest) {
  std::string cmdline(wincmdline);
  /* self */ getArg(cmdline);
  lang = getArg(cmdline);
  srcfile = getArg(cmdline);
  dfile = getArg(cmdline);
  objfile = getArg(cmdline);
  prefix = getArg(cmdline);
  clpath = getArg(cmdline);
  binpath = getArg(cmdline);
  rest = trimLeadingSpace(cmdline);
}

static void outputDepFile(const std::string& dfile, const std::string& objfile,
        std::vector<std::string>& incs) {

  if (dfile.empty())
    return;

  // strip duplicates
  std::sort(incs.begin(), incs.end());
  incs.erase(std::unique(incs.begin(), incs.end()), incs.end());

  FILE* out = fopen(dfile.c_str(), "wb");

  // FIXME should this be fatal or not? delete obj? delete d?
  if (!out)
    return;

  std::string tmp = objfile;
  doEscape(tmp, " ", "\\ ");
  fprintf(out, "%s: \\\n", tmp.c_str());

  std::vector<std::string>::iterator it = incs.begin();
  for (; it != incs.end(); ++it) {
    tmp = *it;
    doEscape(tmp, "\\", "/");
    doEscape(tmp, " ", "\\ ");
    fprintf(out, "%s \\\n", tmp.c_str());
  }

  fprintf(out, "\n");
  fclose(out);
}


bool startsWith(const std::string& str, const std::string& what) {
  return str.compare(0, what.size(), what) == 0;
}

bool contains(const std::string& str, const std::string& what) {
  return str.find(what) != std::string::npos;
}

std::string replace(const std::string& str, const std::string& what,
                    const std::string& replacement) {
  size_t pos = str.find(what);
  if (pos == std::string::npos)
    return str;
  std::string replaced = str;
  return replaced.replace(pos, what.size(), replacement);
}



static int process( const std::string& srcfilename,
                    const std::string& dfile,
                    const std::string& objfile,
                    const std::string& prefix,
                    const std::string& cmd,
                    const std::string& dir = "",
                    bool quiet = false) {

  SubprocessSet subprocs;
  Subprocess* subproc = subprocs.Add(cmd, dir);

  if(!subproc)
    return 2;

  while ((subproc = subprocs.NextFinished()) == NULL) {
    subprocs.DoWork();
  }

  bool success = subproc->Finish() == ExitSuccess;
  int exit_code = subproc->ExitCode();

  std::string output = subproc->GetOutput();
  delete subproc;

  // process the include directives and output everything else
  std::stringstream ss(output);
  std::string line;
  std::vector<std::string> includes;
  bool isFirstLine = true; // cl prints always first the source filename
  while (std::getline(ss, line)) {
    if (startsWith(line, prefix)) {
      std::string inc = trimLeadingSpace(line.substr(prefix.size()).c_str());
      if (inc[inc.size() - 1] == '\r') // blech, stupid \r\n
        inc = inc.substr(0, inc.size() - 1);
      includes.push_back(inc);
    } else {
      if (!isFirstLine || !startsWith(line, srcfilename)) {
        if (!quiet) {
          fprintf(stdout, "%s\n", line.c_str());
        }
      } else {
        isFirstLine = false;
      }
    }
  }

  if (!success) {
    return exit_code;
  }

  // don't update .d until/unless we succeed compilation
  outputDepFile(dfile, objfile, includes);

  return 0;
}


int main() {

  // Use the Win32 api instead of argc/argv so we can avoid interpreting the
  // rest of command line after the .d and .obj. Custom parsing seemed
  // preferable to the ugliness you get into in trying to re-escape quotes for
  // subprocesses, so by avoiding argc/argv, the subprocess is called with
  // the same command line verbatim.

  std::string lang, srcfile, dfile, objfile, prefix, cl, binpath, rest;
  parseCommandLine(GetCommandLine(), lang, srcfile, dfile, objfile,
                                     prefix, cl, binpath, rest);

  // needed to suppress filename output of msvc tools
  std::string srcfilename;
  std::string::size_type pos = srcfile.rfind("\\");
  if (pos != std::string::npos) {
    srcfilename = srcfile.substr(pos + 1);
  }

  std::string nol = " /nologo ";
  std::string show = " /showIncludes ";
  if (lang == "C" || lang == "CXX") {
    return process(srcfilename, dfile, objfile, prefix,
                   binpath + nol + show + rest);
  } else if (lang == "RC") {
    // "misuse" cl.exe to get headers from .rc files

    std::string clrest = rest;
    // rc: /fo x.dir\x.rc.res  ->  cl: /out:x.dir\x.rc.res.dep.obj
    clrest = replace(clrest, "/fo", "/out:");
    clrest = replace(clrest, objfile, objfile + ".dep.obj ");
    // rc: src\x\x.rc  ->  cl: /Tc src\x\x.rc
    clrest = replace(clrest, srcfile, "/Tc " + srcfile);

    cl = "\"" + cl + "\" /P /DRC_INVOKED ";

    // call cl in object dir so the .i is generated there
    std::string objdir;
    std::string::size_type pos = objfile.rfind("\\");
    if (pos != std::string::npos) {
      objdir = objfile.substr(0, pos);
    }

    // extract dependencies with cl.exe
    process(srcfilename, dfile, objfile,
                  prefix, cl + nol + show + clrest, objdir, true);

    // compile rc file with rc.exe
    return process(srcfilename, "" , objfile, prefix, binpath + " " + rest);
  }

  usage("Invalid language specified.");
  return 1;
}
