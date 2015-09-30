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

#include "cmStandardIncludes.h"

class cmake;
class cmMetadataServer;

class cmServerProtocol
{
public:
  cmServerProtocol(cmMetadataServer* server, std::string buildDir);
  ~cmServerProtocol();

  void processRequest(const std::string& json);

private:
  void ProcessHandshake(const std::string& protocolVersion);
  void ProcessVersion();
  void ProcessBuildsystem();

private:
  cmMetadataServer* Server;
  cmake* CMakeInstance;
  std::string m_buildDir;
};
