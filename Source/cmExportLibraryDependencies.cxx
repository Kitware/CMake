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
#include "cmake.h"

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

  m_Args = args;
  return true;
}


void cmExportLibraryDependenciesCommand::FinalPass()
{
  // don't do anything if local mode
  if(m_Makefile->GetLocal())
    {
    return;
    }
    

  // Create a full path filename for output Testfile
  std::string fname = m_Args[0];
  bool append = false;
  if(m_Args.size() > 1)
    {
    if(m_Args[1] == "APPEND")
      {
      append = true;
      }
    }
  // Open the output Testfile
  std::ofstream fout;
  if(append)
    {
    fout.open(fname.c_str(), std::ios::app);
    }
  else
    {
      fout.open(fname.c_str());
    }
  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    return;
    }
  cmake* cm = m_Makefile->GetCMakeInstance();
  cmGlobalGenerator* global = cm->GetGlobalGenerator();
  std::vector<cmLocalGenerator *> locals;
  global->GetLocalGenerators(locals);
  std::string libDepName;
  for(std::vector<cmLocalGenerator *>::iterator i = locals.begin();
      i != locals.end(); ++i)
    {
    cmLocalGenerator* gen = *i;
    cmTargets &tgts = gen->GetMakefile()->GetTargets();  
    for(cmTargets::const_iterator l = tgts.begin();
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        libDepName = l->first;
        libDepName += "_LIB_DEPENDS";
        const char* def = m_Makefile->GetDefinition(libDepName.c_str());
        if(def)
          {
          fout << "SET(" << libDepName << " \"" << def << "\")\n";
          }
        }
      }
    }
  fout.close();
  return;
}
