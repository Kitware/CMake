#include <algorithm>
#include <csignal>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cm/memory>

#include <cm3p/uv.h>

#include "cmGetPipes.h"
#include "cmStringAlgorithms.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStreambuf.h"

struct ExpectedStatus
{
  bool Finished;
  bool MatchExitStatus;
  bool MatchTermSignal;
  cmUVProcessChain::Status Status;
  cmUVProcessChain::ExceptionCode ExceptionCode;
  std::string ExceptionString;
};

static const std::vector<ExpectedStatus> status1 = {
  { false, false, false, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
  { false, false, false, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
  { false, false, false, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
};

static const std::vector<ExpectedStatus> status2 = {
  { true, true, true, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
  { false, false, false, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
  { false, false, false, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
};

static const std::vector<ExpectedStatus> status3 = {
  { true, true, true, { 0, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
  { true, true, true, { 1, 0 }, cmUVProcessChain::ExceptionCode::None, "" },
#ifdef _WIN32
  { true,
    true,
    true,
    { STATUS_ACCESS_VIOLATION, 0 },
    cmUVProcessChain::ExceptionCode::Fault,
    "Access violation" },
#else
  { true,
    false,
    true,
    { 0, SIGABRT },
    cmUVProcessChain::ExceptionCode::Other,
    "Subprocess aborted" },
#endif
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
    case cmUVProcessChain::ExceptionCode::Other:
      return "Other";
    default:
      return "";
  }
}

bool operator==(const cmUVProcessChain::Status* actual,
                const ExpectedStatus& expected)
{
  if (!expected.Finished) {
    return !actual;
  } else if (!actual) {
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
  if (expected.Finished &&
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
    if (e.Finished) {
      std::cout << "  ExitStatus: "
                << printExpected(e.MatchExitStatus, e.Status.ExitStatus)
                << ", TermSignal: "
                << printExpected(e.MatchTermSignal, e.Status.TermSignal)
                << ", ExceptionCode: "
                << printExpected(e.Finished,
                                 ExceptionCodeToString(e.ExceptionCode))
                << ", ExceptionString: \""
                << printExpected(e.Finished, e.ExceptionString) << '"'
                << std::endl;
    } else {
      std::cout << "  null" << std::endl;
    }
  }
  std::cout << "Actual:" << std::endl;
  for (auto const& a : actual) {
    if (a) {
      auto exception = a->GetException();
      std::cout << "  ExitStatus: " << a->ExitStatus
                << ", TermSignal: " << a->TermSignal << ", ExceptionCode: "
                << ExceptionCodeToString(exception.first)
                << ", ExceptionString: \"" << exception.second << '"'
                << std::endl;
    } else {
      std::cout << "  null" << std::endl;
    }
  }
}

static bool checkExecution(cmUVProcessChainBuilder& builder,
                           std::unique_ptr<cmUVProcessChain>& chain)
{
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

  if (chain->Wait(6000)) {
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

  if (!chain->OutputStream()) {
    std::cout << "OutputStream() was null, expecting not null" << std::endl;
    return false;
  }
  if (!chain->ErrorStream()) {
    std::cout << "ErrorStream() was null, expecting not null" << std::endl;
    return false;
  }

  if (!checkOutput(*chain->OutputStream(), *chain->ErrorStream())) {
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

  if (!chain->OutputStream()) {
    std::cout << "OutputStream() was null, expecting not null" << std::endl;
    return false;
  }
  if (!chain->ErrorStream()) {
    std::cout << "ErrorStream() was null, expecting not null" << std::endl;
    return false;
  }
  if (chain->OutputStream() != chain->ErrorStream()) {
    std::cout << "OutputStream() and ErrorStream() expected to be the same"
              << std::endl;
    return false;
  }

  std::string merged = getInput(*chain->OutputStream());
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

  if (chain->OutputStream()) {
    std::cout << "OutputStream() was not null, expecting null" << std::endl;
    return false;
  }
  if (chain->ErrorStream()) {
    std::cout << "ErrorStream() was not null, expecting null" << std::endl;
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

  if (chain->OutputStream()) {
    std::cout << "OutputStream() was not null, expecting null" << std::endl;
    return false;
  }
  if (chain->ErrorStream()) {
    std::cout << "ErrorStream() was not null, expecting null" << std::endl;
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

  auto cwd = getInput(*chain.OutputStream());
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

  auto cwd = getInput(*chain.OutputStream());
  if (!cmHasLiteralSuffix(cwd, "/Tests")) {
    std::cout << "Working directory was \"" << cwd
              << "\", expected to end in \"/Tests\"" << std::endl;
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

  return 0;
}
