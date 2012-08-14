/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmOptionCommand.h"

// cmOptionCommand
bool cmOptionCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  bool argError = false;
  if(args.size() < 2)
    {
    argError = true;
    }
  // for VTK 4.0 we have to support the option command with more than 3
  // arguments if CMAKE_MINIMUM_REQUIRED_VERSION is not defined, if
  // CMAKE_MINIMUM_REQUIRED_VERSION is defined, then we can have stricter
  // checking.
  if(this->Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION"))
    {
    if(args.size() > 3)
      {
      argError = true;
      }
    }
  if(argError)
    {
    std::string m = "called with incorrect number of arguments: ";
    for(size_t i =0; i < args.size(); ++i)
      {
      m += args[i];
      m += " ";
      }
    this->SetError(m.c_str());
    return false;
    }

  std::string initialValue = "Off";
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  cmCacheManager::CacheIterator it =
    this->Makefile->GetCacheManager()->GetCacheIterator(args[0].c_str());
  if(!it.IsAtEnd())
    {
    if ( it.GetType() != cmCacheManager::UNINITIALIZED )
      {
      it.SetProperty("HELPSTRING", args[1].c_str());
      return true;
      }
    if ( it.GetValue() )
      {
      initialValue = it.GetValue();
      }
    }
  if(args.size() == 3)
    {
    initialValue = args[2];
    }
  bool init = cmSystemTools::IsOn(initialValue.c_str());
  this->Makefile->AddCacheDefinition(args[0].c_str(), init? "ON":"OFF",
                                     args[1].c_str(), cmCacheManager::BOOL);
  return true;
}
