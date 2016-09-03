/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmMessenger_h
#define cmMessenger_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmListFileCache.h"
#include "cmake.h"

#include <string>

class cmState;

class cmMessenger
{
public:
  cmMessenger(cmState* state);

  void IssueMessage(
    cmake::MessageType t, std::string const& text,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  void DisplayMessage(cmake::MessageType t, std::string const& text,
                      cmListFileBacktrace const& backtrace) const;

  bool GetSuppressDevWarnings() const;
  bool GetSuppressDeprecatedWarnings() const;
  bool GetDevWarningsAsErrors() const;
  bool GetDeprecatedWarningsAsErrors() const;

private:
  bool IsMessageTypeVisible(cmake::MessageType t) const;
  cmake::MessageType ConvertMessageType(cmake::MessageType t) const;

  cmState* State;
};

#endif
