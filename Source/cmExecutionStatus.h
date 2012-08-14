/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecutionStatus_h
#define cmExecutionStatus_h

#include "cmObject.h"

/** \class cmExecutionStatus
 * \brief Superclass for all command status classes
 *
 * when a command is involked it may set values on a command status instance
 */
class cmExecutionStatus : public cmObject
{
public:
  cmTypeMacro(cmExecutionStatus, cmObject);

  cmExecutionStatus() { this->Clear();};

  virtual void SetReturnInvoked(bool val)
  { this->ReturnInvoked = val; }
  virtual bool GetReturnInvoked()
  { return this->ReturnInvoked; }

  virtual void SetBreakInvoked(bool val)
  { this->BreakInvoked = val; }
  virtual bool GetBreakInvoked()
  { return this->BreakInvoked; }

  virtual void Clear()
    {
    this->ReturnInvoked = false;
    this->BreakInvoked = false;
    this->NestedError = false;
    }
  virtual void SetNestedError(bool val) { this->NestedError = val; }
  virtual bool GetNestedError() { return this->NestedError; }


protected:
  bool ReturnInvoked;
  bool BreakInvoked;
  bool NestedError;
};

#endif
