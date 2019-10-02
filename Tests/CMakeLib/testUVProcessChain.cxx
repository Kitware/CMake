#include <algorithm>
#include <csignal>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cm/memory>

#include "cm_uv.h"

#include "cmGetPipes.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStreambuf.h"

struct ExpectedStatus
{
  bool Finished;
  bool MatchExitStatus;
  bool MatchTermSignal;
  cmUVProcessChain::Status Status;
};

static const std::vector<ExpectedStatus> status1 = {
  { false, false, false, { 0, 0 } },
  { false, false, false, { 0, 0 } },
  { false, false, false, { 0, 0 } },
};

static const std::vector<ExpectedStatus> status2 = {
  { true, true, true, { 0, 0 } },
  { false, false, false, { 0, 0 } },
  { false, false, false, { 0, 0 } },
};

static const std::vector<ExpectedStatus> status3 = {
  { true, true, true, { 0, 0 } },
  { true, true, true, { 1, 0 } },
#ifdef _WIN32
  { true, true, true, { 2, 0 } },
#else
  { true, false, true, { 0, SIGABRT } },
#endif
};

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
  return true;
}

bool resultsMatch(const std::vector<const cmUVProcessChain::Status*>& actual,
                  const std::vector<ExpectedStatus>& expected)
{
  return actual.size() == expected.size() &&
    std::equal(actual.begin(), actual.end(), expected.begin());
}

std::string getInput(std::istream& input)
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

void printResults(const std::vector<const cmUVProcessChain::Status*>& actual,
                  const std::vector<ExpectedStatus>& expected)
{
  std::cout << "Expected: " << std::endl;
  for (auto const& e : expected) {
    if (e.Finished) {
      std::cout << "  ExitStatus: "
                << printExpected(e.MatchExitStatus, e.Status.ExitStatus)
                << ", TermSignal: "
                << printExpected(e.MatchTermSignal, e.Status.TermSignal)
                << std::endl;
    } else {
      std::cout << "  null" << std::endl;
    }
  }
  std::cout << "Actual:" << std::endl;
  for (auto const& a : actual) {
    if (a) {
      std::cout << "  ExitStatus: " << a->ExitStatus
                << ", TermSignal: " << a->TermSignal << std::endl;
    } else {
      std::cout << "  null" << std::endl;
    }
  }
}

bool checkExecution(cmUVProcessChainBuilder& builder,
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

  return true;
}

bool checkOutput(std::istream& outputStream, std::istream& errorStream)
{
  std::string output = getInput(outputStream);
  if (output != "HELO WRD!") {
    std::cout << "Output was \"" << output << "\", expected \"HELO WRD!\""
              << std::endl;
    return false;
  }

  std::string error = getInput(errorStream);
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

  if (!testUVProcessChainExternal(argv[1])) {
    std::cout << "While executing testUVProcessChainExternal().\n";
    return -1;
  }

  if (!testUVProcessChainNone(argv[1])) {
    std::cout << "While executing testUVProcessChainNone().\n";
    return -1;
  }

  return 0;
}
