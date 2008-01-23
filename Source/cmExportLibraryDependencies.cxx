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
#include "cmExportLibraryDependencies.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"

#include <cmsys/auto_ptr.hxx>

bool cmExportLibraryDependenciesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the arguments for the final pass
  this->Filename = args[0];
  this->Append = false;
  if(args.size() > 1)
    {
    if(args[1] == "APPEND")
      {
      this->Append = true;
      }
    }
  return true;
}


void cmExportLibraryDependenciesCommand::FinalPass()
{
  // export_library_dependencies() shouldn't modify anything
  // ensure this by calling a const method
  this->ConstFinalPass();
}

void cmExportLibraryDependenciesCommand::ConstFinalPass() const
{
  // Use copy-if-different if not appending.
  cmsys::auto_ptr<std::ofstream> foutPtr;
  if(this->Append)
    {
    cmsys::auto_ptr<std::ofstream> ap(
      new std::ofstream(this->Filename.c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    cmsys::auto_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(this->Filename.c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  std::ostream& fout = *foutPtr.get();

  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", this->Filename.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  const cmake* cm = this->Makefile->GetCMakeInstance();
  const cmGlobalGenerator* global = cm->GetGlobalGenerator();
  const std::vector<cmLocalGenerator *>& locals = global->GetLocalGenerators();
  std::string libDepName;
  for(std::vector<cmLocalGenerator *>::const_iterator i = locals.begin();
      i != locals.end(); ++i)
    {
    const cmLocalGenerator* gen = *i;
    const cmTargets &tgts = gen->GetMakefile()->GetTargets();  
    std::vector<std::string> depends;
    const char *defType;
    for(cmTargets::const_iterator l = tgts.begin();
        l != tgts.end(); ++l)
      {
      libDepName = l->first;
      libDepName += "_LIB_DEPENDS";
      const char* def = this->Makefile->GetDefinition(libDepName.c_str());
      if(def)
        {
        fout << "SET(" << libDepName << " \"" << def << "\")\n";
        // now for each dependency, check for link type
        cmSystemTools::ExpandListArgument(def, depends);
        for(std::vector<std::string>::const_iterator d = depends.begin();
            d != depends.end(); ++d)
          {
          libDepName = *d;
          libDepName += "_LINK_TYPE";
          defType = this->Makefile->GetDefinition(libDepName.c_str());
          libDepName = cmSystemTools::EscapeSpaces(libDepName.c_str());
          if(defType)
            {
            fout << "SET(" << libDepName << " \"" << defType << "\")\n";
            }
          }
        }
      }
    }
  return;
}
