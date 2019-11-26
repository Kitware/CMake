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

#include "cm_uv.h"

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

public:
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
  uv_pipe_t* uv_pipe() const { return UVPipe_.get(); }
  //! uv_pipe() casted to libuv stream
  uv_stream_t* uv_stream() const { return static_cast<uv_stream_t*>(UVPipe_); }
  //! uv_pipe() casted to libuv handle
  uv_handle_t* uv_handle() { return static_cast<uv_handle_t*>(UVPipe_); }

private:
  // -- Libuv callbacks
  static void UVAlloc(uv_handle_t* handle, size_t suggestedSize,
                      uv_buf_t* buf);
  static void UVData(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

private:
  cm::uv_pipe_ptr UVPipe_;
  std::vector<char> Buffer_;
  DataFunction DataFunction_;
  EndFunction EndFunction_;
};

void cmUVPipeBuffer::reset()
{
  if (UVPipe_.get() != nullptr) {
    EndFunction_ = nullptr;
    DataFunction_ = nullptr;
    Buffer_.clear();
    Buffer_.shrink_to_fit();
    UVPipe_.reset();
  }
}

bool cmUVPipeBuffer::init(uv_loop_t* uv_loop)
{
  reset();
  if (uv_loop == nullptr) {
    return false;
  }
  int ret = UVPipe_.init(*uv_loop, 0, this);
  return (ret == 0);
}

bool cmUVPipeBuffer::startRead(DataFunction dataFunction,
                               EndFunction endFunction)
{
  if (UVPipe_.get() == nullptr) {
    return false;
  }
  if (!dataFunction || !endFunction) {
    return false;
  }
  DataFunction_ = std::move(dataFunction);
  EndFunction_ = std::move(endFunction);
  int ret = uv_read_start(uv_stream(), &cmUVPipeBuffer::UVAlloc,
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

public:
  // -- Const accessors
  SetupT const& Setup() const { return Setup_; }
  cmWorkerPool::ProcessResultT* Result() const { return Setup_.Result; }
  bool IsStarted() const { return IsStarted_; }
  bool IsFinished() const { return IsFinished_; }

  // -- Runtime
  void setup(cmWorkerPool::ProcessResultT* result, bool mergedOutput,
             std::vector<std::string> const& command,
             std::string const& workingDirectory = std::string());
  bool start(uv_loop_t* uv_loop, std::function<void()> finishedCallback);

private:
  // -- Libuv callbacks
  static void UVExit(uv_process_t* handle, int64_t exitStatus, int termSignal);
  void UVPipeOutData(cmUVPipeBuffer::DataRange data);
  void UVPipeOutEnd(ssize_t error);
  void UVPipeErrData(cmUVPipeBuffer::DataRange data);
  void UVPipeErrEnd(ssize_t error);
  void UVTryFinish();

private:
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
  Setup_.WorkingDirectory = workingDirectory;
  Setup_.Command = command;
  Setup_.Result = result;
  Setup_.MergedOutput = mergedOutput;
}

bool cmUVReadOnlyProcess::start(uv_loop_t* uv_loop,
                                std::function<void()> finishedCallback)
{
  if (IsStarted() || (Result() == nullptr)) {
    return false;
  }

  // Reset result before the start
  Result()->reset();

  // Fill command string pointers
  if (!Setup().Command.empty()) {
    CommandPtr_.reserve(Setup().Command.size() + 1);
    for (std::string const& arg : Setup().Command) {
      CommandPtr_.push_back(arg.c_str());
    }
    CommandPtr_.push_back(nullptr);
  } else {
    Result()->ErrorMessage = "Empty command";
  }

  if (!Result()->error()) {
    if (!UVPipeOut_.init(uv_loop)) {
      Result()->ErrorMessage = "libuv stdout pipe initialization failed";
    }
  }
  if (!Result()->error()) {
    if (!UVPipeErr_.init(uv_loop)) {
      Result()->ErrorMessage = "libuv stderr pipe initialization failed";
    }
  }
  if (!Result()->error()) {
    // -- Setup process stdio options
    // stdin
    UVOptionsStdIO_[0].flags = UV_IGNORE;
    UVOptionsStdIO_[0].data.stream = nullptr;
    // stdout
    UVOptionsStdIO_[1].flags =
      static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    UVOptionsStdIO_[1].data.stream = UVPipeOut_.uv_stream();
    // stderr
    UVOptionsStdIO_[2].flags =
      static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
    UVOptionsStdIO_[2].data.stream = UVPipeErr_.uv_stream();

    // -- Setup process options
    std::fill_n(reinterpret_cast<char*>(&UVOptions_), sizeof(UVOptions_), 0);
    UVOptions_.exit_cb = &cmUVReadOnlyProcess::UVExit;
    UVOptions_.file = CommandPtr_[0];
    UVOptions_.args = const_cast<char**>(CommandPtr_.data());
    UVOptions_.cwd = Setup_.WorkingDirectory.c_str();
    UVOptions_.flags = UV_PROCESS_WINDOWS_HIDE;
    UVOptions_.stdio_count = static_cast<int>(UVOptionsStdIO_.size());
    UVOptions_.stdio = UVOptionsStdIO_.data();

    // -- Spawn process
    int uvErrorCode = UVProcess_.spawn(*uv_loop, UVOptions_, this);
    if (uvErrorCode != 0) {
      Result()->ErrorMessage = "libuv process spawn failed";
      if (const char* uvErr = uv_strerror(uvErrorCode)) {
        Result()->ErrorMessage += ": ";
        Result()->ErrorMessage += uvErr;
      }
    }
  }
  // -- Start reading from stdio streams
  if (!Result()->error()) {
    if (!UVPipeOut_.startRead(
          [this](cmUVPipeBuffer::DataRange range) {
            this->UVPipeOutData(range);
          },
          [this](ssize_t error) { this->UVPipeOutEnd(error); })) {
      Result()->ErrorMessage = "libuv start reading from stdout pipe failed";
    }
  }
  if (!Result()->error()) {
    if (!UVPipeErr_.startRead(
          [this](cmUVPipeBuffer::DataRange range) {
            this->UVPipeErrData(range);
          },
          [this](ssize_t error) { this->UVPipeErrEnd(error); })) {
      Result()->ErrorMessage = "libuv start reading from stderr pipe failed";
    }
  }

  if (!Result()->error()) {
    IsStarted_ = true;
    FinishedCallback_ = std::move(finishedCallback);
  } else {
    // Clear libuv handles and finish
    UVProcess_.reset();
    UVPipeOut_.reset();
    UVPipeErr_.reset();
    CommandPtr_.clear();
  }

  return IsStarted();
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

void cmUVReadOnlyProcess::UVPipeOutData(cmUVPipeBuffer::DataRange data)
{
  Result()->StdOut.append(data.begin(), data.end());
}

void cmUVReadOnlyProcess::UVPipeOutEnd(ssize_t error)
{
  // Process pipe error
  if ((error != 0) && !Result()->error()) {
    Result()->ErrorMessage = cmStrCat(
      "Reading from stdout pipe failed with libuv error code ", error);
  }
  // Try finish
  UVTryFinish();
}

void cmUVReadOnlyProcess::UVPipeErrData(cmUVPipeBuffer::DataRange data)
{
  std::string* str =
    Setup_.MergedOutput ? &Result()->StdOut : &Result()->StdErr;
  str->append(data.begin(), data.end());
}

void cmUVReadOnlyProcess::UVPipeErrEnd(ssize_t error)
{
  // Process pipe error
  if ((error != 0) && !Result()->error()) {
    Result()->ErrorMessage = cmStrCat(
      "Reading from stderr pipe failed with libuv error code ", error);
  }
  // Try finish
  UVTryFinish();
}

void cmUVReadOnlyProcess::UVTryFinish()
{
  // There still might be data in the pipes after the process has finished.
  // Therefore check if the process is finished AND all pipes are closed
  // before signaling the worker thread to continue.
  if ((UVProcess_.get() != nullptr) || (UVPipeOut_.uv_pipe() != nullptr) ||
      (UVPipeErr_.uv_pipe() != nullptr)) {
    return;
  }
  IsFinished_ = true;
  FinishedCallback_();
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
  void SetThread(std::thread&& aThread) { Thread_ = std::move(aThread); }

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

private:
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
  Proc_.Request.init(uvLoop, &cmWorkerPoolWorker::UVProcessStart, this);
}

cmWorkerPoolWorker::~cmWorkerPoolWorker()
{
  if (Thread_.joinable()) {
    Thread_.join();
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
    std::lock_guard<std::mutex> lock(Proc_.Mutex);
    Proc_.ROP = cm::make_unique<cmUVReadOnlyProcess>();
    Proc_.ROP->setup(&result, true, command, workingDirectory);
  }
  // Send asynchronous process start request to libuv loop
  Proc_.Request.send();
  // Wait until the process has been finished and destroyed
  {
    std::unique_lock<std::mutex> ulock(Proc_.Mutex);
    while (Proc_.ROP) {
      Proc_.Condition.wait(ulock);
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
  {
    std::lock_guard<std::mutex> lock(Proc_.Mutex);
    if (Proc_.ROP && (Proc_.ROP->IsFinished() || !Proc_.ROP->IsStarted())) {
      Proc_.ROP.reset();
    }
  }
  // Notify idling thread
  Proc_.Condition.notify_one();
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

public:
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
  std::vector<std::unique_ptr<cmWorkerPoolWorker>> Workers;

  // -- References
  cmWorkerPool* Pool = nullptr;
};

void cmWorkerPool::ProcessResultT::reset()
{
  ExitStatus = 0;
  TermSignal = 0;
  if (!StdOut.empty()) {
    StdOut.clear();
    StdOut.shrink_to_fit();
  }
  if (!StdErr.empty()) {
    StdErr.clear();
    StdErr.shrink_to_fit();
  }
  if (!ErrorMessage.empty()) {
    ErrorMessage.clear();
    ErrorMessage.shrink_to_fit();
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
  UVLoop = cm::make_unique<uv_loop_t>();
  uv_loop_init(UVLoop.get());
}

cmWorkerPoolInternal::~cmWorkerPoolInternal()
{
  uv_loop_close(UVLoop.get());
}

bool cmWorkerPoolInternal::Process()
{
  // Reset state flags
  Processing = true;
  Aborting = false;
  // Initialize libuv asynchronous request
  UVRequestBegin.init(*UVLoop, &cmWorkerPoolInternal::UVSlotBegin, this);
  UVRequestEnd.init(*UVLoop, &cmWorkerPoolInternal::UVSlotEnd, this);
  // Send begin request
  UVRequestBegin.send();
  // Run libuv loop
  bool success = (uv_run(UVLoop.get(), UV_RUN_DEFAULT) == 0);
  // Update state flags
  Processing = false;
  Aborting = false;
  return success;
}

void cmWorkerPoolInternal::Abort()
{
  bool notifyThreads = false;
  // Clear all jobs and set abort flag
  {
    std::lock_guard<std::mutex> guard(Mutex);
    if (Processing && !Aborting) {
      // Register abort and clear queue
      Aborting = true;
      Queue.clear();
      notifyThreads = true;
    }
  }
  if (notifyThreads) {
    // Wake threads
    Condition.notify_all();
  }
}

inline bool cmWorkerPoolInternal::PushJob(cmWorkerPool::JobHandleT&& jobHandle)
{
  std::lock_guard<std::mutex> guard(Mutex);
  if (Aborting) {
    return false;
  }
  // Append the job to the queue
  Queue.emplace_back(std::move(jobHandle));
  // Notify an idle worker if there's one
  if (WorkersIdle != 0) {
    Condition.notify_one();
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
  std::unique_lock<std::mutex> uLock(Mutex);
  // Increment running workers count
  ++WorkersRunning;
  // Enter worker main loop
  while (true) {
    // Abort on request
    if (Aborting) {
      break;
    }
    // Wait for new jobs
    if (Queue.empty()) {
      ++WorkersIdle;
      Condition.wait(uLock);
      --WorkersIdle;
      continue;
    }

    // Check for fence jobs
    if (FenceProcessing || Queue.front()->IsFence()) {
      if (JobsProcessing != 0) {
        Condition.wait(uLock);
        continue;
      }
      // No jobs get processed. Set the fence job processing flag.
      FenceProcessing = true;
    }

    // Pop next job from queue
    jobHandle = std::move(Queue.front());
    Queue.pop_front();

    // Unlocked scope for job processing
    ++JobsProcessing;
    {
      uLock.unlock();
      jobHandle->Work(Pool, workerIndex); // Process job
      jobHandle.reset();                  // Destroy job
      uLock.lock();
    }
    --JobsProcessing;

    // Was this a fence job?
    if (FenceProcessing) {
      FenceProcessing = false;
      Condition.notify_all();
    }
  }

  // Decrement running workers count
  if (--WorkersRunning == 0) {
    // Last worker thread about to finish. Send libuv event.
    UVRequestEnd.send();
  }
}

cmWorkerPool::JobT::~JobT() = default;

bool cmWorkerPool::JobT::RunProcess(ProcessResultT& result,
                                    std::vector<std::string> const& command,
                                    std::string const& workingDirectory)
{
  // Get worker by index
  auto* wrk = Pool_->Int_->Workers.at(WorkerIndex_).get();
  return wrk->RunProcess(result, command, workingDirectory);
}

cmWorkerPool::cmWorkerPool()
  : Int_(cm::make_unique<cmWorkerPoolInternal>(this))
{
}

cmWorkerPool::~cmWorkerPool() = default;

void cmWorkerPool::SetThreadCount(unsigned int threadCount)
{
  if (!Int_->Processing) {
    ThreadCount_ = (threadCount > 0) ? threadCount : 1u;
  }
}

bool cmWorkerPool::Process(void* userData)
{
  // Setup user data
  UserData_ = userData;
  // Run libuv loop
  bool success = Int_->Process();
  // Clear user data
  UserData_ = nullptr;
  // Return
  return success;
}

bool cmWorkerPool::PushJob(JobHandleT&& jobHandle)
{
  return Int_->PushJob(std::move(jobHandle));
}

void cmWorkerPool::Abort()
{
  Int_->Abort();
}
