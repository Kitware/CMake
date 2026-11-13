#include <cassert>
#include <cstddef>
#include <deque>
#include <functional>
#include <iostream>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/uv.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#include "cmGetPipes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVJobServerClient.h"

namespace {

std::size_t const kTOTAL_JOBS = 10;
std::size_t const kTOTAL_TOKENS = 3;

#ifdef _WIN32
std::size_t NextSemaphoreId = 0;

struct JobServerSemaphore
{
  std::string Name;
  HANDLE Handle = nullptr;

  JobServerSemaphore(LONG initialCount, LONG maximumCount)
    : Name(cmStrCat("cmake_test_jobserver_"_s, GetCurrentProcessId(), '_',
                    ++NextSemaphoreId))
  {
    SetLastError(ERROR_SUCCESS);
    this->Handle = CreateSemaphoreA(nullptr, initialCount, maximumCount,
                                    this->Name.c_str());
    if (this->Handle && GetLastError() == ERROR_ALREADY_EXISTS) {
      CloseHandle(this->Handle);
      this->Handle = nullptr;
    }
  }

  ~JobServerSemaphore()
  {
    if (this->Handle) {
      CloseHandle(this->Handle);
    }
  }

  JobServerSemaphore(JobServerSemaphore const&) = delete;
  JobServerSemaphore& operator=(JobServerSemaphore const&) = delete;

  explicit operator bool() const { return this->Handle != nullptr; }

  bool CountAvailableTokens(std::size_t& count) const
  {
    count = 0;
    for (;;) {
      DWORD const result = WaitForSingleObject(this->Handle, 0);
      if (result == WAIT_OBJECT_0) {
        ++count;
      } else if (result == WAIT_TIMEOUT) {
        break;
      } else {
        return false;
      }
    }

    return count == 0 ||
      ReleaseSemaphore(this->Handle, static_cast<LONG>(count), nullptr);
  }
};

struct MakeFlagsGuard
{
  cm::optional<std::string> Original = cmSystemTools::GetEnvVar("MAKEFLAGS");

  ~MakeFlagsGuard()
  {
    if (this->Original) {
      cmSystemTools::PutEnv(cmStrCat("MAKEFLAGS="_s, *this->Original));
    } else {
      cmSystemTools::UnsetEnv("MAKEFLAGS");
    }
  }
};

void SetJobServer(JobServerSemaphore const& jobServer)
{
  cmSystemTools::PutEnv(cmStrCat("MAKEFLAGS=--flags-before"
                                 " --jobserver-auth=bogus"
                                 " --flags-between"
                                 " --jobserver-auth="_s,
                                 jobServer.Name, " --flags-after"_s));
}

bool CheckTokenCount(JobServerSemaphore const& jobServer, std::size_t expected)
{
  std::size_t count;
  if (!jobServer.CountAvailableTokens(count)) {
    std::cerr << "Failed to inspect job server semaphore\n";
    return false;
  }
  if (count != expected) {
    std::cerr << "Expected " << expected << " job server tokens, got " << count
              << '\n';
    return false;
  }
  return true;
}
#endif

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
    return this->Okay;
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
  JobServerSemaphore jobServer(kTOTAL_TOKENS - 1, kTOTAL_TOKENS - 1);
  if (!jobServer) {
    std::cerr << "Failed to create job server semaphore\n";
    return false;
  }
  SetJobServer(jobServer);
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
                                 " --jobserver-fds="_s,
                                 jobServerPipe[0], ',', jobServerPipe[1],
                                 " --flags-after"_s));
#endif

  bool passed;
  {
    JobRunner jobRunner;
    passed = jobRunner.Run();
  }
#ifdef _WIN32
  passed = CheckTokenCount(jobServer, kTOTAL_TOKENS - 1) && passed;
#endif
  return passed;
}

#ifdef _WIN32
bool ConnectsWithMakeFlags(std::string const& makeFlags)
{
  cmSystemTools::PutEnv(cmStrCat("MAKEFLAGS="_s, makeFlags));
  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }
  cm::optional<cmUVJobServerClient> client =
    cmUVJobServerClient::Connect(*loop, nullptr, nullptr);
  return client.has_value();
}

bool testJobServerParsing()
{
  JobServerSemaphore jobServer(1, 1);
  if (!jobServer) {
    std::cerr << "Failed to create parsing test semaphore\n";
    return false;
  }

  bool passed = true;
  passed =
    ConnectsWithMakeFlags(cmStrCat("--jobserver-auth="_s, jobServer.Name)) &&
    passed;
  passed = ConnectsWithMakeFlags(cmStrCat("--jobserver-auth=bogus "
                                          "--jobserver-fds=1,2 "
                                          "--jobserver-auth="_s,
                                          jobServer.Name)) &&
    passed;
  passed =
    !ConnectsWithMakeFlags(cmStrCat("--jobserver-auth="_s, jobServer.Name,
                                    " --jobserver-fds=1,2"_s)) &&
    passed;
  passed = !ConnectsWithMakeFlags("--jobserver-auth=fifo:somewhere") && passed;
  passed = !ConnectsWithMakeFlags("--jobserver-auth=") && passed;
  cmSystemTools::UnsetEnv("MAKEFLAGS");
  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }
  passed = !cmUVJobServerClient::Connect(*loop, nullptr, nullptr) && passed;

  if (!passed) {
    std::cerr << "Job server MAKEFLAGS parsing test failed\n";
  }
  return passed;
}

bool testDeferredImplicitTokenOrdering()
{
  JobServerSemaphore jobServer(1, 1);
  if (!jobServer) {
    return false;
  }
  SetJobServer(jobServer);

  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }

  std::size_t deliveries = 0;
  cm::optional<cmUVJobServerClient> client;
  client = cmUVJobServerClient::Connect(
    *loop,
    [&]() {
      ++deliveries;
      client->ReleaseToken();
    },
    nullptr);
  if (!client) {
    return false;
  }

  client->RequestToken();
  client->RequestToken();
  bool passed = deliveries == 0;
  uv_run(loop, UV_RUN_DEFAULT);

  passed = deliveries == 2 && client->GetHeldTokens() == 0 &&
    client->GetNeedTokens() == 0 && CheckTokenCount(jobServer, 1) && passed;
  if (!passed) {
    std::cerr << "Deferred implicit-token ordering test failed\n";
  }
  return passed;
}

bool testTimerRestartFromCallback()
{
  JobServerSemaphore jobServer(2, 2);
  if (!jobServer) {
    return false;
  }
  SetJobServer(jobServer);

  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }

  std::size_t deliveries = 0;
  cm::optional<cmUVJobServerClient> client;
  client = cmUVJobServerClient::Connect(
    *loop,
    [&]() {
      ++deliveries;
      if (deliveries == 2) {
        client->ReleaseToken();
        client->RequestToken();
      } else if (deliveries == 3) {
        client->ReleaseToken();
      }
    },
    nullptr);
  if (!client) {
    return false;
  }

  client->RequestToken();
  uv_run(loop, UV_RUN_DEFAULT);
  bool passed = deliveries == 1 && client->GetHeldTokens() == 1;

  client->RequestToken();
  uv_run(loop, UV_RUN_DEFAULT);
  passed = deliveries == 3 && client->GetHeldTokens() == 1 &&
    client->GetNeedTokens() == 0 && passed;
  client->ReleaseToken();
  passed = CheckTokenCount(jobServer, 2) && passed;
  if (!passed) {
    std::cerr << "Timer restart test failed\n";
  }
  return passed;
}

bool testBoundedTokenDrain()
{
  std::size_t const explicitTokens = 40;
  JobServerSemaphore jobServer(explicitTokens, explicitTokens);
  if (!jobServer) {
    return false;
  }
  SetJobServer(jobServer);

  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }

  std::size_t deliveries = 0;
  cm::optional<cmUVJobServerClient> client =
    cmUVJobServerClient::Connect(*loop, [&]() { ++deliveries; }, nullptr);
  if (!client) {
    return false;
  }

  client->RequestToken();
  uv_run(loop, UV_RUN_DEFAULT);
  for (std::size_t i = 0; i < explicitTokens; ++i) {
    client->RequestToken();
  }

  uv_run(loop, UV_RUN_NOWAIT);
  bool passed = deliveries == 33 && client->GetNeedTokens() == 8;
  uv_run(loop, UV_RUN_DEFAULT);
  passed = deliveries == explicitTokens + 1 &&
    client->GetHeldTokens() == static_cast<int>(explicitTokens + 1) &&
    client->GetNeedTokens() == 0 && passed;

  for (std::size_t i = 0; i < explicitTokens + 1; ++i) {
    client->ReleaseToken();
  }
  passed = CheckTokenCount(jobServer, explicitTokens) && passed;
  if (!passed) {
    std::cerr << "Bounded token drain test failed\n";
  }
  return passed;
}

bool testPendingRequestTeardown()
{
  JobServerSemaphore jobServer(0, 1);
  if (!jobServer) {
    return false;
  }
  SetJobServer(jobServer);

  cm::uv_loop_ptr loop;
  if (loop.init(nullptr) != 0) {
    return false;
  }

  std::size_t deliveries = 0;
  {
    cm::optional<cmUVJobServerClient> client =
      cmUVJobServerClient::Connect(*loop, [&]() { ++deliveries; }, nullptr);
    if (!client) {
      return false;
    }

    client->RequestToken();
    uv_run(loop, UV_RUN_NOWAIT);
    client->RequestToken();
    uv_run(loop, UV_RUN_NOWAIT);
    if (deliveries != 1 || client->GetHeldTokens() != 1 ||
        client->GetNeedTokens() != 1) {
      std::cerr << "Pending request setup failed\n";
      return false;
    }
  }

  uv_run(loop, UV_RUN_DEFAULT);
  bool const passed = CheckTokenCount(jobServer, 0);
  if (!passed) {
    std::cerr << "Pending request teardown test failed\n";
  }
  return passed;
}
#endif
}

int testUVJobServerClient(int, char** const)
{
#ifdef _WIN32
  MakeFlagsGuard makeFlagsGuard;
#endif
  bool passed = true;
  passed = testJobServer() && passed;
#ifdef _WIN32
  passed = testJobServerParsing() && passed;
  passed = testDeferredImplicitTokenOrdering() && passed;
  passed = testTimerRestartFromCallback() && passed;
  passed = testBoundedTokenDrain() && passed;
  passed = testPendingRequestTeardown() && passed;
#endif
  return passed ? 0 : -1;
}
