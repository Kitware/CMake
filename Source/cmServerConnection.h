/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmConnection.h"
#include "cmPipeConnection.h"
#include "cm_uv.h"

class cmServerBase;

/***
 * This connection buffer strategy accepts messages in the form of
 * [== "CMake Server" ==[
{
  ... some JSON message ...
}
]== "CMake Server" ==]
 * and only passes on the core json; it discards the envelope.
 */
class cmServerBufferStrategy : public cmConnectionBufferStrategy
{
public:
  std::string BufferMessage(std::string& rawBuffer) override;

private:
  std::string RequestBuffer;
};

/***
 * Generic connection over std io interfaces -- tty
 */
class cmStdIoConnection : public cmEventBasedConnection
{
public:
  cmStdIoConnection(cmConnectionBufferStrategy* bufferStrategy);

  void SetServer(cmServerBase* s) override;

  bool OnConnectionShuttingDown() override;

  bool OnServeStart(std::string* pString) override;

private:
  typedef union
  {
    uv_tty_t* tty;
    uv_pipe_t* pipe;
  } InOutUnion;

  bool usesTty = false;

  InOutUnion Input;
  InOutUnion Output;
};

/***
 * These specific connections use the cmake server
 * buffering strategy.
 */
class cmServerStdIoConnection : public cmStdIoConnection
{
public:
  cmServerStdIoConnection();
};

class cmServerPipeConnection : public cmPipeConnection
{
public:
  cmServerPipeConnection(const std::string& name);
};
