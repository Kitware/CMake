/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include <cm3p/uv.h>

#include "cmDuration.h"
#include "cmProcessOutput.h"
#include "cmUVHandlePtr.h"

class cmCTestRunTest;

/** \class cmProcess
 * \brief run a process with c++
 *
 * cmProcess wraps the kwsys process stuff in a c++ class.
 */
class cmProcess
{
public:
  explicit cmProcess(std::unique_ptr<cmCTestRunTest> runner);
  ~cmProcess();
  void SetCommand(std::string const& command);
  void SetCommandArguments(std::vector<std::string> const& arg);
  void SetWorkingDirectory(std::string const& dir);
  void SetTimeout(cmDuration t) { this->Timeout = t; }
  void ChangeTimeout(cmDuration t);
  void ResetStartTime();
  // Return true if the process starts
  bool StartProcess(uv_loop_t& loop, std::vector<size_t>* affinity);

  enum class TimeoutReason
  {
    Normal,
    StopTime,
  };
  void SetTimeoutReason(TimeoutReason r) { this->TimeoutReason_ = r; }
  TimeoutReason GetTimeoutReason() const { return this->TimeoutReason_; }

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
  int GetId() const { return this->Id; }
  void SetId(int id) { this->Id = id; }
  int64_t GetExitValue() const { return this->ExitValue; }
  cmDuration GetTotalTime() { return this->TotalTime; }

  enum class Exception
  {
    None,
    Fault,
    Illegal,
    Interrupt,
    Numerical,
    Other
  };

  Exception GetExitException() const;
  std::string GetExitExceptionString() const;

  std::unique_ptr<cmCTestRunTest> GetRunner()
  {
    return std::move(this->Runner);
  }

  enum class Termination
  {
    Normal,
    Custom,
    Forced,
  };
  Termination GetTerminationStyle() const { return this->TerminationStyle; }

private:
  cm::optional<cmDuration> Timeout;
  TimeoutReason TimeoutReason_ = TimeoutReason::Normal;
  std::chrono::steady_clock::time_point StartTime;
  cmDuration TotalTime;
  bool ReadHandleClosed = false;
  bool ProcessHandleClosed = false;

  cm::uv_process_ptr Process;
  cm::uv_pipe_ptr PipeReader;
  cm::uv_timer_ptr Timer;
  std::vector<char> Buf;

  std::unique_ptr<cmCTestRunTest> Runner;
  cmProcessOutput Conv;
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
  void Finish();

  class Buffer : public std::vector<char>
  {
    // Half-open index range of partial line already scanned.
    size_type First = 0;
    size_type Last = 0;

  public:
    Buffer() = default;
    bool GetLine(std::string& line);
    bool GetLast(std::string& line);
  };
  Buffer Output;
  std::string Command;
  std::string WorkingDirectory;
  std::vector<std::string> Arguments;
  std::vector<const char*> ProcessArgs;
  int Id;
  int64_t ExitValue;
  Termination TerminationStyle = Termination::Normal;
};
