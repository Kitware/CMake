/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTest.h"
#include "cmSystemTools.h"

#include "cmake.h"
#include "cmMakefile.h"

//----------------------------------------------------------------------------
cmTest::cmTest(cmMakefile* mf)
{
  this->Makefile = mf;
  this->OldStyle = true;
  this->Properties.SetCMakeInstance(mf->GetCMakeInstance());
  this->Backtrace = new cmListFileBacktrace;
  this->Makefile->GetBacktrace(*this->Backtrace);
}

//----------------------------------------------------------------------------
cmTest::~cmTest()
{
  delete this->Backtrace;
}

//----------------------------------------------------------------------------
cmListFileBacktrace const& cmTest::GetBacktrace() const
{
  return *this->Backtrace;
}

//----------------------------------------------------------------------------
void cmTest::SetName(const char* name)
{
  if ( !name )
    {
    name = "";
    }
  this->Name = name;
}

//----------------------------------------------------------------------------
void cmTest::SetCommand(std::vector<std::string> const& command)
{
  this->Command = command;
}

//----------------------------------------------------------------------------
const char *cmTest::GetProperty(const char* prop) const
{
  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, cmProperty::TEST, chain);
  if (chain)
    {
    return this->Makefile->GetProperty(prop,cmProperty::TEST);
    }
  return retVal;
}

//----------------------------------------------------------------------------
bool cmTest::GetPropertyAsBool(const char* prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
void cmTest::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  this->Properties.SetProperty(prop, value, cmProperty::TEST);
}

//----------------------------------------------------------------------------
void cmTest::AppendProperty(const char* prop, const char* value, bool asString)
{
  if (!prop)
    {
    return;
    }
  this->Properties.AppendProperty(prop, value, cmProperty::TEST, asString);
}
