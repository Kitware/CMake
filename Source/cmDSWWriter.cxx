/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmDSWWriter.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmDSPWriter.h"
#include "cmMSProjectGenerator.h"
#include "cmCacheManager.h"


cmDSWWriter::cmDSWWriter(cmMakefile* m)
{
  m_Makefile = m;
}

// output the DSW file
void cmDSWWriter::OutputDSWFile()
{ 
  // if this is an out of source build, create the output directory
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating output directory for DSW file",
                           m_Makefile->GetStartOutputDirectory());
      }
    }
  // create the dsw file name
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_Makefile->GetProjectName()))
    {
    fname += m_Makefile->GetProjectName();
    }
  else
    {
    fname += "Project";
    }
  fname += ".dsw";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open DSW file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteDSWFile(fout);
}


// Write a DSW file to the stream
void cmDSWWriter::WriteDSWFile(std::ostream& fout)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  
  // Create a list of cmMakefile created from all the
  // CMakeLists.txt files that are in sub directories of
  // this one.
  std::vector<cmMakefile*> allListFiles;
  // add this makefile to the list
  allListFiles.push_back(m_Makefile);
  // add a special target that depends on ALL projects for easy build
  // of Debug only
  m_Makefile->AddUtilityCommand("ALL_BUILD", "echo", "\"Build all projects\"",
                                false);
  m_Makefile->FindSubDirectoryCMakeListsFiles(allListFiles);
  // For each cmMakefile, create a DSP for it, and
  // add it to this DSW file
  for(std::vector<cmMakefile*>::iterator k = allListFiles.begin();
      k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSProjectGenerator* pg = 0;
    // if not this makefile, then create a new generator
    if(m_Makefile != mf)
      {
      // Create an MS generator with DSW off, so it only creates dsp files
      pg = new cmMSProjectGenerator;
      }
    else
      {
      pg = (cmMSProjectGenerator*)m_Makefile->GetMakefileGenerator();
      }
    // make sure the generator is building dsp files
    pg->BuildDSWOff();
    mf->SetMakefileGenerator(pg);
    mf->GenerateMakefile();
    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // Get the home directory with the trailing slash
    std::string homedir = m_Makefile->GetHomeDirectory();
    homedir += "/";
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the cmDSPWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetDSPWriter()->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetDSPWriter()->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == m_Makefile)
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD")
          {
          for(std::vector<cmMakefile*>::iterator a = allListFiles.begin();
              a != allListFiles.end(); ++a)
            {
            const cmTargets &atgts = (*a)->GetTargets();
            for(cmTargets::const_iterator al = atgts.begin();
                al != atgts.end(); ++al)
              {
              if (al->second.IsInAll())
                {
                if (al->second.GetType() == cmTarget::UTILITY)
                  {
                  l->second.AddUtility(al->first.c_str());
                  }
                else
                  {
                  l->second.GetLinkLibraries().push_back(
                    cmTarget::LinkLibraries::value_type(al->first, cmTarget::GENERAL));
                  }
                }
              }
            }
          }
        }
      // Write the project into the DSW file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
      {
        cmCustomCommand cc = l->second.GetCustomCommands()[0];
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends = cc.GetOutputs();
        this->WriteExternalProject(fout, stuff[0].c_str(), stuff[1].c_str(), depends);
        ++si;
      }
      else if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProject(fout, si->c_str(), dir.c_str(), 
                           pg->GetDSPWriter(),l->second);
        ++si;
        }
      }
    // delete the cmMakefile which also deletes the cmMSProjectGenerator
    if(mf != m_Makefile)
      {
      delete mf;
      }
    }

  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmDSWWriter::WriteProject(std::ostream& fout, 
				 const char* dspname,
				 const char* dir,
                                 cmDSPWriter*,
                                 const cmTarget& target
                                 )
{
  fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << dspname << "\"=" 
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";

  // insert Begin Project Dependency  Project_Dep_Name project stuff here 
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    cmTarget::LinkLibraries::const_iterator j, jend;
    j = target.GetLinkLibraries().begin();
    jend = target.GetLinkLibraries().end();
    for(;j!= jend; ++j)
      {
      if(j->first != dspname)
	{
        // is the library part of this DSW ? If so add dependency
        const char* cacheValue
          = m_Makefile->GetDefinition(j->first.c_str());
        if(cacheValue)
          {
          fout << "Begin Project Dependency\n";
          fout << "Project_Dep_Name " << j->first << "\n";
          fout << "End Project Dependency\n";
          }
        }
      }
    }

  std::set<std::string>::const_iterator i, end;
  // write utility dependencies.
  i = target.GetUtilities().begin();
  end = target.GetUtilities().end();
  for(;i!= end; ++i)
    {
    if(*i != dspname)
      {
      fout << "Begin Project Dependency\n";
      fout << "Project_Dep_Name " << *i << "\n";
      fout << "End Project Dependency\n";
      }
    }
  fout << "}}}\n\n";
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmDSWWriter::WriteExternalProject(std::ostream& fout, 
			       const char* name,
			       const char* location,
                               const std::vector<std::string>& dependencies)
{
 fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << name << "\"=" 
       << location << " - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";

  
  std::vector<std::string>::const_iterator i, end;
  // write dependencies.
  i = dependencies.begin();
  end = dependencies.end();
  for(;i!= end; ++i)
  {
    fout << "Begin Project Dependency\n";
    fout << "Project_Dep_Name " << *i << "\n";
    fout << "End Project Dependency\n";
  }
  fout << "}}}\n\n";
}



// Standard end of dsw file
void cmDSWWriter::WriteDSWFooter(std::ostream& fout)
{
  fout << "######################################################"
    "#########################\n\n";
  fout << "Global:\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<3>\n{{{\n}}}\n\n";
  fout << "#####################################################"
    "##########################\n\n";
}

  
// ouput standard header for dsw file
void cmDSWWriter::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}
