/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUVProcessChain.h"

#include <cassert>
#include <istream> // IWYU pragma: keep
#include <iterator>
#include <utility>

#include <cm/memory>

#include "cm_uv.h"

#include "cmGetPipes.h"
#include "cmUVHandlePtr.h"
#include "cmUVStreambuf.h"

struct cmUVProcessChain::InternalData
{
  struct BasicStreamData
  {
    cmUVStreambuf Streambuf;
    cm::uv_pipe_ptr BuiltinStream;
    uv_stdio_container_t Stdio;
  };

  template <typename IOStream>
  struct StreamData : public BasicStreamData
  {
    StreamData()
      : BuiltinIOStream(&this->Streambuf)
    {
    }

    IOStream BuiltinIOStream;

    IOStream* GetBuiltinStream()
    {
      if (this->BuiltinStream.get()) {
        return &this->BuiltinIOStream;
      }
      return nullptr;
    }
  };

  struct ProcessData
  {
    cmUVProcessChain::InternalData* Data;
    cm::uv_process_ptr Process;
    cm::uv_pipe_ptr OutputPipe;
    bool Finished = false;
    Status ProcessStatus;
  };

  const cmUVProcessChainBuilder* Builder = nullptr;

  bool Valid = false;

  cm::uv_loop_ptr Loop;

  StreamData<std::istream> OutputStreamData;
  StreamData<std::istream> ErrorStreamData;

  unsigned int ProcessesCompleted = 0;
  std::vector<std::unique_ptr<ProcessData>> Processes;

  bool Prepare(const cmUVProcessChainBuilder* builder);
  bool AddCommand(const cmUVProcessChainBuilder::ProcessConfiguration& config,
                  bool first, bool last);
  bool Finish();

  static const Status* GetStatus(const ProcessData& data);
};

cmUVProcessChainBuilder::cmUVProcessChainBuilder()
{
  this->SetNoStream(Stream_INPUT)
    .SetNoStream(Stream_OUTPUT)
    .SetNoStream(Stream_ERROR);
}

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
      // FIXME
      break;

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

cmUVProcessChain cmUVProcessChainBuilder::Start() const
{
  cmUVProcessChain chain;

  if (!chain.Data->Prepare(this)) {
    return chain;
  }

  for (auto it = this->Processes.begin(); it != this->Processes.end(); ++it) {
    if (!chain.Data->AddCommand(*it, it == this->Processes.begin(),
                                it == std::prev(this->Processes.end()))) {
      return chain;
    }
  }

  chain.Data->Finish();

  return chain;
}

const cmUVProcessChain::Status* cmUVProcessChain::InternalData::GetStatus(
  const cmUVProcessChain::InternalData::ProcessData& data)
{
  if (data.Finished) {
    return &data.ProcessStatus;
  }
  return nullptr;
}

bool cmUVProcessChain::InternalData::Prepare(
  const cmUVProcessChainBuilder* builder)
{
  this->Builder = builder;

  auto const& output =
    this->Builder->Stdio[cmUVProcessChainBuilder::Stream_OUTPUT];
  auto& outputData = this->OutputStreamData;
  switch (output.Type) {
    case cmUVProcessChainBuilder::None:
      outputData.Stdio.flags = UV_IGNORE;
      break;

    case cmUVProcessChainBuilder::Builtin:
      outputData.BuiltinStream.init(*this->Loop, 0);
      outputData.Stdio.flags =
        static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
      outputData.Stdio.data.stream = outputData.BuiltinStream;
      break;

    case cmUVProcessChainBuilder::External:
      outputData.Stdio.flags = UV_INHERIT_FD;
      outputData.Stdio.data.fd = output.FileDescriptor;
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

      errorData.BuiltinStream.init(*this->Loop, 0);
      if (uv_pipe_open(errorData.BuiltinStream, pipeFd[0]) < 0) {
        return false;
      }
      errorData.Stdio.flags = UV_INHERIT_FD;
      errorData.Stdio.data.fd = pipeFd[1];
      break;
    }

    case cmUVProcessChainBuilder::External:
      errorData.Stdio.flags = UV_INHERIT_FD;
      errorData.Stdio.data.fd = error.FileDescriptor;
      break;
  }

  return true;
}

bool cmUVProcessChain::InternalData::AddCommand(
  const cmUVProcessChainBuilder::ProcessConfiguration& config, bool first,
  bool last)
{
  this->Processes.emplace_back(cm::make_unique<ProcessData>());
  auto& process = *this->Processes.back();
  process.Data = this;

  auto options = uv_process_options_t();

  // Bounds were checked at add time, first element is guaranteed to exist
  options.file = config.Arguments[0].c_str();

  std::vector<const char*> arguments;
  for (auto const& arg : config.Arguments) {
    arguments.push_back(arg.c_str());
  }
  arguments.push_back(nullptr);
  options.args = const_cast<char**>(arguments.data());
  options.flags = UV_PROCESS_WINDOWS_HIDE;

  std::array<uv_stdio_container_t, 3> stdio;
  stdio[0] = uv_stdio_container_t();
  if (first) {
    stdio[0].flags = UV_IGNORE;
  } else {
    assert(this->Processes.size() >= 2);
    auto& prev = *this->Processes[this->Processes.size() - 2];
    stdio[0].flags = UV_INHERIT_STREAM;
    stdio[0].data.stream = prev.OutputPipe;
  }
  if (last) {
    stdio[1] = this->OutputStreamData.Stdio;
  } else {
    if (process.OutputPipe.init(*this->Loop, 0) < 0) {
      return false;
    }
    stdio[1] = uv_stdio_container_t();
    stdio[1].flags =
      static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    stdio[1].data.stream = process.OutputPipe;
  }
  stdio[2] = this->ErrorStreamData.Stdio;

  options.stdio = stdio.data();
  options.stdio_count = 3;
  options.exit_cb = [](uv_process_t* handle, int64_t exitStatus,
                       int termSignal) {
    auto* processData = static_cast<ProcessData*>(handle->data);
    processData->Finished = true;
    processData->ProcessStatus.ExitStatus = exitStatus;
    processData->ProcessStatus.TermSignal = termSignal;
    processData->Data->ProcessesCompleted++;
  };

  return process.Process.spawn(*this->Loop, options, &process) >= 0;
}

bool cmUVProcessChain::InternalData::Finish()
{
  if (this->Builder->Stdio[cmUVProcessChainBuilder::Stream_OUTPUT].Type ==
      cmUVProcessChainBuilder::Builtin) {
    this->OutputStreamData.Streambuf.open(
      this->OutputStreamData.BuiltinStream);
  }

  if (this->Builder->Stdio[cmUVProcessChainBuilder::Stream_ERROR].Type ==
      cmUVProcessChainBuilder::Builtin) {
    cm::uv_pipe_ptr tmpPipe;
    if (tmpPipe.init(*this->Loop, 0) < 0) {
      return false;
    }
    if (uv_pipe_open(tmpPipe, this->ErrorStreamData.Stdio.data.fd) < 0) {
      return false;
    }
    tmpPipe.reset();

    this->ErrorStreamData.Streambuf.open(this->ErrorStreamData.BuiltinStream);
  }

  this->Valid = true;
  return true;
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

std::istream* cmUVProcessChain::OutputStream()
{
  return this->Data->OutputStreamData.GetBuiltinStream();
}

std::istream* cmUVProcessChain::ErrorStream()
{
  return this->Data->ErrorStreamData.GetBuiltinStream();
}

bool cmUVProcessChain::Valid() const
{
  return this->Data->Valid;
}

bool cmUVProcessChain::Wait(int64_t milliseconds)
{
  bool timeout = false;
  cm::uv_timer_ptr timer;

  if (milliseconds >= 0) {
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
    statuses[i] = this->GetStatus(i);
  }
  return statuses;
}

const cmUVProcessChain::Status* cmUVProcessChain::GetStatus(
  std::size_t index) const
{
  auto const& process = *this->Data->Processes[index];
  if (process.Finished) {
    return &process.ProcessStatus;
  }
  return nullptr;
}
