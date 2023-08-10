/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <vector>

#include "cmDebuggerAdapter.h"
#include "cmDebuggerProtocol.h"
#include "cmListFileCache.h"
#include "cmMessenger.h"
#include <cmcppdap/include/dap/io.h>
#include <cmcppdap/include/dap/session.h>
#include <cmcppdap/include/dap/types.h>

#include "testCommon.h"

#define ASSERT_VARIABLE(x, expectedName, expectedValue, expectedType)         \
  do {                                                                        \
    ASSERT_TRUE(x.name == expectedName);                                      \
    ASSERT_TRUE(x.value == expectedValue);                                    \
    if (expectedType == nullptr) {                                            \
      ASSERT_TRUE(x.type == dap::optional<dap::string>());                    \
    } else {                                                                  \
      ASSERT_TRUE(x.type == dap::optional<dap::string>(expectedType));        \
      if (std::string(expectedType) == "collection") {                        \
        ASSERT_TRUE(x.variablesReference != 0);                               \
      }                                                                       \
    }                                                                         \
    ASSERT_TRUE(x.evaluateName.has_value() == false);                         \
  } while (false)

#define ASSERT_VARIABLE_REFERENCE(x, expectedName, expectedValue,             \
                                  expectedType, expectedReference)            \
  do {                                                                        \
    ASSERT_VARIABLE(x, expectedName, expectedValue, expectedType);            \
    ASSERT_TRUE(x.variablesReference == (expectedReference));                 \
  } while (false)

#define ASSERT_VARIABLE_REFERENCE_NOT_ZERO(x, expectedName, expectedValue,    \
                                           expectedType)                      \
  do {                                                                        \
    ASSERT_VARIABLE(x, expectedName, expectedValue, expectedType);            \
    ASSERT_TRUE(x.variablesReference != 0);                                   \
  } while (false)

#define ASSERT_BREAKPOINT(x, expectedId, expectedLine, sourcePath,            \
                          isVerified)                                         \
  do {                                                                        \
    ASSERT_TRUE(x.id.has_value());                                            \
    ASSERT_TRUE(x.id.value() == expectedId);                                  \
    ASSERT_TRUE(x.line.has_value());                                          \
    ASSERT_TRUE(x.line.value() == expectedLine);                              \
    ASSERT_TRUE(x.source.has_value());                                        \
    ASSERT_TRUE(x.source.value().path.has_value());                           \
    ASSERT_TRUE(x.source.value().path.value() == sourcePath);                 \
    ASSERT_TRUE(x.verified == isVerified);                                    \
  } while (false)

class DebuggerTestHelper
{
  std::shared_ptr<dap::ReaderWriter> Client2Debugger = dap::pipe();
  std::shared_ptr<dap::ReaderWriter> Debugger2Client = dap::pipe();

public:
  std::unique_ptr<dap::Session> Client = dap::Session::create();
  std::unique_ptr<dap::Session> Debugger = dap::Session::create();
  void bind()
  {
    auto client2server = dap::pipe();
    auto server2client = dap::pipe();
    Client->bind(server2client, client2server);
    Debugger->bind(client2server, server2client);
  }
  std::vector<cmListFileFunction> CreateListFileFunctions(const char* str,
                                                          const char* filename)
  {
    cmMessenger messenger;
    cmListFileBacktrace backtrace;
    cmListFile listfile;
    listfile.ParseString(str, filename, &messenger, backtrace);
    return listfile.Functions;
  }
};

class ScopedThread
{
public:
  template <class... Args>
  explicit ScopedThread(Args&&... args)
    : Thread(std::forward<Args>(args)...)
  {
  }

  ~ScopedThread()
  {
    if (Thread.joinable())
      Thread.join();
  }

private:
  std::thread Thread;
};
