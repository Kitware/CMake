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

#include "cmExportBuildFileGenerator.h"

cmExportCommand::cmExportCommand()
:cmCommand()
,ArgumentGroup()
,Targets(&Helper, "TARGETS")
,Namespace(&Helper, "NAMESPACE", &ArgumentGroup)
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
    return false;
    }

  if (this->Targets.WasFound() == false)
    {
    this->SetError("TARGETS option missing.");
    return false;
    }

  if(!this->Filename.WasFound())
    {
    this->SetError("FILE <filename> option missing.");
    return false;
    }

  // Make sure the file has a .cmake extension.
  if(cmSystemTools::GetFilenameLastExtension(this->Filename.GetCString())
     != ".cmake")
    {
    cmOStringStream e;
    e << "FILE option given filename \"" << this->Filename.GetString()
      << "\" which does not have an extension of \".cmake\".\n";
    this->SetError(e.str().c_str());
    return false;
    }

  // Get the file to write.
  std::string fname = this->Filename.GetString();
  if(cmSystemTools::FileIsFullPath(fname.c_str()))
    {
    if(!this->Makefile->CanIWriteThisFile(fname.c_str()))
      {
      cmOStringStream e;
      e << "FILE option given filename \"" << fname
        << "\" which is in the source tree.\n";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  else
    {
    // Interpret relative paths with respect to the current build dir.
    fname = this->Makefile->GetCurrentOutputDirectory();
    fname += "/";
    fname += this->Filename.GetString();
    }

  // If no targets are to be exported we are done.
  if(this->Targets.GetVector().empty())
    {
    return true;
    }

  // Collect the targets to be exported.
  std::vector<cmTarget*> targets;
  for(std::vector<std::string>::const_iterator
      currentTarget = this->Targets.GetVector().begin();
      currentTarget != this->Targets.GetVector().end();
      ++currentTarget)
    {
    if(cmTarget* target =
       this->Makefile->GetLocalGenerator()->
       GetGlobalGenerator()->FindTarget(0, currentTarget->c_str()))
      {
      if((target->GetType() == cmTarget::EXECUTABLE) ||
         (target->GetType() == cmTarget::STATIC_LIBRARY) ||
         (target->GetType() == cmTarget::SHARED_LIBRARY) ||
         (target->GetType() == cmTarget::MODULE_LIBRARY))
        {
        targets.push_back(target);
        }
      else
        {
        cmOStringStream e;
        e << "given target \"" << *currentTarget
          << "\" which is not an executable or library.";
        this->SetError(e.str().c_str());
        return false;
        }
      }
    else
      {
      cmOStringStream e;
      e << "given target \"" << *currentTarget
        << "\" which is not built by this project.";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Setup export file generation.
  cmExportBuildFileGenerator ebfg;
  ebfg.SetExportFile(fname.c_str());
  ebfg.SetNamespace(this->Namespace.GetCString());
  ebfg.SetExports(&targets);

  // Compute the set of configurations exported.
  if(const char* types =
     this->Makefile->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    std::vector<std::string> configurationTypes;
    cmSystemTools::ExpandListArgument(types, configurationTypes);
    for(std::vector<std::string>::const_iterator
          ci = configurationTypes.begin();
        ci != configurationTypes.end(); ++ci)
      {
      ebfg.AddConfiguration(ci->c_str());
      }
    }
  else if(const char* config =
          this->Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    ebfg.AddConfiguration(config);
    }
  else
    {
    ebfg.AddConfiguration("");
    }

  // Generate the import file.
  if(!ebfg.GenerateImportFile())
    {
    this->SetError("could not write export file.");
    return false;
    }

  return true;
}
