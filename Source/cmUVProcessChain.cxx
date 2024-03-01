/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUVProcessChain.h"

#include <array>
#include <csignal>
#include <cstdio>
#include <istream> // IWYU pragma: keep
#include <utility>

#include <cm/memory>

#include <cm3p/uv.h>

#include "cm_fileno.hxx"

#include "cmGetPipes.h"
#include "cmUVHandlePtr.h"

struct cmUVProcessChain::InternalData
{
  struct StreamData
  {
    int BuiltinStream = -1;
    uv_stdio_container_t Stdio;
  };

  struct ProcessData
  {
    cmUVProcessChain::InternalData* Data;
    cm::uv_process_ptr Process;
    cm::uv_pipe_ptr InputPipe;
    cm::uv_pipe_ptr OutputPipe;
    Status ProcessStatus;

    void Finish();
  };

  const cmUVProcessChainBuilder* Builder = nullptr;

  bool Valid = false;

  cm::uv_loop_ptr Loop;

  StreamData InputStreamData;
  StreamData OutputStreamData;
  StreamData ErrorStreamData;
  cm::uv_pipe_ptr TempOutputPipe;
  cm::uv_pipe_ptr TempErrorPipe;

  unsigned int ProcessesCompleted = 0;
  std::vector<std::unique_ptr<ProcessData>> Processes;

  bool Prepare(const cmUVProcessChainBuilder* builder);
  void SpawnProcess(
    std::size_t index,
    const cmUVProcessChainBuilder::ProcessConfiguration& config, bool first,
    bool last);
  void Finish();
};

cmUVProcessChainBuilder::cmUVProcessChainBuilder() = default;

cmUVProcessChainBuilder& cmUVProcessChainBuilder::AddCommand(
  const std::vector<std::string>& arguments)
{
  if (!arguments.empty()) {
    this->Processes.emplace_back();
    this->Processes.back().Arguments = arguments;
  }
  return *this;
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetNoStream(Stream stdio)
{
  switch (stdio) {
    case Stream_INPUT:
    case Stream_OUTPUT:
    case Stream_ERROR: {
      auto& streamData = this->Stdio[stdio];
      streamData.Type = None;
      break;
    }
  }
  return *this;
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetBuiltinStream(
  Stream stdio)
{
  switch (stdio) {
    case Stream_INPUT:
      // FIXME
      break;

    case Stream_OUTPUT:
    case Stream_ERROR: {
      auto& streamData = this->Stdio[stdio];
      streamData.Type = Builtin;
      break;
    }
  }
  return *this;
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetExternalStream(
  Stream stdio, int fd)
{
  switch (stdio) {
    case Stream_INPUT:
    case Stream_OUTPUT:
    case Stream_ERROR: {
      auto& streamData = this->Stdio[stdio];
      streamData.Type = External;
      streamData.FileDescriptor = fd;
      break;
    }
  }
  return *this;
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetExternalStream(
  Stream stdio, FILE* stream)
{
  int fd = cm_fileno(stream);
  if (fd >= 0) {
    return this->SetExternalStream(stdio, fd);
  }
  return this->SetNoStream(stdio);
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetMergedBuiltinStreams()
{
  this->MergedBuiltinStreams = true;
  return this->SetBuiltinStream(Stream_OUTPUT).SetBuiltinStream(Stream_ERROR);
}

cmUVProcessChainBuilder& cmUVProcessChainBuilder::SetWorkingDirectory(
  std::string dir)
{
  this->WorkingDirectory = std::move(dir);
  return *this;
}

cmUVProcessChain cmUVProcessChainBuilder::Start() const
{
  cmUVProcessChain chain;

  if (!chain.Data->Prepare(this)) {
    return chain;
  }

  for (std::size_t i = 0; i < this->Processes.size(); i++) {
    chain.Data->SpawnProcess(i, this->Processes[i], i == 0,
                             i == this->Processes.size() - 1);
  }

  chain.Data->Finish();

  return chain;
}

bool cmUVProcessChain::InternalData::Prepare(
  const cmUVProcessChainBuilder* builder)
{
  this->Builder = builder;

  auto const& input =
    this->Builder->Stdio[cmUVProcessChainBuilder::Stream_INPUT];
  auto& inputData = this->InputStreamData;
  switch (input.Type) {
    case cmUVProcessChainBuilder::None:
      inputData.Stdio.flags = UV_IGNORE;
      break;

    case cmUVProcessChainBuilder::Builtin: {
      // FIXME
      break;
    }

    case cmUVProcessChainBuilder::External:
      inputData.Stdio.flags = UV_INHERIT_FD;
      inputData.Stdio.data.fd = input.FileDescriptor;
      break;
  }

  auto const& error =
    this->Builder->Stdio[cmUVProcessChainBuilder::Stream_ERROR];
  auto& errorData = this->ErrorStreamData;
  switch (error.Type) {
    case cmUVProcessChainBuilder::None:
      errorData.Stdio.flags = UV_IGNORE;
      break;

    case cmUVProcessChainBuilder::Builtin: {
      int pipeFd[2];
      if (cmGetPipes(pipeFd) < 0) {
        return false;
      }

      errorData.BuiltinStream = pipeFd[0];
      errorData.Stdio.flags = UV_INHERIT_FD;
      errorData.Stdio.data.fd = pipeFd[1];

      if (this->TempErrorPipe.init(*this->Loop, 0) < 0) {
        return false;
      }
      if (uv_pipe_open(this->TempErrorPipe, errorData.Stdio.data.fd) < 0) {
        return false;
      }

      break;
    }

    case cmUVProcessChainBuilder::External:
      errorData.Stdio.flags = UV_INHERIT_FD;
      errorData.Stdio.data.fd = error.FileDescriptor;
      break;
  }

  auto const& output =
    this->Builder->Stdio[cmUVProcessChainBuilder::Stream_OUTPUT];
  auto& outputData = this->OutputStreamData;
  switch (output.Type) {
    case cmUVProcessChainBuilder::None:
      outputData.Stdio.flags = UV_IGNORE;
      break;

    case cmUVProcessChainBuilder::Builtin:
      if (this->Builder->MergedBuiltinStreams) {
        outputData.BuiltinStream = errorData.BuiltinStream;
        outputData.Stdio.flags = UV_INHERIT_FD;
        outputData.Stdio.data.fd = errorData.Stdio.data.fd;
      } else {
        int pipeFd[2];
        if (cmGetPipes(pipeFd) < 0) {
          return false;
        }

        outputData.BuiltinStream = pipeFd[0];
        outputData.Stdio.flags = UV_INHERIT_FD;
        outputData.Stdio.data.fd = pipeFd[1];

        if (this->TempOutputPipe.init(*this->Loop, 0) < 0) {
          return false;
        }
        if (uv_pipe_open(this->TempOutputPipe, outputData.Stdio.data.fd) < 0) {
          return false;
        }
      }
      break;

    case cmUVProcessChainBuilder::External:
      outputData.Stdio.flags = UV_INHERIT_FD;
      outputData.Stdio.data.fd = output.FileDescriptor;
      break;
  }

  bool first = true;
  for (std::size_t i = 0; i < this->Builder->Processes.size(); i++) {
    this->Processes.emplace_back(cm::make_unique<ProcessData>());
    auto& process = *this->Processes.back();
    process.Data = this;
    process.ProcessStatus.Finished = false;

    if (!first) {
      auto& prevProcess = *this->Processes[i - 1];

      int pipeFd[2];
      if (cmGetPipes(pipeFd) < 0) {
        return false;
      }

      if (prevProcess.OutputPipe.init(*this->Loop, 0) < 0) {
        return false;
      }
      if (uv_pipe_open(prevProcess.OutputPipe, pipeFd[1]) < 0) {
        return false;
      }
      if (process.InputPipe.init(*this->Loop, 0) < 0) {
        return false;
      }
      if (uv_pipe_open(process.InputPipe, pipeFd[0]) < 0) {
        return false;
      }
    }

    first = false;
  }

  return true;
}

void cmUVProcessChain::InternalData::SpawnProcess(
  std::size_t index,
  const cmUVProcessChainBuilder::ProcessConfiguration& config, bool first,
  bool last)
{
  auto& process = *this->Processes[index];

  auto options = uv_process_options_t();

  // Bounds were checked at add time, first element is guaranteed to exist
  options.file = config.Arguments[0].c_str();

  std::vector<const char*> arguments;
  arguments.reserve(config.Arguments.size());
  for (auto const& arg : config.Arguments) {
    arguments.push_back(arg.c_str());
  }
  arguments.push_back(nullptr);
  options.args = const_cast<char**>(arguments.data());
  options.flags = UV_PROCESS_WINDOWS_HIDE;
  if (!this->Builder->WorkingDirectory.empty()) {
    options.cwd = this->Builder->WorkingDirectory.c_str();
  }

  std::array<uv_stdio_container_t, 3> stdio;
  if (first) {
    stdio[0] = this->InputStreamData.Stdio;
  } else {
    stdio[0] = uv_stdio_container_t();
    stdio[0].flags = UV_INHERIT_STREAM;
    stdio[0].data.stream = process.InputPipe;
  }
  if (last) {
    stdio[1] = this->OutputStreamData.Stdio;
  } else {
    stdio[1] = uv_stdio_container_t();
    stdio[1].flags = UV_INHERIT_STREAM;
    stdio[1].data.stream = process.OutputPipe;
  }
  stdio[2] = this->ErrorStreamData.Stdio;

  options.stdio = stdio.data();
  options.stdio_count = 3;
  options.exit_cb = [](uv_process_t* handle, int64_t exitStatus,
                       int termSignal) {
    auto* processData = static_cast<ProcessData*>(handle->data);
    processData->ProcessStatus.ExitStatus = exitStatus;
    processData->ProcessStatus.TermSignal = termSignal;
    processData->Finish();
  };

  if ((process.ProcessStatus.SpawnResult =
         process.Process.spawn(*this->Loop, options, &process)) < 0) {
    process.Finish();
  }
  process.InputPipe.reset();
  process.OutputPipe.reset();
}

void cmUVProcessChain::InternalData::Finish()
{
  this->TempOutputPipe.reset();
  this->TempErrorPipe.reset();
  this->Valid = true;
}

cmUVProcessChain::cmUVProcessChain()
  : Data(cm::make_unique<InternalData>())
{
  this->Data->Loop.init();
}

cmUVProcessChain::cmUVProcessChain(cmUVProcessChain&& other) noexcept
  : Data(std::move(other.Data))
{
}

cmUVProcessChain::~cmUVProcessChain() = default;

cmUVProcessChain& cmUVProcessChain::operator=(
  cmUVProcessChain&& other) noexcept
{
  this->Data = std::move(other.Data);
  return *this;
}

uv_loop_t& cmUVProcessChain::GetLoop()
{
  return *this->Data->Loop;
}

int cmUVProcessChain::OutputStream()
{
  return this->Data->OutputStreamData.BuiltinStream;
}

int cmUVProcessChain::ErrorStream()
{
  return this->Data->ErrorStreamData.BuiltinStream;
}

bool cmUVProcessChain::Valid() const
{
  return this->Data->Valid;
}

bool cmUVProcessChain::Wait(uint64_t milliseconds)
{
  bool timeout = false;
  cm::uv_timer_ptr timer;

  if (milliseconds > 0) {
    timer.init(*this->Data->Loop, &timeout);
    timer.start(
      [](uv_timer_t* handle) {
        auto* timeoutPtr = static_cast<bool*>(handle->data);
        *timeoutPtr = true;
      },
      milliseconds, 0);
  }

  while (!timeout &&
         this->Data->ProcessesCompleted < this->Data->Processes.size()) {
    uv_run(this->Data->Loop, UV_RUN_ONCE);
  }

  return !timeout;
}

std::vector<const cmUVProcessChain::Status*> cmUVProcessChain::GetStatus()
  const
{
  std::vector<const cmUVProcessChain::Status*> statuses(
    this->Data->Processes.size(), nullptr);
  for (std::size_t i = 0; i < statuses.size(); i++) {
    statuses[i] = &this->GetStatus(i);
  }
  return statuses;
}

const cmUVProcessChain::Status& cmUVProcessChain::GetStatus(
  std::size_t index) const
{
  return this->Data->Processes[index]->ProcessStatus;
}

bool cmUVProcessChain::Finished() const
{
  return this->Data->ProcessesCompleted >= this->Data->Processes.size();
}

std::pair<cmUVProcessChain::ExceptionCode, std::string>
cmUVProcessChain::Status::GetException() const
{
  if (this->SpawnResult) {
    return std::make_pair(ExceptionCode::Spawn,
                          uv_strerror(this->SpawnResult));
  }
#ifdef _WIN32
  if (this->Finished && (this->ExitStatus & 0xF0000000) == 0xC0000000) {
    // Child terminated due to exceptional behavior.
    switch (this->ExitStatus) {
      case STATUS_CONTROL_C_EXIT:
        return std::make_pair(ExceptionCode::Interrupt, "User interrupt");

      case STATUS_FLOAT_DENORMAL_OPERAND:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point exception (denormal operand)");
      case STATUS_FLOAT_DIVIDE_BY_ZERO:
        return std::make_pair(ExceptionCode::Numerical, "Divide-by-zero");
      case STATUS_FLOAT_INEXACT_RESULT:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point exception (inexact result)");
      case STATUS_FLOAT_INVALID_OPERATION:
        return std::make_pair(ExceptionCode::Numerical,
                              "Invalid floating-point operation");
      case STATUS_FLOAT_OVERFLOW:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point overflow");
      case STATUS_FLOAT_STACK_CHECK:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point stack check failed");
      case STATUS_FLOAT_UNDERFLOW:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point underflow");
#  ifdef STATUS_FLOAT_MULTIPLE_FAULTS
      case STATUS_FLOAT_MULTIPLE_FAULTS:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point exception (multiple faults)");
#  endif
#  ifdef STATUS_FLOAT_MULTIPLE_TRAPS
      case STATUS_FLOAT_MULTIPLE_TRAPS:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point exception (multiple traps)");
#  endif
      case STATUS_INTEGER_DIVIDE_BY_ZERO:
        return std::make_pair(ExceptionCode::Numerical,
                              "Integer divide-by-zero");
      case STATUS_INTEGER_OVERFLOW:
        return std::make_pair(ExceptionCode::Numerical, "Integer overflow");

      case STATUS_DATATYPE_MISALIGNMENT:
        return std::make_pair(ExceptionCode::Fault, "Datatype misalignment");
      case STATUS_ACCESS_VIOLATION:
        return std::make_pair(ExceptionCode::Fault, "Access violation");
      case STATUS_IN_PAGE_ERROR:
        return std::make_pair(ExceptionCode::Fault, "In-page error");
      case STATUS_INVALID_HANDLE:
        return std::make_pair(ExceptionCode::Fault, "Invalid handle");
      case STATUS_NONCONTINUABLE_EXCEPTION:
        return std::make_pair(ExceptionCode::Fault,
                              "Noncontinuable exception");
      case STATUS_INVALID_DISPOSITION:
        return std::make_pair(ExceptionCode::Fault, "Invalid disposition");
      case STATUS_ARRAY_BOUNDS_EXCEEDED:
        return std::make_pair(ExceptionCode::Fault, "Array bounds exceeded");
      case STATUS_STACK_OVERFLOW:
        return std::make_pair(ExceptionCode::Fault, "Stack overflow");

      case STATUS_ILLEGAL_INSTRUCTION:
        return std::make_pair(ExceptionCode::Illegal, "Illegal instruction");
      case STATUS_PRIVILEGED_INSTRUCTION:
        return std::make_pair(ExceptionCode::Illegal,
                              "Privileged instruction");

      case STATUS_NO_MEMORY:
      default: {
        char buf[256];
        snprintf(buf, sizeof(buf), "Exit code 0x%x\n",
                 static_cast<unsigned int>(this->ExitStatus));
        return std::make_pair(ExceptionCode::Other, buf);
      }
    }
  }
#else
  if (this->Finished && this->TermSignal) {
    switch (this->TermSignal) {
#  ifdef SIGSEGV
      case SIGSEGV:
        return std::make_pair(ExceptionCode::Fault, "Segmentation fault");
#  endif
#  ifdef SIGBUS
#    if !defined(SIGSEGV) || SIGBUS != SIGSEGV
      case SIGBUS:
        return std::make_pair(ExceptionCode::Fault, "Bus error");
#    endif
#  endif
#  ifdef SIGFPE
      case SIGFPE:
        return std::make_pair(ExceptionCode::Numerical,
                              "Floating-point exception");
#  endif
#  ifdef SIGILL
      case SIGILL:
        return std::make_pair(ExceptionCode::Illegal, "Illegal instruction");
#  endif
#  ifdef SIGINT
      case SIGINT:
        return std::make_pair(ExceptionCode::Interrupt, "User interrupt");
#  endif
#  ifdef SIGABRT
      case SIGABRT:
        return std::make_pair(ExceptionCode::Other, "Subprocess aborted");
#  endif
#  ifdef SIGKILL
      case SIGKILL:
        return std::make_pair(ExceptionCode::Other, "Subprocess killed");
#  endif
#  ifdef SIGTERM
      case SIGTERM:
        return std::make_pair(ExceptionCode::Other, "Subprocess terminated");
#  endif
#  ifdef SIGHUP
      case SIGHUP:
        return std::make_pair(ExceptionCode::Other, "SIGHUP");
#  endif
#  ifdef SIGQUIT
      case SIGQUIT:
        return std::make_pair(ExceptionCode::Other, "SIGQUIT");
#  endif
#  ifdef SIGTRAP
      case SIGTRAP:
        return std::make_pair(ExceptionCode::Other, "SIGTRAP");
#  endif
#  ifdef SIGIOT
#    if !defined(SIGABRT) || SIGIOT != SIGABRT
      case SIGIOT:
        return std::make_pair(ExceptionCode::Other, "SIGIOT");
#    endif
#  endif
#  ifdef SIGUSR1
      case SIGUSR1:
        return std::make_pair(ExceptionCode::Other, "SIGUSR1");
#  endif
#  ifdef SIGUSR2
      case SIGUSR2:
        return std::make_pair(ExceptionCode::Other, "SIGUSR2");
#  endif
#  ifdef SIGPIPE
      case SIGPIPE:
        return std::make_pair(ExceptionCode::Other, "SIGPIPE");
#  endif
#  ifdef SIGALRM
      case SIGALRM:
        return std::make_pair(ExceptionCode::Other, "SIGALRM");
#  endif
#  ifdef SIGSTKFLT
      case SIGSTKFLT:
        return std::make_pair(ExceptionCode::Other, "SIGSTKFLT");
#  endif
#  ifdef SIGCHLD
      case SIGCHLD:
        return std::make_pair(ExceptionCode::Other, "SIGCHLD");
#  elif defined(SIGCLD)
      case SIGCLD:
        return std::make_pair(ExceptionCode::Other, "SIGCLD");
#  endif
#  ifdef SIGCONT
      case SIGCONT:
        return std::make_pair(ExceptionCode::Other, "SIGCONT");
#  endif
#  ifdef SIGSTOP
      case SIGSTOP:
        return std::make_pair(ExceptionCode::Other, "SIGSTOP");
#  endif
#  ifdef SIGTSTP
      case SIGTSTP:
        return std::make_pair(ExceptionCode::Other, "SIGTSTP");
#  endif
#  ifdef SIGTTIN
      case SIGTTIN:
        return std::make_pair(ExceptionCode::Other, "SIGTTIN");
#  endif
#  ifdef SIGTTOU
      case SIGTTOU:
        return std::make_pair(ExceptionCode::Other, "SIGTTOU");
#  endif
#  ifdef SIGURG
      case SIGURG:
        return std::make_pair(ExceptionCode::Other, "SIGURG");
#  endif
#  ifdef SIGXCPU
      case SIGXCPU:
        return std::make_pair(ExceptionCode::Other, "SIGXCPU");
#  endif
#  ifdef SIGXFSZ
      case SIGXFSZ:
        return std::make_pair(ExceptionCode::Other, "SIGXFSZ");
#  endif
#  ifdef SIGVTALRM
      case SIGVTALRM:
        return std::make_pair(ExceptionCode::Other, "SIGVTALRM");
#  endif
#  ifdef SIGPROF
      case SIGPROF:
        return std::make_pair(ExceptionCode::Other, "SIGPROF");
#  endif
#  ifdef SIGWINCH
      case SIGWINCH:
        return std::make_pair(ExceptionCode::Other, "SIGWINCH");
#  endif
#  ifdef SIGPOLL
      case SIGPOLL:
        return std::make_pair(ExceptionCode::Other, "SIGPOLL");
#  endif
#  ifdef SIGIO
#    if !defined(SIGPOLL) || SIGIO != SIGPOLL
      case SIGIO:
        return std::make_pair(ExceptionCode::Other, "SIGIO");
#    endif
#  endif
#  ifdef SIGPWR
      case SIGPWR:
        return std::make_pair(ExceptionCode::Other, "SIGPWR");
#  endif
#  ifdef SIGSYS
      case SIGSYS:
        return std::make_pair(ExceptionCode::Other, "SIGSYS");
#  endif
#  ifdef SIGUNUSED
#    if !defined(SIGSYS) || SIGUNUSED != SIGSYS
      case SIGUNUSED:
        return std::make_pair(ExceptionCode::Other, "SIGUNUSED");
#    endif
#  endif
      default: {
        char buf[256];
        snprintf(buf, sizeof(buf), "Signal %d", this->TermSignal);
        return std::make_pair(ExceptionCode::Other, buf);
      }
    }
  }
#endif
  return std::make_pair(ExceptionCode::None, "");
}

void cmUVProcessChain::InternalData::ProcessData::Finish()
{
  this->ProcessStatus.Finished = true;
  this->Data->ProcessesCompleted++;
}
