/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWorkerPool.h"

#include <algorithm>
#include <array>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

#include <cm/memory>

#include <cm3p/uv.h>

#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmUVHandlePtr.h"
#include "cmUVSignalHackRAII.h" // IWYU pragma: keep

/**
 * @brief libuv pipe buffer class
 */
class cmUVPipeBuffer
{
public:
  using DataRange = cmRange<const char*>;
  using DataFunction = std::function<void(DataRange)>;
  /// On error the ssize_t argument is a non zero libuv error code
  using EndFunction = std::function<void(ssize_t)>;

  /**
   * Reset to construction state
   */
  void reset();

  /**
   * Initializes uv_pipe(), uv_stream() and uv_handle()
   * @return true on success
   */
  bool init(uv_loop_t* uv_loop);

  /**
   * Start reading
   * @return true on success
   */
  bool startRead(DataFunction dataFunction, EndFunction endFunction);

  //! libuv pipe
  uv_pipe_t* uv_pipe() const { return this->UVPipe_.get(); }
  //! uv_pipe() casted to libuv stream
  uv_stream_t* uv_stream() const
  {
    return static_cast<uv_stream_t*>(this->UVPipe_);
  }
  //! uv_pipe() casted to libuv handle
  uv_handle_t* uv_handle() { return static_cast<uv_handle_t*>(this->UVPipe_); }

private:
  // -- Libuv callbacks
  static void UVAlloc(uv_handle_t* handle, size_t suggestedSize,
                      uv_buf_t* buf);
  static void UVData(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

  cm::uv_pipe_ptr UVPipe_;
  std::vector<char> Buffer_;
  DataFunction DataFunction_;
  EndFunction EndFunction_;
};

void cmUVPipeBuffer::reset()
{
  if (this->UVPipe_.get() != nullptr) {
    this->EndFunction_ = nullptr;
    this->DataFunction_ = nullptr;
    this->Buffer_.clear();
    this->Buffer_.shrink_to_fit();
    this->UVPipe_.reset();
  }
}

bool cmUVPipeBuffer::init(uv_loop_t* uv_loop)
{
  this->reset();
  if (uv_loop == nullptr) {
    return false;
  }
  int ret = this->UVPipe_.init(*uv_loop, 0, this);
  return (ret == 0);
}

bool cmUVPipeBuffer::startRead(DataFunction dataFunction,
                               EndFunction endFunction)
{
  if (this->UVPipe_.get() == nullptr) {
    return false;
  }
  if (!dataFunction || !endFunction) {
    return false;
  }
  this->DataFunction_ = std::move(dataFunction);
  this->EndFunction_ = std::move(endFunction);
  int ret = uv_read_start(this->uv_stream(), &cmUVPipeBuffer::UVAlloc,
                          &cmUVPipeBuffer::UVData);
  return (ret == 0);
}

void cmUVPipeBuffer::UVAlloc(uv_handle_t* handle, size_t suggestedSize,
                             uv_buf_t* buf)
{
  auto& pipe = *reinterpret_cast<cmUVPipeBuffer*>(handle->data);
  pipe.Buffer_.resize(suggestedSize);
  buf->base = pipe.Buffer_.data();
  buf->len = static_cast<unsigned long>(pipe.Buffer_.size());
}

void cmUVPipeBuffer::UVData(uv_stream_t* stream, ssize_t nread,
                            const uv_buf_t* buf)
{
  auto& pipe = *reinterpret_cast<cmUVPipeBuffer*>(stream->data);
  if (nread > 0) {
    if (buf->base != nullptr) {
      // Call data function
      pipe.DataFunction_(DataRange(buf->base, buf->base + nread));
    }
  } else if (nread < 0) {
    // Save the end function on the stack before resetting the pipe
    EndFunction efunc;
    efunc.swap(pipe.EndFunction_);
    // Reset pipe before calling the end function
    pipe.reset();
    // Call end function
    efunc((nread == UV_EOF) ? 0 : nread);
  }
}

/**
 * @brief External process management class
 */
class cmUVReadOnlyProcess
{
public:
  // -- Types
  //! @brief Process settings
  struct SetupT
  {
    std::string WorkingDirectory;
    std::vector<std::string> Command;
    cmWorkerPool::ProcessResultT* Result = nullptr;
    bool MergedOutput = false;
  };

  // -- Const accessors
  SetupT const& Setup() const { return this->Setup_; }
  cmWorkerPool::ProcessResultT* Result() const { return this->Setup_.Result; }
  bool IsStarted() const { return this->IsStarted_; }
  bool IsFinished() const { return this->IsFinished_; }

  // -- Runtime
  void setup(cmWorkerPool::ProcessResultT* result, bool mergedOutput,
             std::vector<std::string> const& command,
             std::string const& workingDirectory = std::string());
  bool start(uv_loop_t* uv_loop, std::function<void()> finishedCallback);

private:
  // -- Libuv callbacks
  static void UVExit(uv_process_t* handle, int64_t exitStatus, int termSignal);
  void UVPipeOutData(cmUVPipeBuffer::DataRange data) const;
  void UVPipeOutEnd(ssize_t error);
  void UVPipeErrData(cmUVPipeBuffer::DataRange data) const;
  void UVPipeErrEnd(ssize_t error);
  void UVTryFinish();

  // -- Setup
  SetupT Setup_;
  // -- Runtime
  bool IsStarted_ = false;
  bool IsFinished_ = false;
  std::function<void()> FinishedCallback_;
  std::vector<const char*> CommandPtr_;
  std::array<uv_stdio_container_t, 3> UVOptionsStdIO_;
  uv_process_options_t UVOptions_;
  cm::uv_process_ptr UVProcess_;
  cmUVPipeBuffer UVPipeOut_;
  cmUVPipeBuffer UVPipeErr_;
};

void cmUVReadOnlyProcess::setup(cmWorkerPool::ProcessResultT* result,
                                bool mergedOutput,
                                std::vector<std::string> const& command,
                                std::string const& workingDirectory)
{
  this->Setup_.WorkingDirectory = workingDirectory;
  this->Setup_.Command = command;
  this->Setup_.Result = result;
  this->Setup_.MergedOutput = mergedOutput;
}

bool cmUVReadOnlyProcess::start(uv_loop_t* uv_loop,
                                std::function<void()> finishedCallback)
{
  if (this->IsStarted() || (this->Result() == nullptr)) {
    return false;
  }

  // Reset result before the start
  this->Result()->reset();

  // Fill command string pointers
  if (!this->Setup().Command.empty()) {
    this->CommandPtr_.reserve(this->Setup().Command.size() + 1);
    for (std::string const& arg : this->Setup().Command) {
      this->CommandPtr_.push_back(arg.c_str());
    }
    this->CommandPtr_.push_back(nullptr);
  } else {
    this->Result()->ErrorMessage = "Empty command";
  }

  if (!this->Result()->error()) {
    if (!this->UVPipeOut_.init(uv_loop)) {
      this->Result()->ErrorMessage = "libuv stdout pipe initialization failed";
    }
  }
  if (!this->Result()->error()) {
    if (!this->UVPipeErr_.init(uv_loop)) {
      this->Result()->ErrorMessage = "libuv stderr pipe initialization failed";
    }
  }
  if (!this->Result()->error()) {
    // -- Setup process stdio options
    // stdin
    this->UVOptionsStdIO_[0].flags = UV_IGNORE;
    this->UVOptionsStdIO_[0].data.stream = nullptr;
    // stdout
    this->UVOptionsStdIO_[1].flags =
      static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    this->UVOptionsStdIO_[1].data.stream = this->UVPipeOut_.uv_stream();
    // stderr
    this->UVOptionsStdIO_[2].flags =
      static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    this->UVOptionsStdIO_[2].data.stream = this->UVPipeErr_.uv_stream();

    // -- Setup process options
    std::fill_n(reinterpret_cast<char*>(&this->UVOptions_),
                sizeof(this->UVOptions_), 0);
    this->UVOptions_.exit_cb = &cmUVReadOnlyProcess::UVExit;
    this->UVOptions_.file = this->CommandPtr_[0];
    this->UVOptions_.args = const_cast<char**>(this->CommandPtr_.data());
    this->UVOptions_.cwd = this->Setup_.WorkingDirectory.c_str();
    this->UVOptions_.flags = UV_PROCESS_WINDOWS_HIDE;
    this->UVOptions_.stdio_count =
      static_cast<int>(this->UVOptionsStdIO_.size());
    this->UVOptions_.stdio = this->UVOptionsStdIO_.data();

    // -- Spawn process
    int uvErrorCode = this->UVProcess_.spawn(*uv_loop, this->UVOptions_, this);
    if (uvErrorCode != 0) {
      this->Result()->ErrorMessage = "libuv process spawn failed";
      if (const char* uvErr = uv_strerror(uvErrorCode)) {
        this->Result()->ErrorMessage += ": ";
        this->Result()->ErrorMessage += uvErr;
      }
    }
  }
  // -- Start reading from stdio streams
  if (!this->Result()->error()) {
    if (!this->UVPipeOut_.startRead(
          [this](cmUVPipeBuffer::DataRange range) {
            this->UVPipeOutData(range);
          },
          [this](ssize_t error) { this->UVPipeOutEnd(error); })) {
      this->Result()->ErrorMessage =
        "libuv start reading from stdout pipe failed";
    }
  }
  if (!this->Result()->error()) {
    if (!this->UVPipeErr_.startRead(
          [this](cmUVPipeBuffer::DataRange range) {
            this->UVPipeErrData(range);
          },
          [this](ssize_t error) { this->UVPipeErrEnd(error); })) {
      this->Result()->ErrorMessage =
        "libuv start reading from stderr pipe failed";
    }
  }

  if (!this->Result()->error()) {
    this->IsStarted_ = true;
    this->FinishedCallback_ = std::move(finishedCallback);
  } else {
    // Clear libuv handles and finish
    this->UVProcess_.reset();
    this->UVPipeOut_.reset();
    this->UVPipeErr_.reset();
    this->CommandPtr_.clear();
  }

  return this->IsStarted();
}

void cmUVReadOnlyProcess::UVExit(uv_process_t* handle, int64_t exitStatus,
                                 int termSignal)
{
  auto& proc = *reinterpret_cast<cmUVReadOnlyProcess*>(handle->data);
  if (proc.IsStarted() && !proc.IsFinished()) {
    // Set error message on demand
    proc.Result()->ExitStatus = exitStatus;
    proc.Result()->TermSignal = termSignal;
    if (!proc.Result()->error()) {
      if (termSignal != 0) {
        proc.Result()->ErrorMessage = cmStrCat(
          "Process was terminated by signal ", proc.Result()->TermSignal);
      } else if (exitStatus != 0) {
        proc.Result()->ErrorMessage = cmStrCat(
          "Process failed with return value ", proc.Result()->ExitStatus);
      }
    }

    // Reset process handle
    proc.UVProcess_.reset();
    // Try finish
    proc.UVTryFinish();
  }
}

void cmUVReadOnlyProcess::UVPipeOutData(cmUVPipeBuffer::DataRange data) const
{
  this->Result()->StdOut.append(data.begin(), data.end());
}

void cmUVReadOnlyProcess::UVPipeOutEnd(ssize_t error)
{
  // Process pipe error
  if ((error != 0) && !this->Result()->error()) {
    this->Result()->ErrorMessage = cmStrCat(
      "Reading from stdout pipe failed with libuv error code ", error);
  }
  // Try finish
  this->UVTryFinish();
}

void cmUVReadOnlyProcess::UVPipeErrData(cmUVPipeBuffer::DataRange data) const
{
  std::string* str = this->Setup_.MergedOutput ? &this->Result()->StdOut
                                               : &this->Result()->StdErr;
  str->append(data.begin(), data.end());
}

void cmUVReadOnlyProcess::UVPipeErrEnd(ssize_t error)
{
  // Process pipe error
  if ((error != 0) && !this->Result()->error()) {
    this->Result()->ErrorMessage = cmStrCat(
      "Reading from stderr pipe failed with libuv error code ", error);
  }
  // Try finish
  this->UVTryFinish();
}

void cmUVReadOnlyProcess::UVTryFinish()
{
  // There still might be data in the pipes after the process has finished.
  // Therefore check if the process is finished AND all pipes are closed
  // before signaling the worker thread to continue.
  if ((this->UVProcess_.get() != nullptr) ||
      (this->UVPipeOut_.uv_pipe() != nullptr) ||
      (this->UVPipeErr_.uv_pipe() != nullptr)) {
    return;
  }
  this->IsFinished_ = true;
  this->FinishedCallback_();
}

/**
 * @brief Worker pool worker thread
 */
class cmWorkerPoolWorker
{
public:
  cmWorkerPoolWorker(uv_loop_t& uvLoop);
  ~cmWorkerPoolWorker();

  cmWorkerPoolWorker(cmWorkerPoolWorker const&) = delete;
  cmWorkerPoolWorker& operator=(cmWorkerPoolWorker const&) = delete;

  /**
   * Set the internal thread
   */
  void SetThread(std::thread&& aThread) { this->Thread_ = std::move(aThread); }

  /**
   * Run an external process
   */
  bool RunProcess(cmWorkerPool::ProcessResultT& result,
                  std::vector<std::string> const& command,
                  std::string const& workingDirectory);

private:
  // -- Libuv callbacks
  static void UVProcessStart(uv_async_t* handle);
  void UVProcessFinished();

  // -- Process management
  struct
  {
    std::mutex Mutex;
    cm::uv_async_ptr Request;
    std::condition_variable Condition;
    std::unique_ptr<cmUVReadOnlyProcess> ROP;
  } Proc_;
  // -- System thread
  std::thread Thread_;
};

cmWorkerPoolWorker::cmWorkerPoolWorker(uv_loop_t& uvLoop)
{
  this->Proc_.Request.init(uvLoop, &cmWorkerPoolWorker::UVProcessStart, this);
}

cmWorkerPoolWorker::~cmWorkerPoolWorker()
{
  if (this->Thread_.joinable()) {
    this->Thread_.join();
  }
}

bool cmWorkerPoolWorker::RunProcess(cmWorkerPool::ProcessResultT& result,
                                    std::vector<std::string> const& command,
                                    std::string const& workingDirectory)
{
  if (command.empty()) {
    return false;
  }
  // Create process instance
  {
    std::lock_guard<std::mutex> lock(this->Proc_.Mutex);
    this->Proc_.ROP = cm::make_unique<cmUVReadOnlyProcess>();
    this->Proc_.ROP->setup(&result, true, command, workingDirectory);
  }
  // Send asynchronous process start request to libuv loop
  this->Proc_.Request.send();
  // Wait until the process has been finished and destroyed
  {
    std::unique_lock<std::mutex> ulock(this->Proc_.Mutex);
    while (this->Proc_.ROP) {
      this->Proc_.Condition.wait(ulock);
    }
  }
  return !result.error();
}

void cmWorkerPoolWorker::UVProcessStart(uv_async_t* handle)
{
  auto* wrk = reinterpret_cast<cmWorkerPoolWorker*>(handle->data);
  bool startFailed = false;
  {
    auto& Proc = wrk->Proc_;
    std::lock_guard<std::mutex> lock(Proc.Mutex);
    if (Proc.ROP && !Proc.ROP->IsStarted()) {
      startFailed =
        !Proc.ROP->start(handle->loop, [wrk] { wrk->UVProcessFinished(); });
    }
  }
  // Clean up if starting of the process failed
  if (startFailed) {
    wrk->UVProcessFinished();
  }
}

void cmWorkerPoolWorker::UVProcessFinished()
{
  std::lock_guard<std::mutex> lock(this->Proc_.Mutex);
  if (this->Proc_.ROP &&
      (this->Proc_.ROP->IsFinished() || !this->Proc_.ROP->IsStarted())) {
    this->Proc_.ROP.reset();
  }
  // Notify idling thread
  this->Proc_.Condition.notify_one();
}

/**
 * @brief Private worker pool internals
 */
class cmWorkerPoolInternal
{
public:
  // -- Constructors
  cmWorkerPoolInternal(cmWorkerPool* pool);
  ~cmWorkerPoolInternal();

  /**
   * Runs the libuv loop.
   */
  bool Process();

  /**
   * Clear queue and abort threads.
   */
  void Abort();

  /**
   * Push a job to the queue and notify a worker.
   */
  bool PushJob(cmWorkerPool::JobHandleT&& jobHandle);

  /**
   * Worker thread main loop method.
   */
  void Work(unsigned int workerIndex);

  // -- Request slots
  static void UVSlotBegin(uv_async_t* handle);
  static void UVSlotEnd(uv_async_t* handle);

  // -- UV loop
#ifdef CMAKE_UV_SIGNAL_HACK
  std::unique_ptr<cmUVSignalHackRAII> UVHackRAII;
#endif
  std::unique_ptr<uv_loop_t> UVLoop;
  cm::uv_async_ptr UVRequestBegin;
  cm::uv_async_ptr UVRequestEnd;

  // -- Thread pool and job queue
  std::mutex Mutex;
  bool Processing = false;
  bool Aborting = false;
  bool FenceProcessing = false;
  unsigned int WorkersRunning = 0;
  unsigned int WorkersIdle = 0;
  unsigned int JobsProcessing = 0;
  std::deque<cmWorkerPool::JobHandleT> Queue;
  std::condition_variable Condition;
  std::condition_variable ConditionFence;
  std::vector<std::unique_ptr<cmWorkerPoolWorker>> Workers;

  // -- References
  cmWorkerPool* Pool = nullptr;
};

void cmWorkerPool::ProcessResultT::reset()
{
  this->ExitStatus = 0;
  this->TermSignal = 0;
  if (!this->StdOut.empty()) {
    this->StdOut.clear();
    this->StdOut.shrink_to_fit();
  }
  if (!this->StdErr.empty()) {
    this->StdErr.clear();
    this->StdErr.shrink_to_fit();
  }
  if (!this->ErrorMessage.empty()) {
    this->ErrorMessage.clear();
    this->ErrorMessage.shrink_to_fit();
  }
}

cmWorkerPoolInternal::cmWorkerPoolInternal(cmWorkerPool* pool)
  : Pool(pool)
{
  // Initialize libuv loop
  uv_disable_stdio_inheritance();
#ifdef CMAKE_UV_SIGNAL_HACK
  UVHackRAII = cm::make_unique<cmUVSignalHackRAII>();
#endif
  this->UVLoop = cm::make_unique<uv_loop_t>();
  uv_loop_init(this->UVLoop.get());
}

cmWorkerPoolInternal::~cmWorkerPoolInternal()
{
  uv_loop_close(this->UVLoop.get());
}

bool cmWorkerPoolInternal::Process()
{
  // Reset state flags
  this->Processing = true;
  this->Aborting = false;
  // Initialize libuv asynchronous request
  this->UVRequestBegin.init(*this->UVLoop, &cmWorkerPoolInternal::UVSlotBegin,
                            this);
  this->UVRequestEnd.init(*this->UVLoop, &cmWorkerPoolInternal::UVSlotEnd,
                          this);
  // Send begin request
  this->UVRequestBegin.send();
  // Run libuv loop
  bool success = (uv_run(this->UVLoop.get(), UV_RUN_DEFAULT) == 0);
  // Update state flags
  this->Processing = false;
  this->Aborting = false;
  return success;
}

void cmWorkerPoolInternal::Abort()
{
  // Clear all jobs and set abort flag
  std::lock_guard<std::mutex> guard(this->Mutex);
  if (!this->Aborting) {
    // Register abort and clear queue
    this->Aborting = true;
    this->Queue.clear();
    this->Condition.notify_all();
  }
}

inline bool cmWorkerPoolInternal::PushJob(cmWorkerPool::JobHandleT&& jobHandle)
{
  std::lock_guard<std::mutex> guard(this->Mutex);
  if (this->Aborting) {
    return false;
  }
  // Append the job to the queue
  this->Queue.emplace_back(std::move(jobHandle));
  // Notify an idle worker if there's one
  if (this->WorkersIdle != 0) {
    this->Condition.notify_one();
  }
  // Return success
  return true;
}

void cmWorkerPoolInternal::UVSlotBegin(uv_async_t* handle)
{
  auto& gint = *reinterpret_cast<cmWorkerPoolInternal*>(handle->data);
  // Create worker threads
  {
    unsigned int const num = gint.Pool->ThreadCount();
    // Create workers
    gint.Workers.reserve(num);
    for (unsigned int ii = 0; ii != num; ++ii) {
      gint.Workers.emplace_back(
        cm::make_unique<cmWorkerPoolWorker>(*gint.UVLoop));
    }
    // Start worker threads
    for (unsigned int ii = 0; ii != num; ++ii) {
      gint.Workers[ii]->SetThread(
        std::thread(&cmWorkerPoolInternal::Work, &gint, ii));
    }
  }
  // Destroy begin request
  gint.UVRequestBegin.reset();
}

void cmWorkerPoolInternal::UVSlotEnd(uv_async_t* handle)
{
  auto& gint = *reinterpret_cast<cmWorkerPoolInternal*>(handle->data);
  // Join and destroy worker threads
  gint.Workers.clear();
  // Destroy end request
  gint.UVRequestEnd.reset();
}

void cmWorkerPoolInternal::Work(unsigned int workerIndex)
{
  cmWorkerPool::JobHandleT jobHandle;
  std::unique_lock<std::mutex> uLock(this->Mutex);
  // Increment running workers count
  ++this->WorkersRunning;
  // Enter worker main loop
  while (true) {
    // Abort on request
    if (this->Aborting) {
      break;
    }
    // Wait for new jobs on the main CV
    if (this->Queue.empty()) {
      ++this->WorkersIdle;
      this->Condition.wait(uLock);
      --this->WorkersIdle;
      continue;
    }

    // If there is a fence currently active or waiting,
    // sleep on the main CV and try again.
    if (this->FenceProcessing) {
      this->Condition.wait(uLock);
      continue;
    }

    // Pop next job from queue
    jobHandle = std::move(this->Queue.front());
    this->Queue.pop_front();

    // Check for fence jobs
    bool raisedFence = false;
    if (jobHandle->IsFence()) {
      this->FenceProcessing = true;
      raisedFence = true;
      // Wait on the Fence CV until all pending jobs are done.
      while (this->JobsProcessing != 0 && !this->Aborting) {
        this->ConditionFence.wait(uLock);
      }
      // When aborting, explicitly kick all threads alive once more.
      if (this->Aborting) {
        this->FenceProcessing = false;
        this->Condition.notify_all();
        break;
      }
    }

    // Unlocked scope for job processing
    ++this->JobsProcessing;
    {
      uLock.unlock();
      jobHandle->Work(this->Pool, workerIndex); // Process job
      jobHandle.reset();                        // Destroy job
      uLock.lock();
    }
    --this->JobsProcessing;

    // If this was the thread that entered fence processing
    // originally, notify all idling workers that the fence
    // is done.
    if (raisedFence) {
      this->FenceProcessing = false;
      this->Condition.notify_all();
    }
    // If fence processing is still not done, notify the
    // the fencing worker when all active jobs are done.
    if (this->FenceProcessing && this->JobsProcessing == 0) {
      this->ConditionFence.notify_all();
    }
  }

  // Decrement running workers count
  if (--this->WorkersRunning == 0) {
    // Last worker thread about to finish. Send libuv event.
    this->UVRequestEnd.send();
  }
}

cmWorkerPool::JobT::~JobT() = default;

bool cmWorkerPool::JobT::RunProcess(ProcessResultT& result,
                                    std::vector<std::string> const& command,
                                    std::string const& workingDirectory)
{
  // Get worker by index
  auto* wrk = this->Pool_->Int_->Workers.at(this->WorkerIndex_).get();
  return wrk->RunProcess(result, command, workingDirectory);
}

cmWorkerPool::cmWorkerPool()
  : Int_(cm::make_unique<cmWorkerPoolInternal>(this))
{
}

cmWorkerPool::~cmWorkerPool() = default;

void cmWorkerPool::SetThreadCount(unsigned int threadCount)
{
  if (!this->Int_->Processing) {
    this->ThreadCount_ = (threadCount > 0) ? threadCount : 1u;
  }
}

bool cmWorkerPool::Process(void* userData)
{
  // Setup user data
  this->UserData_ = userData;
  // Run libuv loop
  bool success = this->Int_->Process();
  // Clear user data
  this->UserData_ = nullptr;
  // Return
  return success;
}

bool cmWorkerPool::PushJob(JobHandleT&& jobHandle)
{
  return this->Int_->PushJob(std::move(jobHandle));
}

void cmWorkerPool::Abort()
{
  this->Int_->Abort();
}
