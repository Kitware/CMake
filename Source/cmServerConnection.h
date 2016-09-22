/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>
  Copyright 2016 Tobias Hunger <tobias.hunger@qt.io>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#pragma once

#include <string>
#include <vector>

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_uv.h"
#endif

class cmServer;
class LoopGuard;

class cmServerConnection
{
public:
  cmServerConnection();
  virtual ~cmServerConnection();

  void SetServer(cmServer* s);

  bool ProcessEvents(std::string* errorMessage);

  void ReadData(const std::string& data);
  void HandleEof();
  void WriteData(const std::string& data);
  void ProcessNextRequest();

  virtual void Connect(uv_stream_t* server) { (void)(server); }

protected:
  virtual bool DoSetup(std::string* errorMessage) = 0;
  virtual void TearDown() = 0;

  void SendGreetings();

  uv_loop_t* Loop() const { return mLoop; }

protected:
  std::string RawReadBuffer;
  std::string RequestBuffer;

  uv_stream_t* ReadStream = nullptr;
  uv_stream_t* WriteStream = nullptr;

private:
  uv_loop_t* mLoop = nullptr;
  cmServer* Server = nullptr;

  friend class LoopGuard;
};

class cmServerStdIoConnection : public cmServerConnection
{
public:
  bool DoSetup(std::string* errorMessage) override;

  void TearDown() override;

private:
  typedef union
  {
    uv_tty_t tty;
    uv_pipe_t pipe;
  } InOutUnion;

  InOutUnion Input;
  InOutUnion Output;
};

class cmServerPipeConnection : public cmServerConnection
{
public:
  cmServerPipeConnection(const std::string& name);
  bool DoSetup(std::string* errorMessage) override;

  void TearDown() override;

  void Connect(uv_stream_t* server) override;

private:
  const std::string PipeName;
  uv_pipe_t ServerPipe;
  uv_pipe_t ClientPipe;
};
