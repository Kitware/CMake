/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmExportCommand.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"

#include <cmsys/auto_ptr.hxx>

cmExportCommand::cmExportCommand()
:cmCommand()
,ArgumentGroup()
,Targets(&Helper, "TARGETS")
,Append(&Helper, "APPEND", &ArgumentGroup)
,Prefix(&Helper, "PREFIX", &ArgumentGroup)
,Filename(&Helper, "FILE", &ArgumentGroup)
{
  // at first TARGETS
  this->Targets.Follows(0);
  // and after that the other options in any order
  this->ArgumentGroup.Follows(&this->Targets);
}


// cmExportCommand
bool cmExportCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with too few arguments");
    return false;
    }

  std::vector<std::string> unknownArgs;
  this->Helper.Parse(&args, &unknownArgs);

  if (!unknownArgs.empty())
    {
    this->SetError("Unknown arguments.");
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  if (this->Targets.WasFound() == false)
    {
    this->SetError("TARGETS option missing.");
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }


  if ( !this->Makefile->CanIWriteThisFile(this->Filename.GetString().c_str()) )
    {
    std::string e = "attempted to write a file: " + this->Filename.GetString()
      + " into a source directory.";
    this->SetError(e.c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  if((this->Targets.GetVector().empty())||(this->Filename.GetString().empty()))
    {
    return true;
    }

  // Use copy-if-different if not appending.
  cmsys::auto_ptr<std::ofstream> foutPtr;
  if(this->Append.IsEnabled())
    {
    cmsys::auto_ptr<std::ofstream> ap(
      new std::ofstream(this->Filename.GetString().c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    cmsys::auto_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(this->Filename.GetString().c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  std::ostream& fout = *foutPtr.get();

  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", this->Filename.GetString().c_str());
    cmSystemTools::ReportLastSystemError("");
    return true;
    }

  // the following code may move into an "export generator"
  // Compute the set of configurations.
  std::vector<std::string> configurationTypes;
  if(const char* types =
     this->Makefile->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(types, configurationTypes);
    }
  if(configurationTypes.empty())
    {
    const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
    if (config!=0)
      {
      configurationTypes.push_back(config);
      }
    }

  for(std::vector<std::string>::const_iterator 
      currentTarget = this->Targets.GetVector().begin();
      currentTarget != this->Targets.GetVector().end();
      ++currentTarget)
    {
    cmTarget* target = this->Makefile->GetLocalGenerator()->
             GetGlobalGenerator()->FindTarget(0, currentTarget->c_str(), true);
    if (target == 0)
      {
      std::string e = "detected unknown target: " + *currentTarget;
      this->SetError(e.c_str());
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }
    }

  for(std::vector<std::string>::const_iterator 
      currentTarget = this->Targets.GetVector().begin();
      currentTarget != this->Targets.GetVector().end();
      ++currentTarget)
    {
    // Look for a CMake target with the given name, which is an executable
    // and which can be run
    cmTarget* target = this->Makefile->GetLocalGenerator()->
             GetGlobalGenerator()->FindTarget(0, currentTarget->c_str(), true);
    if ((target != 0)
       && ((target->GetType() == cmTarget::EXECUTABLE)
        || (target->GetType() == cmTarget::STATIC_LIBRARY)
        || (target->GetType() == cmTarget::SHARED_LIBRARY)
        || (target->GetType() == cmTarget::MODULE_LIBRARY)))
      {
      switch (target->GetType())
        {
        case cmTarget::EXECUTABLE:
          fout << "ADD_EXECUTABLE(" 
               << this->Prefix.GetString().c_str() << currentTarget->c_str()
               << " IMPORT )\n";
          break;
        case cmTarget::STATIC_LIBRARY:
          fout << "ADD_LIBRARY(" 
               << this->Prefix.GetString().c_str() << currentTarget->c_str()
               << " STATIC IMPORT )\n";
          break;
        case cmTarget::SHARED_LIBRARY:
          fout << "ADD_LIBRARY(" 
               << this->Prefix.GetString().c_str() << currentTarget->c_str()
               << " SHARED IMPORT )\n";
          break;
        case cmTarget::MODULE_LIBRARY:
          fout << "ADD_LIBRARY(" 
               << this->Prefix.GetString().c_str() << currentTarget->c_str()
               << " MODULE IMPORT )\n";
          break;
        default:  // should never happen
          break;
        }

      fout << "SET_TARGET_PROPERTIES(" << this->Prefix.GetString().c_str()
                                 << currentTarget->c_str() << " PROPERTIES \n"
        <<"                      LOCATION \""<< target->GetLocation(0)<<"\"\n";
      for(std::vector<std::string>::const_iterator
          currentConfig = configurationTypes.begin();
          currentConfig != configurationTypes.end();
          ++currentConfig)
        {
        if (!currentConfig->empty())
          {
          const char* loc = target->GetLocation(currentConfig->c_str());
          if (loc && *loc)
            {
            fout << "                      " << currentConfig->c_str()
                                            << "_LOCATION \"" << loc << "\"\n";
            }
          }
        }
      fout << "                     )\n\n";
      }
    }

  return true;
}
