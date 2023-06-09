#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <cmext/algorithm>

#include <cm3p/uv.h>
#include <stdint.h>

#include "cmGetPipes.h"
#include "cmUVHandlePtr.h"
#include "cmUVStream.h"
#include "cmUVStreambuf.h"

#define TEST_STR_LINE_1 "This string must be exactly 128 characters long so"
#define TEST_STR_LINE_2 "that we can test CMake's std::streambuf integration"
#define TEST_STR_LINE_3 "with libuv's uv_stream_t."
#define TEST_STR TEST_STR_LINE_1 "\n" TEST_STR_LINE_2 "\n" TEST_STR_LINE_3

static bool writeDataToStreamPipe(uv_loop_t& loop, cm::uv_pipe_ptr& inputPipe,
                                  char* outputData,
                                  unsigned int outputDataLength,
                                  const char* /* unused */)
{
  int err;

  // Create the pipe
  int pipeHandles[2];
  if (cmGetPipes(pipeHandles) < 0) {
    std::cout << "Could not open pipe" << std::endl;
    return false;
  }

  cm::uv_pipe_ptr outputPipe;
  inputPipe.init(loop, 0);
  outputPipe.init(loop, 0);
  uv_pipe_open(inputPipe, pipeHandles[0]);
  uv_pipe_open(outputPipe, pipeHandles[1]);

  // Write data for reading
  uv_write_t writeReq;
  struct WriteCallbackData
  {
    bool Finished = false;
    int Status;
  } writeData;
  writeReq.data = &writeData;
  uv_buf_t outputBuf;
  outputBuf.base = outputData;
  outputBuf.len = outputDataLength;
  if ((err = uv_write(&writeReq, outputPipe, &outputBuf, 1,
                      [](uv_write_t* req, int status) {
                        auto data = static_cast<WriteCallbackData*>(req->data);
                        data->Finished = true;
                        data->Status = status;
                      })) < 0) {
    std::cout << "Could not write to pipe: " << uv_strerror(err) << std::endl;
    return false;
  }
  while (!writeData.Finished) {
    uv_run(&loop, UV_RUN_ONCE);
  }
  if (writeData.Status < 0) {
    std::cout << "Status is " << uv_strerror(writeData.Status)
              << ", should be 0" << std::endl;
    return false;
  }

  return true;
}

static bool writeDataToStreamProcess(uv_loop_t& loop,
                                     cm::uv_pipe_ptr& inputPipe,
                                     char* outputData,
                                     unsigned int /* unused */,
                                     const char* cmakeCommand)
{
  int err;

  inputPipe.init(loop, 0);
  std::vector<std::string> arguments = { cmakeCommand, "-E", "echo_append",
                                         outputData };
  std::vector<const char*> processArgs;
  for (auto const& arg : arguments) {
    processArgs.push_back(arg.c_str());
  }
  processArgs.push_back(nullptr);
  std::vector<uv_stdio_container_t> stdio(3);
  stdio[0].flags = UV_IGNORE;
  stdio[0].data.stream = nullptr;
  stdio[1].flags =
    static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
  stdio[1].data.stream = inputPipe;
  stdio[2].flags = UV_IGNORE;
  stdio[2].data.stream = nullptr;

  struct ProcessExitData
  {
    bool Finished = false;
    int64_t ExitStatus;
    int TermSignal;
  } exitData;
  cm::uv_process_ptr process;
  auto options = uv_process_options_t();
  options.file = cmakeCommand;
  options.args = const_cast<char**>(processArgs.data());
  options.flags = UV_PROCESS_WINDOWS_HIDE;
  options.stdio = stdio.data();
  options.stdio_count = static_cast<int>(stdio.size());
  options.exit_cb = [](uv_process_t* handle, int64_t exitStatus,
                       int termSignal) {
    auto data = static_cast<ProcessExitData*>(handle->data);
    data->Finished = true;
    data->ExitStatus = exitStatus;
    data->TermSignal = termSignal;
  };
  if ((err = process.spawn(loop, options, &exitData)) < 0) {
    std::cout << "Could not spawn process: " << uv_strerror(err) << std::endl;
    return false;
  }
  while (!exitData.Finished) {
    uv_run(&loop, UV_RUN_ONCE);
  }
  if (exitData.ExitStatus != 0) {
    std::cout << "Process exit status is " << exitData.ExitStatus
              << ", should be 0" << std::endl;
    return false;
  }
  if (exitData.TermSignal != 0) {
    std::cout << "Process term signal is " << exitData.TermSignal
              << ", should be 0" << std::endl;
    return false;
  }

  return true;
}

static bool testUVStreambufRead(
  bool (*cb)(uv_loop_t& loop, cm::uv_pipe_ptr& inputPipe, char* outputData,
             unsigned int outputDataLength, const char* cmakeCommand),
  const char* cmakeCommand)
{
  char outputData[] = TEST_STR;
  bool success = false;
  cm::uv_loop_ptr loop;
  loop.init();
  cm::uv_pipe_ptr inputPipe;
  std::vector<char> inputData(128);
  std::streamsize readLen;
  std::string line;
  cm::uv_timer_ptr timer;

  // Create the streambuf
  cmUVStreambuf inputBuf(64);
  std::istream inputStream(&inputBuf);
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  // Perform first read test - read all the data
  if (!cb(*loop, inputPipe, outputData, 128, cmakeCommand)) {
    goto end;
  }
  inputBuf.open(inputPipe);
  if (!inputBuf.is_open()) {
    std::cout << "is_open() is false, should be true" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 128) {
    std::cout << "sgetn() returned " << readLen << ", should be 128"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if (std::memcmp(inputData.data(), outputData, 128)) {
    std::cout << "Read data does not match write data" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  inputData.assign(128, char{});
  inputBuf.close();
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }

  // Perform second read test - read some data and then close
  if (!cb(*loop, inputPipe, outputData, 128, cmakeCommand)) {
    goto end;
  }
  inputBuf.open(inputPipe);
  if (!inputBuf.is_open()) {
    std::cout << "is_open() is false, should be true" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 64)) != 64) {
    std::cout << "sgetn() returned " << readLen << ", should be 64"
              << std::endl;
    goto end;
  }
  if (std::memcmp(inputData.data(), outputData, 64)) {
    std::cout << "Read data does not match write data" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 8) {
    std::cout << "in_avail() returned " << readLen << ", should be 8"
              << std::endl;
    goto end;
  }
  inputData.assign(128, char{});
  inputBuf.close();
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  // Perform third read test - read line by line
  if (!cb(*loop, inputPipe, outputData, 128, cmakeCommand)) {
    goto end;
  }
  inputBuf.open(inputPipe);
  if (!inputBuf.is_open()) {
    std::cout << "is_open() is false, should be true" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  if (!std::getline(inputStream, line)) {
    std::cout << "getline returned false, should be true" << std::endl;
    goto end;
  }
  if (line != TEST_STR_LINE_1) {
    std::cout << "Line 1 is \"" << line
              << "\", should be \"" TEST_STR_LINE_1 "\"" << std::endl;
    goto end;
  }

  if (!std::getline(inputStream, line)) {
    std::cout << "getline returned false, should be true" << std::endl;
    goto end;
  }
  if (line != TEST_STR_LINE_2) {
    std::cout << "Line 2 is \"" << line
              << "\", should be \"" TEST_STR_LINE_2 "\"" << std::endl;
    goto end;
  }

  if (!std::getline(inputStream, line)) {
    std::cout << "getline returned false, should be true" << std::endl;
    goto end;
  }
  if (line != TEST_STR_LINE_3) {
    std::cout << "Line 3 is \"" << line
              << "\", should be \"" TEST_STR_LINE_3 "\"" << std::endl;
    goto end;
  }

  if (std::getline(inputStream, line)) {
    std::cout << "getline returned true, should be false" << std::endl;
    goto end;
  }

  inputBuf.close();
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  // Perform fourth read test - run the event loop outside of underflow()
  if (!cb(*loop, inputPipe, outputData, 128, cmakeCommand)) {
    goto end;
  }
  inputBuf.open(inputPipe);
  if (!inputBuf.is_open()) {
    std::cout << "is_open() is false, should be true" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  uv_run(loop, UV_RUN_DEFAULT);
  if ((readLen = inputBuf.in_avail()) != 72) {
    std::cout << "in_avail() returned " << readLen << ", should be 72"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 128) {
    std::cout << "sgetn() returned " << readLen << ", should be 128"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 128"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }

  inputBuf.close();
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  // Perform fifth read test - close the streambuf in the middle of a read
  timer.init(*loop, &inputBuf);
  if (!cb(*loop, inputPipe, outputData, 128, cmakeCommand)) {
    goto end;
  }
  inputBuf.open(inputPipe);
  if (!inputBuf.is_open()) {
    std::cout << "is_open() is false, should be true" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != 0) {
    std::cout << "in_avail() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  uv_timer_start(
    timer,
    [](uv_timer_t* handle) {
      auto buf = static_cast<cmUVStreambuf*>(handle->data);
      buf->close();
    },
    0, 0);
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }
  if (inputBuf.is_open()) {
    std::cout << "is_open() is true, should be false" << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.in_avail()) != -1) {
    std::cout << "in_avail() returned " << readLen << ", should be -1"
              << std::endl;
    goto end;
  }
  if ((readLen = inputBuf.sgetn(inputData.data(), 128)) != 0) {
    std::cout << "sgetn() returned " << readLen << ", should be 0"
              << std::endl;
    goto end;
  }

  success = true;

end:
  return success;
}

bool testUVPipeIStream()
{
  int pipe[] = { -1, -1 };
  if (cmGetPipes(pipe) < 0) {
    std::cout << "cmGetPipes() returned an error" << std::endl;
    return false;
  }

  cm::uv_loop_ptr loop;
  loop.init();
  cm::uv_pipe_ptr pipeSink;
  pipeSink.init(*loop, 0);
  uv_pipe_open(pipeSink, pipe[1]);

  std::string str = "Hello world!\n";
  uv_write_t writeReq;
  uv_buf_t buf;
  buf.base = &str.front();
  buf.len = str.length();
  uv_write(&writeReq, pipeSink, &buf, 1, nullptr);
  uv_run(loop, UV_RUN_DEFAULT);

  cmUVPipeIStream pin(*loop, pipe[0]);
  std::string line;
  std::getline(pin, line);
  if (line != "Hello world!") {
    std::cout << "Line was \"" << line << "\", should be \"Hello world!\""
              << std::endl;
    return false;
  }

  return true;
}

bool testUVStreamRead()
{
  int pipe[] = { -1, -1 };
  if (cmGetPipes(pipe) < 0) {
    std::cout << "cmGetPipes() returned an error" << std::endl;
    return false;
  }

  cm::uv_loop_ptr loop;
  loop.init();
  cm::uv_pipe_ptr pipeSink;
  pipeSink.init(*loop, 0);
  uv_pipe_open(pipeSink, pipe[1]);

  std::string str = "Hello world!";
  uv_write_t writeReq;
  uv_buf_t buf;
  buf.base = &str.front();
  buf.len = str.length();
  uv_write(&writeReq, pipeSink, &buf, 1, nullptr);
  uv_run(loop, UV_RUN_DEFAULT);
  pipeSink.reset();

  cm::uv_pipe_ptr pipeSource;
  pipeSource.init(*loop, 0);
  uv_pipe_open(pipeSource, pipe[0]);

  std::string output;
  bool finished = false;
  cmUVStreamRead(
    pipeSource,
    [&output](std::vector<char> data) { cm::append(output, data); },
    [&output, &finished]() {
      if (output != "Hello world!") {
        std::cout << "Output was \"" << output
                  << "\", should be \"Hello world!\"" << std::endl;
        return;
      }
      finished = true;
    });
  uv_run(loop, UV_RUN_DEFAULT);

  if (!finished) {
    std::cout << "finished was not set" << std::endl;
    return false;
  }

  return true;
}

int testUVStreambuf(int argc, char** const argv)
{
  if (argc < 2) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  if (!testUVStreambufRead(writeDataToStreamPipe, argv[1])) {
    std::cout << "While executing testUVStreambufRead() with pipe.\n";
    return -1;
  }

  if (!testUVStreambufRead(writeDataToStreamProcess, argv[1])) {
    std::cout << "While executing testUVStreambufRead() with process.\n";
    return -1;
  }

  if (!testUVPipeIStream()) {
    std::cout << "While executing testUVPipeIStream().\n";
    return -1;
  }

  if (!testUVStreamRead()) {
    std::cout << "While executing testUVPipeIStream().\n";
    return -1;
  }

  return 0;
}
