/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVariableRequiresCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmVariableRequiresCommand
::InitialPass(std::vector<std::string>const& args, cmExecutionStatus &)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string testVariable = args[0]; 
  if(!this->Makefile->IsOn(testVariable.c_str()))
    {
    return true;
    }
  std::string resultVariable = args[1];
  bool requirementsMet = true;
  std::string notSet;
  bool hasAdvanced = false;
  for(unsigned int i = 2; i < args.size(); ++i)
    {
    if(!this->Makefile->IsOn(args[i].c_str()))
      {
      requirementsMet = false;
      notSet += args[i];
      notSet += "\n";
      cmCacheManager::CacheIterator it = 
        this->Makefile->GetCacheManager()->GetCacheIterator(args[i].c_str());
      if(!it.IsAtEnd() && it.GetPropertyAsBool("ADVANCED"))
        {
        hasAdvanced = true;
        }
      }
    }
  const char* reqVar = this->Makefile->GetDefinition(resultVariable.c_str());
  // if reqVar is unset, then set it to requirementsMet 
  // if reqVar is set to true, but requirementsMet is false , then
  // set reqVar to false.
  if(!reqVar || (!requirementsMet && this->Makefile->IsOn(reqVar)))
    {
    this->Makefile->AddDefinition(resultVariable.c_str(), requirementsMet);
    }

  if(!requirementsMet)
    {
    std::string message = "Variable assertion failed:\n";
    message += testVariable + 
      " Requires that the following unset variables are set:\n";
    message += notSet;
    message += "\nPlease set them, or set ";
    message += testVariable + " to false, and re-configure.\n";
    if(hasAdvanced)
      {
      message += 
        "One or more of the required variables is advanced."
        "  To set the variable, you must turn on advanced mode in cmake.";
      }
    cmSystemTools::Error(message.c_str());
    }

  return true;
}
