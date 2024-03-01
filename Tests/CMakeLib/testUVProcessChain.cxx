#include <algorithm>
#include <csignal>
#include <cstdio>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include <cm3p/uv.h>

#include "cmGetPipes.h"
#include "cmStringAlgorithms.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"
#include "cmUVStreambuf.h"

struct ExpectedStatus
{
  bool MatchExitStatus;
  bool MatchTermSignal;
  cmUVProcessChain::Status Status;
  cmUVProcessChain::ExceptionCode ExceptionCode;
  std::string ExceptionString;
};

static const char* ExceptionCodeToString(cmUVProcessChain::ExceptionCode code)
{
  switch (code) {
    case cmUVProcessChain::ExceptionCode::None:
      return "None";
    case cmUVProcessChain::ExceptionCode::Fault:
      return "Fault";
    case cmUVProcessChain::ExceptionCode::Illegal:
      return "Illegal";
    case cmUVProcessChain::ExceptionCode::Interrupt:
      return "Interrupt";
    case cmUVProcessChain::ExceptionCode::Numerical:
      return "Numerical";
    case cmUVProcessChain::ExceptionCode::Spawn:
      return "Spawn";
    case cmUVProcessChain::ExceptionCode::Other:
      return "Other";
    default:
      return "";
  }
}

bool operator==(const cmUVProcessChain::Status* actual,
                const ExpectedStatus& expected)
{
  if (expected.Status.SpawnResult != actual->SpawnResult) {
    return false;
  }
  if (expected.Status.Finished != actual->Finished) {
    return false;
  }
  if (expected.MatchExitStatus &&
      expected.Status.ExitStatus != actual->ExitStatus) {
    return false;
  }
  if (expected.MatchTermSignal &&
      expected.Status.TermSignal != actual->TermSignal) {
    return false;
  }
  if (expected.Status.Finished &&
      std::make_pair(expected.ExceptionCode, expected.ExceptionString) !=
        actual->GetException()) {
    return false;
  }
  return true;
}

static bool resultsMatch(
  const std::vector<const cmUVProcessChain::Status*>& actual,
  const std::vector<ExpectedStatus>& expected)
{
  return actual.size() == expected.size() &&
    std::equal(actual.begin(), actual.end(), expected.begin());
}

static std::string getInput(std::istream& input)
{
  char buffer[1024];
  std::ostringstream str;
  do {
    input.read(buffer, 1024);
    str.write(buffer, input.gcount());
  } while (input.gcount() > 0);
  return str.str();
}

template <typename T>
std::function<std::ostream&(std::ostream&)> printExpected(bool match,
                                                          const T& value)
{
  return [match, value](std::ostream& stream) -> std::ostream& {
    if (match) {
      stream << value;
    } else {
      stream << "*";
    }
    return stream;
  };
}

std::ostream& operator<<(
  std::ostream& stream,
  const std::function<std::ostream&(std::ostream&)>& func)
{
  return func(stream);
}

static void printResults(
  const std::vector<const cmUVProcessChain::Status*>& actual,
  const std::vector<ExpectedStatus>& expected)
{
  std::cout << "Expected: " << std::endl;
  for (auto const& e : expected) {
    std::cout << "  SpawnResult: " << e.Status.SpawnResult
              << ", Finished: " << e.Status.Finished << ", ExitStatus: "
              << printExpected(e.MatchExitStatus, e.Status.ExitStatus)
              << ", TermSignal: "
              << printExpected(e.MatchTermSignal, e.Status.TermSignal)
              << ", ExceptionCode: "
              << printExpected(e.Status.Finished,
                               ExceptionCodeToString(e.ExceptionCode))
              << ", ExceptionString: \""
              << printExpected(e.Status.Finished, e.ExceptionString) << '"'
              << std::endl;
  }
  std::cout << "Actual:" << std::endl;
  for (auto const& a : actual) {
    auto exception = a->GetException();
    std::cout << "  SpawnResult: " << a->SpawnResult
              << ", Finished: " << a->Finished
              << ", ExitStatus: " << a->ExitStatus
              << ", TermSignal: " << a->TermSignal
              << ", ExceptionCode: " << ExceptionCodeToString(exception.first)
              << ", ExceptionString: \"" << exception.second << '"'
              << std::endl;
  }
}

static bool checkExecution(cmUVProcessChainBuilder& builder,
                           std::unique_ptr<cmUVProcessChain>& chain)
{
  static const std::vector<ExpectedStatus> status1 = {
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
  };

  static const std::vector<ExpectedStatus> status2 = {
    { true,
      true,
      { 0, true, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
  };

  static const std::vector<ExpectedStatus> status3 = {
    { true,
      true,
      { 0, true, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { true,
      true,
      { 0, true, 1, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
#ifdef _WIN32
    { true,
      true,
      { 0, true, STATUS_ACCESS_VIOLATION, 0 },
      cmUVProcessChain::ExceptionCode::Fault,
      "Access violation" },
#else
    { false,
      true,
      { 0, true, 0, SIGABRT },
      cmUVProcessChain::ExceptionCode::Other,
      "Subprocess aborted" },
#endif
  };

  std::vector<const cmUVProcessChain::Status*> status;

  chain = cm::make_unique<cmUVProcessChain>(builder.Start());
  if (!chain->Valid()) {
    std::cout << "Valid() returned false, should be true" << std::endl;
    return false;
  }
  status = chain->GetStatus();
  if (!resultsMatch(status, status1)) {
    std::cout << "GetStatus() did not produce expected output" << std::endl;
    printResults(status, status1);
    return false;
  }
  if (chain->Finished()) {
    std::cout << "Finished() returned true, should be false" << std::endl;
    return false;
  }

  if (chain->Wait(9000)) {
    std::cout << "Wait() returned true, should be false" << std::endl;
    return false;
  }
  status = chain->GetStatus();
  if (!resultsMatch(status, status2)) {
    std::cout << "GetStatus() did not produce expected output" << std::endl;
    printResults(status, status2);
    return false;
  }
  if (chain->Finished()) {
    std::cout << "Finished() returned true, should be false" << std::endl;
    return false;
  }

  if (!chain->Wait()) {
    std::cout << "Wait() returned false, should be true" << std::endl;
    return false;
  }
  status = chain->GetStatus();
  if (!resultsMatch(status, status3)) {
    std::cout << "GetStatus() did not produce expected output" << std::endl;
    printResults(status, status3);
    return false;
  }
  if (!chain->Finished()) {
    std::cout << "Finished() returned false, should be true" << std::endl;
    return false;
  }

  return true;
}

static bool checkOutput(std::istream& outputStream, std::istream& errorStream)
{
  std::string output = getInput(outputStream);
  if (output != "HELO WRD!") {
    std::cout << "Output was \"" << output << "\", expected \"HELO WRD!\""
              << std::endl;
    return false;
  }

  std::string error = getInput(errorStream);
  auto qemu_error_pos = error.find("qemu:");
  if (qemu_error_pos != std::string::npos) {
    error.resize(qemu_error_pos);
  }
  if (error.length() != 3 || error.find('1') == std::string::npos ||
      error.find('2') == std::string::npos ||
      error.find('3') == std::string::npos) {
    std::cout << "Error was \"" << error << "\", expected \"123\""
              << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainBuiltin(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  std::unique_ptr<cmUVProcessChain> chain;
  builder.AddCommand({ helperCommand, "echo" })
    .AddCommand({ helperCommand, "capitalize" })
    .AddCommand({ helperCommand, "dedup" })
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);

  if (!checkExecution(builder, chain)) {
    return false;
  }

  if (chain->OutputStream() < 0) {
    std::cout << "OutputStream() was invalid, expecting valid" << std::endl;
    return false;
  }
  if (chain->ErrorStream() < 0) {
    std::cout << "ErrorStream() was invalid, expecting valid" << std::endl;
    return false;
  }

  cmUVPipeIStream output(chain->GetLoop(), chain->OutputStream());
  cmUVPipeIStream error(chain->GetLoop(), chain->ErrorStream());

  if (!checkOutput(output, error)) {
    return false;
  }

  return true;
}

bool testUVProcessChainBuiltinMerged(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  std::unique_ptr<cmUVProcessChain> chain;
  builder.AddCommand({ helperCommand, "echo" })
    .AddCommand({ helperCommand, "capitalize" })
    .AddCommand({ helperCommand, "dedup" })
    .SetMergedBuiltinStreams();

  if (!checkExecution(builder, chain)) {
    return false;
  }

  if (chain->OutputStream() < 0) {
    std::cout << "OutputStream() was invalid, expecting valid" << std::endl;
    return false;
  }
  if (chain->ErrorStream() < 0) {
    std::cout << "ErrorStream() was invalid, expecting valid" << std::endl;
    return false;
  }
  if (chain->OutputStream() != chain->ErrorStream()) {
    std::cout << "OutputStream() and ErrorStream() expected to be the same"
              << std::endl;
    return false;
  }

  cmUVPipeIStream mergedStream(chain->GetLoop(), chain->OutputStream());

  std::string merged = getInput(mergedStream);
  auto qemuErrorPos = merged.find("qemu:");
  if (qemuErrorPos != std::string::npos) {
    merged.resize(qemuErrorPos);
  }
  if (merged.length() != cmStrLen("HELO WRD!123") ||
      merged.find('1') == std::string::npos ||
      merged.find('2') == std::string::npos ||
      merged.find('3') == std::string::npos) {
    std::cout << "Expected output to contain '1', '2', and '3', was \""
              << merged << "\"" << std::endl;
    return false;
  }
  std::string output;
  for (auto const& c : merged) {
    if (c != '1' && c != '2' && c != '3') {
      output += c;
    }
  }
  if (output != "HELO WRD!") {
    std::cout << "Output was \"" << output << "\", expected \"HELO WRD!\""
              << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainExternal(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  std::unique_ptr<cmUVProcessChain> chain;
  int outputPipe[2], errorPipe[2];
  cm::uv_pipe_ptr outputInPipe, outputOutPipe, errorInPipe, errorOutPipe;

  if (cmGetPipes(outputPipe) < 0) {
    std::cout << "Error creating pipes" << std::endl;
    return false;
  }
  if (cmGetPipes(errorPipe) < 0) {
    std::cout << "Error creating pipes" << std::endl;
    return false;
  }

  builder.AddCommand({ helperCommand, "echo" })
    .AddCommand({ helperCommand, "capitalize" })
    .AddCommand({ helperCommand, "dedup" })
    .SetExternalStream(cmUVProcessChainBuilder::Stream_OUTPUT, outputPipe[1])
    .SetExternalStream(cmUVProcessChainBuilder::Stream_ERROR, errorPipe[1]);

  if (!checkExecution(builder, chain)) {
    return false;
  }

  if (chain->OutputStream() >= 0) {
    std::cout << "OutputStream() was valid, expecting invalid" << std::endl;
    return false;
  }
  if (chain->ErrorStream() >= 0) {
    std::cout << "ErrorStream() was valid, expecting invalid" << std::endl;
    return false;
  }

  outputOutPipe.init(chain->GetLoop(), 0);
  uv_pipe_open(outputOutPipe, outputPipe[1]);
  outputOutPipe.reset();

  errorOutPipe.init(chain->GetLoop(), 0);
  uv_pipe_open(errorOutPipe, errorPipe[1]);
  errorOutPipe.reset();

  outputInPipe.init(chain->GetLoop(), 0);
  uv_pipe_open(outputInPipe, outputPipe[0]);
  cmUVStreambuf outputBuf;
  outputBuf.open(outputInPipe);
  std::istream outputStream(&outputBuf);

  errorInPipe.init(chain->GetLoop(), 0);
  uv_pipe_open(errorInPipe, errorPipe[0]);
  cmUVStreambuf errorBuf;
  errorBuf.open(errorInPipe);
  std::istream errorStream(&errorBuf);

  if (!checkOutput(outputStream, errorStream)) {
    return false;
  }

  return true;
}

bool testUVProcessChainNone(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  std::unique_ptr<cmUVProcessChain> chain;
  builder.AddCommand({ helperCommand, "echo" })
    .AddCommand({ helperCommand, "capitalize" })
    .AddCommand({ helperCommand, "dedup" });

  if (!checkExecution(builder, chain)) {
    return false;
  }

  if (chain->OutputStream() >= 0) {
    std::cout << "OutputStream() was valid, expecting invalid" << std::endl;
    return false;
  }
  if (chain->ErrorStream() >= 0) {
    std::cout << "ErrorStream() was valid, expecting invalid" << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainCwdUnchanged(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand({ helperCommand, "pwd" })
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);

  auto chain = builder.Start();
  chain.Wait();
  if (chain.GetStatus().front()->ExitStatus != 0) {
    std::cout << "Exit status was " << chain.GetStatus().front()->ExitStatus
              << ", expecting 0" << std::endl;
    return false;
  }

  cmUVPipeIStream output(chain.GetLoop(), chain.OutputStream());
  auto cwd = getInput(output);
  if (!cmHasLiteralSuffix(cwd, "/Tests/CMakeLib")) {
    std::cout << "Working directory was \"" << cwd
              << "\", expected to end in \"/Tests/CMakeLib\"" << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainCwdChanged(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand({ helperCommand, "pwd" })
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR)
    .SetWorkingDirectory("..");

  auto chain = builder.Start();
  chain.Wait();
  if (chain.GetStatus().front()->ExitStatus != 0) {
    std::cout << "Exit status was " << chain.GetStatus().front()->ExitStatus
              << ", expecting 0" << std::endl;
    return false;
  }

  cmUVPipeIStream output(chain.GetLoop(), chain.OutputStream());
  auto cwd = getInput(output);
  if (!cmHasLiteralSuffix(cwd, "/Tests")) {
    std::cout << "Working directory was \"" << cwd
              << "\", expected to end in \"/Tests\"" << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainSpawnFail(const char* helperCommand)
{
  static const std::vector<ExpectedStatus> status1 = {
    { false,
      false,
      { 0, false, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
    { false,
      false,
      { UV_ENOENT, true, 0, 0 },
      cmUVProcessChain::ExceptionCode::Spawn,
      uv_strerror(UV_ENOENT) },
#ifdef _WIN32
    { true,
      true,
      { 0, true, STATUS_ACCESS_VIOLATION, 0 },
      cmUVProcessChain::ExceptionCode::Fault,
      "Access violation" },
#else
    { false,
      true,
      { 0, true, 0, SIGABRT },
      cmUVProcessChain::ExceptionCode::Other,
      "Subprocess aborted" },
#endif
  };

  static const std::vector<ExpectedStatus> status2 = {
#ifdef _WIN32
    { true,
      true,
      { 0, true, 0, 0 },
      cmUVProcessChain::ExceptionCode::None,
      "" },
#else
    { false,
      true,
      { 0, true, 0, SIGPIPE },
      cmUVProcessChain::ExceptionCode::Other,
      "SIGPIPE" },
#endif
    { false,
      false,
      { UV_ENOENT, true, 0, 0 },
      cmUVProcessChain::ExceptionCode::Spawn,
      uv_strerror(UV_ENOENT) },
#ifdef _WIN32
    { true,
      true,
      { 0, true, STATUS_ACCESS_VIOLATION, 0 },
      cmUVProcessChain::ExceptionCode::Fault,
      "Access violation" },
#else
    { false,
      true,
      { 0, true, 0, SIGABRT },
      cmUVProcessChain::ExceptionCode::Other,
      "Subprocess aborted" },
#endif
  };

  std::vector<const cmUVProcessChain::Status*> status;

  cmUVProcessChainBuilder builder;
  builder.AddCommand({ helperCommand, "echo" })
    .AddCommand({ "this_command_is_for_cmake_and_should_never_exist" })
    .AddCommand({ helperCommand, "dedup" })
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);

  auto chain = builder.Start();
  if (!chain.Valid()) {
    std::cout << "Valid() returned false, should be true" << std::endl;
    return false;
  }

  // Some platforms, like Solaris 10, take a long time to report a trapped
  // subprocess to the parent process (about 1.7 seconds in the case of
  // Solaris 10.) Wait 3 seconds to give it enough time.
  if (chain.Wait(3000)) {
    std::cout << "Wait() did not time out" << std::endl;
    return false;
  }

  status = chain.GetStatus();
  if (!resultsMatch(status, status1)) {
    std::cout << "GetStatus() did not produce expected output" << std::endl;
    printResults(status, status1);
    return false;
  }

  if (!chain.Wait()) {
    std::cout << "Wait() timed out" << std::endl;
    return false;
  }

  status = chain.GetStatus();
  if (!resultsMatch(status, status2)) {
    std::cout << "GetStatus() did not produce expected output" << std::endl;
    printResults(status, status2);
    return false;
  }

  return true;
}

bool testUVProcessChainInputFile(const char* helperCommand)
{
  std::unique_ptr<FILE, int (*)(FILE*)> f(
    fopen("testUVProcessChainInput.txt", "rb"), fclose);

  cmUVProcessChainBuilder builder;
  builder.AddCommand({ helperCommand, "dedup" })
    .SetExternalStream(cmUVProcessChainBuilder::Stream_INPUT, f.get())
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT);

  auto chain = builder.Start();

  if (!chain.Wait()) {
    std::cout << "Wait() timed out" << std::endl;
    return false;
  }

  cmUVPipeIStream stream(chain.GetLoop(), chain.OutputStream());
  std::string output = getInput(stream);
  if (output != "HELO WRD!") {
    std::cout << "Output was \"" << output << "\", expected \"HELO WRD!\""
              << std::endl;
    return false;
  }

  return true;
}

bool testUVProcessChainWait0(const char* helperCommand)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand({ helperCommand, "echo" });

  auto chain = builder.Start();
  if (!chain.Wait(0)) {
    std::cout << "Wait(0) returned false, should be true" << std::endl;
    return false;
  }

  return true;
}

int testUVProcessChain(int argc, char** const argv)
{
  if (argc < 2) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  if (!testUVProcessChainBuiltin(argv[1])) {
    std::cout << "While executing testUVProcessChainBuiltin().\n";
    return -1;
  }

  if (!testUVProcessChainBuiltinMerged(argv[1])) {
    std::cout << "While executing testUVProcessChainBuiltinMerged().\n";
    return -1;
  }

  if (!testUVProcessChainExternal(argv[1])) {
    std::cout << "While executing testUVProcessChainExternal().\n";
    return -1;
  }

  if (!testUVProcessChainNone(argv[1])) {
    std::cout << "While executing testUVProcessChainNone().\n";
    return -1;
  }

  if (!testUVProcessChainCwdUnchanged(argv[1])) {
    std::cout << "While executing testUVProcessChainCwdUnchanged().\n";
    return -1;
  }

  if (!testUVProcessChainCwdChanged(argv[1])) {
    std::cout << "While executing testUVProcessChainCwdChanged().\n";
    return -1;
  }

  if (!testUVProcessChainSpawnFail(argv[1])) {
    std::cout << "While executing testUVProcessChainSpawnFail().\n";
    return -1;
  }

  if (!testUVProcessChainInputFile(argv[1])) {
    std::cout << "While executing testUVProcessChainInputFile().\n";
    return -1;
  }

  if (!testUVProcessChainWait0(argv[1])) {
    std::cout << "While executing testUVProcessChainWait0().\n";
    return -1;
  }

  return 0;
}
