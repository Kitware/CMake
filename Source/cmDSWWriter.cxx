/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmDSWMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmDSPMakefile.h"
#include "cmMSProjectGenerator.h"


cmDSWMakefile::cmDSWMakefile(cmMakefile* m)
{
  m_Makefile = m;
}

// output the DSW file
void cmDSWMakefile::OutputDSWFile()
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
  fname += m_Makefile->GetProjectName();
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
void cmDSWMakefile::WriteDSWFile(std::ostream& fout)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  
  // Create a list of cmMakefile created from all the
  // CMakeLists.txt files that are in sub directories of
  // this one.
  std::vector<cmMakefile*> allListFiles;
  m_Makefile->FindSubDirectoryCMakeListsFiles(allListFiles);
  
  // For each cmMakefile, create a DSP for it, and
  // add it to this DSW file
  for(std::vector<cmMakefile*>::iterator k = allListFiles.begin();
      k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    // Create an MS generator with DSW off, so it only creates dsp files
    cmMSProjectGenerator* pg = new cmMSProjectGenerator;
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

    // Get the list of create dsp files names from the cmDSPMakefile, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = pg->GetDSPMakefile()->GetCreatedProjectNames();
    const cmTargets &tgts = pg->GetDSPMakefile()->GetMakefile()->GetTargets();
    cmTargets::const_iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l, ++si)
      {
      // Write the project into the DSW file
      this->WriteProject(fout, si->c_str(), dir.c_str(), 
                         pg->GetDSPMakefile(),l->second);
      }
    // delete the cmMakefile which also deletes the cmMSProjectGenerator
    delete mf;
    }
  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmDSWMakefile::WriteProject(std::ostream& fout, 
				 const char* dspname,
				 const char* dir,
                                 cmDSPMakefile* project,
                                 const cmTarget &l)
{
  fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << dspname << "\"=" 
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";

  // insert Begin Project Dependency  Project_Dep_Name project stuff here 
  cmMakefile::LinkLibraries::const_iterator j, jend;
  j = project->GetMakefile()->GetLinkLibraries().begin();
  jend = project->GetMakefile()->GetLinkLibraries().end();
  for(;j!= jend; ++j)
    {
    if(j->first != dspname)
      {
      if (!l.IsALibrary() || 
          project->GetLibraryBuildType() == cmDSPMakefile::DLL)
        {
        fout << "Begin Project Dependency\n";
        fout << "Project_Dep_Name " << j->first << "\n";
        fout << "End Project Dependency\n";
        }
      }
    }

  std::vector<std::string>::iterator i, end;
  // write utility dependencies.
  i = project->GetMakefile()->GetUtilities().begin();
  end = project->GetMakefile()->GetUtilities().end();
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

// Standard end of dsw file
void cmDSWMakefile::WriteDSWFooter(std::ostream& fout)
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
void cmDSWMakefile::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}
