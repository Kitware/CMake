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

// cmExportCommand
bool cmExportCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with too few arguments");
    return false;
    }

  std::string filename;
  std::string prefix;
  std::string exportName;
  std::vector<std::string> targets;
  bool append = false;
  if (!this->ParseArgs(args, filename, prefix, exportName, targets, append))
    {
    return false;
    }

  if ( !this->Makefile->CanIWriteThisFile(filename.c_str()) )
    {
    std::string e = "attempted to write a file: " + filename
      + " into a source directory.";
    this->SetError(e.c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  if ((targets.empty()) || (filename.empty()))
    {
    return true;
    }

  // Use copy-if-different if not appending.
  cmsys::auto_ptr<std::ofstream> foutPtr;
  if(append)
    {
    cmsys::auto_ptr<std::ofstream> ap(
      new std::ofstream(filename.c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    cmsys::auto_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(filename.c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  std::ostream& fout = *foutPtr.get();

  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", filename.c_str());
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

  for(std::vector<std::string>::const_iterator currentTarget = targets.begin();
      currentTarget != targets.end();
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

  for(std::vector<std::string>::const_iterator currentTarget = targets.begin();
      currentTarget != targets.end();
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
          fout << "ADD_EXECUTABLE(" << prefix.c_str() << currentTarget->c_str()
                                                              << " IMPORT )\n";
          break;
        case cmTarget::STATIC_LIBRARY:
          fout << "ADD_LIBRARY(" << prefix.c_str() << currentTarget->c_str()
                                                       << " STATIC IMPORT )\n";
          break;
        case cmTarget::SHARED_LIBRARY:
          fout << "ADD_LIBRARY(" << prefix.c_str() << currentTarget->c_str()
                                                       << " SHARED IMPORT )\n";
          break;
        case cmTarget::MODULE_LIBRARY:
          fout << "ADD_LIBRARY(" << prefix.c_str() << currentTarget->c_str()
                                                       << " MODULE IMPORT )\n";
          break;
        default:  // should never happen
          break;
        }

      fout << "SET_TARGET_PROPERTIES(" << prefix.c_str()
                                 << currentTarget->c_str() << " PROPERTIES \n"
        << "                      LOCATION " << target->GetLocation(0) << "\n";
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
                                                << "_LOCATION " << loc << "\n";
            }
          }
        }
      fout << "                     )\n\n";
      }
    }

  return true;
}

bool cmExportCommand::ParseArgs(const std::vector<std::string>& args,
                                std::string& filename,
                                std::string& prefix,
                                std::string& exportName,
                                std::vector<std::string>& targets,
                                bool& append) const
{
  bool doingFile = false;
  bool doingPrefix = false;
  bool doingTargets = false;
  bool doingName = true;
  for(std::vector<std::string>::const_iterator it = args.begin();
      it != args.end();
      ++it)
    {
    if (*it == "FILE")
      {
      doingFile = true;
      doingPrefix = false;
      doingName = false;
      doingTargets = false;
      }
    else if (*it == "PREFIX")
      {
      doingFile = false;
      doingPrefix = true;
      doingName = false;
      doingTargets = false;
      }
    else if (*it == "TARGETS")
      {
      doingFile = false;
      doingPrefix = false;
      doingName = false;
      doingTargets = true;
      }
    else if (*it == "APPEND")
      {
      append = true;
      doingFile = false;
      doingPrefix = false;
      doingName = false;
      doingTargets = false;
      }
    else if (doingFile)
      {
      filename = *it;
      doingFile = false;
      doingPrefix = false;
      doingName = false;
      doingTargets = false;
      }
    else if (doingPrefix)
      {
      prefix = *it;
      doingFile = false;
      doingPrefix = false;
      doingName = false;
      doingTargets = false;
      }
    else if (doingTargets)
      {
      targets.push_back(*it);
      }
    else if (doingName)
      {
      exportName = *it;
      doingFile = false;
      doingPrefix = false;
      doingName = false;
      doingTargets = false;
      }
    else
      {
      }
    }
  return true;
}
