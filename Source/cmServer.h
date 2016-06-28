/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#pragma once

#include "cmListFileCache.h"
#include "cmState.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_value.h"
#include <uv.h>
#endif

class cmServerProtocol;

class cmMetadataServer
{
public:
  enum ServerState
  {
    Uninitialized,
    Started,
    Initializing,
    ProcessingRequests
  };

  cmMetadataServer();

  ~cmMetadataServer();

  void ServeMetadata(const std::string& buildDir);

  bool PopOne();

  void handleData(std::string const& data);

  void WriteResponse(Json::Value const& jsonValue);

  void SetState(ServerState state) { this->State = state; }

  ServerState GetState() const { return this->State; }

private:
  cmServerProtocol* Protocol;
  std::string mBuildDir;
  std::vector<std::string> mQueue;

  std::string mDataBuffer;
  std::string mJsonData;

  uv_loop_t* mLoop;
  uv_pipe_t mStdin_pipe;
  uv_pipe_t mStdout_pipe;
  uv_tty_t mStdin_tty;
  uv_tty_t mStdout_tty;
  uv_pipe_t mLogFile_pipe;

  ServerState State;
  bool Writing;
};
