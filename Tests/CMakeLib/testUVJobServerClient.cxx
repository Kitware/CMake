#include <cassert>
#include <cstddef>
#include <deque>
#include <iostream>
#include <vector>

#include <cm/optional>

#include <cm3p/uv.h>

#ifndef _WIN32
#  include <unistd.h>
#endif

#include "cmGetPipes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVJobServerClient.h"

namespace {

const std::size_t kTOTAL_JOBS = 10;
const std::size_t kTOTAL_TOKENS = 3;

struct Job
{
  cm::uv_timer_ptr Timer;
};

struct JobRunner
{
  cm::uv_loop_ptr Loop;
  cm::optional<cmUVJobServerClient> JSC;
  std::vector<Job> Jobs;
  std::size_t NextJobIndex = 0;

  std::size_t ActiveJobs = 0;

  std::deque<std::size_t> Queue;

  bool Okay = true;

  JobRunner()
    : Jobs(kTOTAL_JOBS)
  {
    this->Loop.init(nullptr);
    this->JSC = cmUVJobServerClient::Connect(
      *this->Loop, [this]() { this->StartQueuedJob(); }, nullptr);
    if (!this->JSC) {
      std::cerr << "Failed to connect to job server.\n";
      this->Okay = false;
    }
  }

  ~JobRunner() {}

  bool Run()
  {
    if (this->Okay) {
      this->QueueNextJobs();
      uv_run(this->Loop, UV_RUN_DEFAULT);
      std::cerr << "HeldTokens: " << this->JSC->GetHeldTokens() << '\n';
      std::cerr << "NeedTokens: " << this->JSC->GetNeedTokens() << '\n';
    }
#ifdef _WIN32
    // FIXME: Windows job server client not yet implemented.
    return true;
#else
    return this->Okay;
#endif
  }

  void QueueNextJobs()
  {
    std::cerr << "QueueNextJobs()\n";
    std::size_t queued = 0;
    while (queued < 2 && this->NextJobIndex < this->Jobs.size()) {
      this->QueueJob(this->NextJobIndex);
      ++this->NextJobIndex;
      ++queued;
    }
    std::cerr << "QueueNextJobs done\n";
  }

  void StartQueuedJob()
  {
    std::cerr << "StartQueuedJob()\n";
    assert(!this->Queue.empty());

    std::size_t index = this->Queue.front();
    this->Queue.pop_front();
    this->StartJob(index);

    std::cerr << "StartQueuedJob done\n";
  }

  void StartJob(std::size_t index)
  {
    cm::uv_timer_ptr& job = this->Jobs[index].Timer;
    job.init(*this->Loop, this);
    uv_timer_start(
      job,
      [](uv_timer_t* handle) {
        uv_timer_stop(handle);
        auto self = static_cast<JobRunner*>(handle->data);
        self->FinishJob();
      },
      /*timeout_ms=*/10 * (1 + (index % 3)), /*repeat_ms=*/0);
    ++this->ActiveJobs;
    std::cerr << "  StartJob(" << index
              << "): Active jobs: " << this->ActiveJobs << '\n';

    if (this->ActiveJobs > kTOTAL_TOKENS) {
      std::cerr << "Started more than " << kTOTAL_TOKENS << " jobs at once!\n";
      this->Okay = false;
      return;
    }
  }

  void QueueJob(std::size_t index)
  {
    this->JSC->RequestToken();
    this->Queue.push_back(index);
    std::cerr << "  QueueJob(" << index
              << "): Queue length: " << this->Queue.size() << '\n';
  }

  void FinishJob()
  {
    --this->ActiveJobs;
    std::cerr << "FinishJob: Active jobs: " << this->ActiveJobs << '\n';

    this->JSC->ReleaseToken();
    this->QueueNextJobs();
  }
};

bool testJobServer()
{
#ifdef _WIN32
  // FIXME: Windows job server client not yet implemented.
#else
  // Create a job server pipe.
  int jobServerPipe[2];
  if (cmGetPipes(jobServerPipe) < 0) {
    std::cerr << "Failed to create job server pipe\n";
    return false;
  }

  // Write N-1 tokens to the pipe.
  std::vector<char> jobServerInit(kTOTAL_TOKENS - 1, '.');
  if (write(jobServerPipe[1], jobServerInit.data(), jobServerInit.size()) !=
      kTOTAL_TOKENS - 1) {
    std::cerr << "Failed to initialize job server pipe\n";
    return false;
  }

  // Establish the job server client context.
  // Add a bogus server spec to verify we use the last spec.
  cmSystemTools::PutEnv(cmStrCat("MAKEFLAGS=--flags-before"
                                 " --jobserver-auth=bogus"
                                 " --flags-between"
                                 " --jobserver-fds=",
                                 jobServerPipe[0], ',', jobServerPipe[1],
                                 " --flags-after"));
#endif

  JobRunner jobRunner;
  return jobRunner.Run();
}
}

int testUVJobServerClient(int, char** const)
{
  bool passed = true;
  passed = testJobServer() && passed;
  return passed ? 0 : -1;
}
