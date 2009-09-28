/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmIncludeExternalMSProjectCommand.h"

// cmIncludeExternalMSProjectCommand
bool cmIncludeExternalMSProjectCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2) 
  {
  this->SetError("INCLUDE_EXTERNAL_MSPROJECT called with incorrect "
                 "number of arguments");
  return false;
  }
// only compile this for win32 to avoid coverage errors
#ifdef _WIN32
  if(this->Makefile->GetDefinition("WIN32"))
    {
    std::string path = args[1];
    cmSystemTools::ConvertToUnixSlashes(path);

    // Create a target instance for this utility.
    cmTarget* target=this->Makefile->AddNewTarget(cmTarget::UTILITY, 
                                                  args[0].c_str());
    target->SetProperty("EXTERNAL_MSPROJECT", path.c_str());
    target->SetProperty("EXCLUDE_FROM_ALL","FALSE"); 
    target->SetProperty("GENERATOR_FILE_NAME", args[0].c_str());
    for (unsigned int i=2; i<args.size(); ++i) 
      {
      target->AddUtility(args[i].c_str());
      }
    }
#endif
  return true;
}
