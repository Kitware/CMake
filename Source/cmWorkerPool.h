/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmWorkerPool_h
#define cmWorkerPool_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmAlgorithms.h" // IWYU pragma: keep

#include <memory> // IWYU pragma: keep
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

// -- Types
class cmWorkerPoolInternal;

/** @class cmWorkerPool
 * @brief Thread pool with job queue
 */
class cmWorkerPool
{
public:
  /**
   * Return value and output of an external process.
   */
  struct ProcessResultT
  {
    void reset();
    bool error() const
    {
      return (ExitStatus != 0) || (TermSignal != 0) || !ErrorMessage.empty();
    }

    std::int64_t ExitStatus = 0;
    int TermSignal = 0;
    std::string StdOut;
    std::string StdErr;
    std::string ErrorMessage;
  };

  /**
   * Abstract job class for concurrent job processing.
   */
  class JobT
  {
  public:
    JobT(JobT const&) = delete;
    JobT& operator=(JobT const&) = delete;

    /**
     * @brief Virtual destructor.
     */
    virtual ~JobT();

    /**
     * @brief Fence job flag
     *
     * Fence jobs require that:
     * - all jobs before in the queue have been processed
     * - no jobs later in the queue will be processed before this job was
     *   processed
     */
    bool IsFence() const { return Fence_; }

  protected:
    /**
     * @brief Protected default constructor
     */
    JobT(bool fence = false)
      : Fence_(fence)
    {
    }

    /**
     * Abstract processing interface that must be implement in derived classes.
     */
    virtual void Process() = 0;

    /**
     * Get the worker pool.
     * Only valid during the JobT::Process() call!
     */
    cmWorkerPool* Pool() const { return Pool_; }

    /**
     * Get the user data.
     * Only valid during the JobT::Process() call!
     */
    void* UserData() const { return Pool_->UserData(); };

    /**
     * Get the worker index.
     * This is the index of the thread processing this job and is in the range
     * [0..ThreadCount).
     * Concurrently processing jobs will never have the same WorkerIndex().
     * Only valid during the JobT::Process() call!
     */
    unsigned int WorkerIndex() const { return WorkerIndex_; }

    /**
     * Run an external read only process.
     * Use only during JobT::Process() call!
     */
    bool RunProcess(ProcessResultT& result,
                    std::vector<std::string> const& command,
                    std::string const& workingDirectory);

  private:
    //! Needs access to Work()
    friend class cmWorkerPoolInternal;
    //! Worker thread entry method.
    void Work(cmWorkerPool* pool, unsigned int workerIndex)
    {
      Pool_ = pool;
      WorkerIndex_ = workerIndex;
      this->Process();
    }

  private:
    cmWorkerPool* Pool_ = nullptr;
    unsigned int WorkerIndex_ = 0;
    bool Fence_ = false;
  };

  /**
   * @brief Job handle type
   */
  typedef std::unique_ptr<JobT> JobHandleT;

  /**
   * @brief Fence job base class
   */
  class JobFenceT : public JobT
  {
  public:
    JobFenceT()
      : JobT(true)
    {
    }
    //! Does nothing
    void Process() override{};
  };

  /**
   * @brief Fence job that aborts the worker pool.
   * This class is useful as the last job in the job queue.
   */
  class JobEndT : JobFenceT
  {
  public:
    //! Does nothing
    void Process() override { Pool()->Abort(); }
  };

public:
  // -- Methods
  cmWorkerPool();
  ~cmWorkerPool();

  /**
   * @brief Blocking function that starts threads to process all Jobs in
   *        the queue.
   *
   * This method blocks until a job calls the Abort() method.
   * @arg threadCount Number of threads to process jobs.
   * @arg userData Common user data pointer available in all Jobs.
   */
  bool Process(unsigned int threadCount, void* userData = nullptr);

  /**
   * Number of worker threads passed to Process().
   * Only valid during Process().
   */
  unsigned int ThreadCount() const { return ThreadCount_; }

  /**
   * User data reference passed to Process().
   * Only valid during Process().
   */
  void* UserData() const { return UserData_; }

  // -- Job processing interface

  /**
   * @brief Clears the job queue and aborts all worker threads.
   *
   * This method is thread safe and can be called from inside a job.
   */
  void Abort();

  /**
   * @brief Push job to the queue.
   *
   * This method is thread safe and can be called from inside a job or before
   * Process().
   */
  bool PushJob(JobHandleT&& jobHandle);

  /**
   * @brief Push job to the queue
   *
   * This method is thread safe and can be called from inside a job or before
   * Process().
   */
  template <class T, typename... Args>
  bool EmplaceJob(Args&&... args)
  {
    return PushJob(cm::make_unique<T>(std::forward<Args>(args)...));
  }

private:
  void* UserData_ = nullptr;
  unsigned int ThreadCount_ = 0;
  std::unique_ptr<cmWorkerPoolInternal> Int_;
};

#endif
