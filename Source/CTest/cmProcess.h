/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmProcess_h
#define cmProcess_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmUVHandlePtr.h"
#include "cm_uv.h"

#include <chrono>
#include <stddef.h>
#include <string>
#include <sys/types.h>
#include <vector>

class cmCTestRunTest;

/** \class cmProcess
 * \brief run a process with c++
 *
 * cmProcess wraps the kwsys process stuff in a c++ class.
 */
class cmProcess
{
public:
  explicit cmProcess(cmCTestRunTest& runner);
  ~cmProcess();
  const char* GetCommand() { return this->Command.c_str(); }
  void SetCommand(const char* command);
  void SetCommandArguments(std::vector<std::string> const& arg);
  void SetWorkingDirectory(const char* dir) { this->WorkingDirectory = dir; }
  void SetTimeout(std::chrono::duration<double> t) { this->Timeout = t; }
  void ChangeTimeout(std::chrono::duration<double> t);
  void ResetStartTime();
  // Return true if the process starts
  bool StartProcess(uv_loop_t& loop);

  enum class State
  {
    Starting,
    Error,
    Exception,
    Executing,
    Exited,
    Expired,
    Killed,
    Disowned
  };

  State GetProcessStatus();
  int GetId() { return this->Id; }
  void SetId(int id) { this->Id = id; }
  int GetExitValue() { return this->ExitValue; }
  std::chrono::duration<double> GetTotalTime() { return this->TotalTime; }

  enum class Exception
  {
    None,
    Fault,
    Illegal,
    Interrupt,
    Numerical,
    Other
  };

  Exception GetExitException();
  std::string GetExitExceptionString();

private:
  std::chrono::duration<double> Timeout;
  std::chrono::steady_clock::time_point StartTime;
  std::chrono::duration<double> TotalTime;
  bool ReadHandleClosed = false;
  bool ProcessHandleClosed = false;

  cm::uv_process_ptr Process;
  cm::uv_pipe_ptr PipeReader;
  cm::uv_timer_ptr Timer;
  std::vector<char> Buf;

  cmCTestRunTest& Runner;
  int Signal = 0;
  cmProcess::State ProcessState = cmProcess::State::Starting;

  static void OnExitCB(uv_process_t* process, int64_t exit_status,
                       int term_signal);
  static void OnTimeoutCB(uv_timer_t* timer);
  static void OnReadCB(uv_stream_t* stream, ssize_t nread,
                       const uv_buf_t* buf);
  static void OnAllocateCB(uv_handle_t* handle, size_t suggested_size,
                           uv_buf_t* buf);

  void OnExit(int64_t exit_status, int term_signal);
  void OnTimeout();
  void OnRead(ssize_t nread, const uv_buf_t* buf);
  void OnAllocate(size_t suggested_size, uv_buf_t* buf);

  void StartTimer();

  class Buffer : public std::vector<char>
  {
    // Half-open index range of partial line already scanned.
    size_type First;
    size_type Last;

  public:
    Buffer()
      : First(0)
      , Last(0)
    {
    }
    bool GetLine(std::string& line);
    bool GetLast(std::string& line);
  };
  Buffer Output;
  std::string Command;
  std::string WorkingDirectory;
  std::vector<std::string> Arguments;
  std::vector<const char*> ProcessArgs;
  int Id;
  int ExitValue;
};

#endif
