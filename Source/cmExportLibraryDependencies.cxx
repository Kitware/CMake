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

#include <memory> // auto_ptr

// cmExecutableCommand
bool cmExportLibraryDependenciesCommand::InitialPass(std::vector<std::string> const& args)
{
  // First argument is the name of the test
  // Second argument is the name of the executable to run (a target or external
  //    program)
  // Remaining arguments are the arguments to pass to the executable
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the arguments for the final pass
  // also expand any CMake variables

  this->Args = args;
  return true;
}


void cmExportLibraryDependenciesCommand::FinalPass()
{
  // Create a full path filename for output
  std::string fname = this->Args[0];
  bool append = false;
  if(this->Args.size() > 1)
    {
    if(this->Args[1] == "APPEND")
      {
      append = true;
      }
    }

  // Use copy-if-different if not appending.
  std::auto_ptr<std::ofstream> foutPtr;
  if(append)
    {
    std::auto_ptr<std::ofstream> ap(
      new std::ofstream(fname.c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    std::auto_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(fname.c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  std::ostream& fout = *foutPtr.get();

  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  cmake* cm = this->Makefile->GetCMakeInstance();
  cmGlobalGenerator* global = cm->GetGlobalGenerator();
  std::vector<cmLocalGenerator *> locals;
  global->GetLocalGenerators(locals);
  std::string libDepName;
  for(std::vector<cmLocalGenerator *>::iterator i = locals.begin();
      i != locals.end(); ++i)
    {
    cmLocalGenerator* gen = *i;
    cmTargets &tgts = gen->GetMakefile()->GetTargets();  
    std::vector<std::string> depends;
    const char *defType;
    for(cmTargets::iterator l = tgts.begin();
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
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
    }
  return;
}
