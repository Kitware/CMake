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
using namespace std;

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
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

  const string& GetOutput() const;

 private:
  Subprocess();
  bool Start(struct SubprocessSet* set, const string& command);
  void OnPipeReady();

  string buf_;

#ifdef _WIN32
  /// Set up pipe_ as the parent-side pipe of the subprocess; return the
  /// other end of the pipe, usable in the child process.
  HANDLE SetupPipe(HANDLE ioport);

  HANDLE child_;
  HANDLE pipe_;
  OVERLAPPED overlapped_;
  char overlapped_buf_[4 << 10];
  bool is_reading_;
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

  Subprocess* Add(const string& command);
  bool DoWork();
  Subprocess* NextFinished();
  void Clear();

  vector<Subprocess*> running_;
  queue<Subprocess*> finished_;

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
string GetLastErrorString() {
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
  string msg = msg_buf;
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

Subprocess::Subprocess() : child_(NULL) , overlapped_(), is_reading_(false) {
}

Subprocess::~Subprocess() {
  if (pipe_) {
    if (!CloseHandle(pipe_))
      Win32Fatal("CloseHandle");
  }
  // Reap child if forgotten.
  if (child_)
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

  if (!CreateIoCompletionPort(pipe_, ioport, (ULONG_PTR)this, 0))
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

bool Subprocess::Start(SubprocessSet* set, const string& command) {
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
                      NULL, NULL,
                      &startup_info, &process_info)) {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) { // file (program) not found error is treated as a normal build action failure
      if (child_pipe)
        CloseHandle(child_pipe);
      CloseHandle(pipe_);
      CloseHandle(nul);
      pipe_ = NULL;
      // child_ is already NULL;
      buf_ = "CreateProcess failed: The system cannot find the file specified.\n";
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
  child_ = process_info.hProcess;

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
  if (!child_)
    return ExitFailure;

  // TODO: add error handling for all of these.
  WaitForSingleObject(child_, INFINITE);

  DWORD exit_code = 0;
  GetExitCodeProcess(child_, &exit_code);

  CloseHandle(child_);
  child_ = NULL;

  return exit_code == 0              ? ExitSuccess :
         exit_code == CONTROL_C_EXIT ? ExitInterrupted :
                                       ExitFailure;
}

bool Subprocess::Done() const {
  return pipe_ == NULL;
}

const string& Subprocess::GetOutput() const {
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

Subprocess *SubprocessSet::Add(const string& command) {
  Subprocess *subprocess = new Subprocess;
  if (!subprocess->Start(this, command)) {
    delete subprocess;
    return 0;
  }
  if (subprocess->child_)
    running_.push_back(subprocess);
  else
    finished_.push(subprocess);
  return subprocess;
}

bool SubprocessSet::DoWork() {
  DWORD bytes_read;
  Subprocess* subproc;
  OVERLAPPED* overlapped;

  if (!GetQueuedCompletionStatus(ioport_, &bytes_read, (PULONG_PTR)&subproc,
                                 &overlapped, INFINITE)) {
    if (GetLastError() != ERROR_BROKEN_PIPE)
      Win32Fatal("GetQueuedCompletionStatus");
  }

  if (!subproc) // A NULL subproc indicates that we were interrupted and is
                // delivered by NotifyInterrupted above.
    return true;

  subproc->OnPipeReady();

  if (subproc->Done()) {
    vector<Subprocess*>::iterator end =
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
  for (vector<Subprocess*>::iterator i = running_.begin();
       i != running_.end(); ++i) {
    if ((*i)->child_)
      if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, GetProcessId((*i)->child_)))
        Win32Fatal("GenerateConsoleCtrlEvent");
  }
  for (vector<Subprocess*>::iterator i = running_.begin();
       i != running_.end(); ++i)
    delete *i;
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
  Fatal("%s\n\nusage:\n"
          "  cldeps "
          "<source file> "
          "<output path for *.d file> "
          "<output path for *.obj file> "
          "<prefix of /showIncludes> "
          "<path to cl> "
          "<rest of command ...>\n", msg);
}

static string trimLeadingSpace(const string& cmdline) {
  int i = 0;
  for (; cmdline[i] == ' '; ++i)
    ;
  return cmdline.substr(i);
}

static void doEscape(string& str, const string& search, const string& repl) {
  string::size_type pos = 0;
  while ((pos = str.find(search, pos)) != string::npos) {
    str.replace(pos, search.size(), repl);
    pos += repl.size();
  }
}

// Strips one argument from the cmdline and returns it. "surrounding quotes"
// are removed from the argument if there were any.
static string getArg(string& cmdline) {
  string ret;
  bool in_quoted = false;
  unsigned int i = 0;

  cmdline = trimLeadingSpace(cmdline);

  for (;; ++i) {
    if (i >= cmdline.size())
      usage("Couldn't parse arguments.");
    if (!in_quoted && cmdline[i] == ' ')
      break;
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
        string& srcfile, string& dfile, string& objfile, string& prefix, string& clpath, string& rest) {
  string cmdline(wincmdline);
  /* self */ getArg(cmdline);
  srcfile = getArg(cmdline);
  std::string::size_type pos = srcfile.rfind("\\");
  if (pos != string::npos) {
    srcfile = srcfile.substr(pos + 1);
  } else {
    srcfile = "";
  }
  dfile = getArg(cmdline);
  objfile = getArg(cmdline);
  prefix = getArg(cmdline);
  clpath = getArg(cmdline);
  rest = trimLeadingSpace(cmdline);
}

static void outputDepFile(const string& dfile, const string& objfile,
        vector<string>& incs) {

  // strip duplicates
  sort(incs.begin(), incs.end());
  incs.erase(unique(incs.begin(), incs.end()), incs.end());

  FILE* out = fopen(dfile.c_str(), "wb");

  // FIXME should this be fatal or not? delete obj? delete d?
  if (!out)
    return;

  fprintf(out, "%s: \\\n", objfile.c_str());
  for (vector<string>::iterator i(incs.begin()); i != incs.end(); ++i) {
    string tmp = *i;
    doEscape(tmp, "\\", "\\\\");
    doEscape(tmp, " ", "\\ ");
    //doEscape(tmp, "(", "("); // TODO ninja cant read ( and )
    //doEscape(tmp, ")", ")");
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

int main() {

  // Use the Win32 api instead of argc/argv so we can avoid interpreting the
  // rest of command line after the .d and .obj. Custom parsing seemed
  // preferable to the ugliness you get into in trying to re-escape quotes for
  // subprocesses, so by avoiding argc/argv, the subprocess is called with
  // the same command line verbatim.

  string srcfile, dfile, objfile, prefix, clpath, rest;
  parseCommandLine(GetCommandLine(), srcfile, dfile, objfile, prefix, clpath, rest);

  //fprintf(stderr, "D: %s\n", dfile.c_str());
  //fprintf(stderr, "OBJ: %s\n", objfile.c_str());
  //fprintf(stderr, "CL: %s\n", clpath.c_str());
  //fprintf(stderr, "REST: %s\n", rest.c_str());

  SubprocessSet subprocs;
  Subprocess* subproc = subprocs.Add(clpath + " /showIncludes " + rest);
  if(!subproc)
    return 2;

  while ((subproc = subprocs.NextFinished()) == NULL) {
    subprocs.DoWork();
  }

  bool success = subproc->Finish() == ExitSuccess;
  string output = subproc->GetOutput();

  delete subproc;

  // process the include directives and output everything else
  stringstream ss(output);
  string line;
  vector<string> includes;
  bool isFirstLine = true; // cl prints always first the source filename
  std::string sysHeadersCamel =    "Program Files (x86)\\Microsoft ";
  std::string sysHeadersLower = "program files (x86)\\microsoft ";
  while (getline(ss, line)) {
    if (startsWith(line, prefix)) {
       if (!contains(line, sysHeadersCamel) && !contains(line, sysHeadersLower)) {
        string inc = trimLeadingSpace(line.substr(prefix.size()).c_str());
        if (inc[inc.size() - 1] == '\r') // blech, stupid \r\n
          inc = inc.substr(0, inc.size() - 1);
        includes.push_back(inc);
       }
    } else {
      if (!isFirstLine || !startsWith(line, srcfile)) {
        fprintf(stdout, "%s\n", line.c_str());
      } else {
        isFirstLine = false;
      }
    }
  }

  if (!success)
    return 3;

  // don't update .d until/unless we succeed compilation
  outputDepFile(dfile, objfile, includes);

  return 0;
}
