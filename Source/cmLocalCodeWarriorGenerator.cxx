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
#include "cmGlobalCodeWarriorGenerator.h"
#include "cmLocalCodeWarriorGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"

cmLocalCodeWarriorGenerator::cmLocalCodeWarriorGenerator()
{
}

cmLocalCodeWarriorGenerator::~cmLocalCodeWarriorGenerator()
{
}


void cmLocalCodeWarriorGenerator::Generate(bool /* fromTheTop */)
{

}

void cmLocalCodeWarriorGenerator::WriteTargets(std::ostream& fout)
{
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    this->WriteTarget(fout,l->first.c_str(),&(l->second));
    }
}

void cmLocalCodeWarriorGenerator::WriteTarget(std::ostream& fout,
                                              const char *tgtName,
                                              cmTarget const *l)
{
  fout << "<TARGET>\n";
  fout << "<NAME>" << tgtName << "</NAME>\n";
  
  this->WriteSettingList(fout, tgtName,l);
  this->WriteFileList(fout, tgtName,l);
  this->WriteLinkOrder(fout, tgtName, l);
  // this->WriteSubTargetList(fout,l);
  
  fout << "</TARGET>\n";
}

void cmLocalCodeWarriorGenerator::AddFileMapping(std::ostream& fout,
                                                 const char *ftype,
                                                 const char *ext,
                                                 const char *comp,
                                                 const char *edit,
                                                 bool precomp,
                                                 bool launch,
                                                 bool res,
                                                 bool ignored)
{
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>FileType</NAME><VALUE>" << ftype << 
    "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FileExtension</NAME><VALUE>" << ext << 
    "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>Compiler</NAME><VALUE>" << comp << 
    "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>EditLanguage</NAME><VALUE>" << edit <<
    "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>Precompile</NAME><VALUE>" 
       << (precomp ? "true" : "false") << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>Launchable</NAME><VALUE>" 
       <<  (launch ? "true" : "false") << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>ResourceFile</NAME><VALUE>" 
       << (res ? "true" : "false") << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>IgnoredByMake</NAME><VALUE>" 
       << (ignored ? "true" : "false") << "</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
}


void cmLocalCodeWarriorGenerator::WriteSettingList(std::ostream& fout,
                                                   const char *tgtName,
                                                   cmTarget const *l)
{
  fout << "<SETTINGLIST>\n";

  fout << "<SETTING><NAME>UserSourceTrees</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>AlwaysSearchUserPaths</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>InterpretDOSAndUnixPaths</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>RequireFrameworkStyleIncludes</NAME><VALUE>false</VALUE></SETTING>\n";
  
  // list the include paths
  fout << "<SETTING><NAME>UserSearchPaths</NAME>\n";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    fout << "<SETTING>\n";
    fout << "<SETTING><NAME>SearchPath</NAME>\n";
    fout << "<SETTING><NAME>Path</NAME><VALUE>" << i->c_str() << "</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    }
  fout << "</SETTING>\n";

  fout << "<SETTING><NAME>SystemSearchPaths</NAME>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:MSL:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>CodeWarrior</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:MacOS Support:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>CodeWarrior</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "</SETTING>\n";

  fout << "<SETTING><NAME>MWRuntimeSettings_WorkingDirectory</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWRuntimeSettings_CommandLine</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWRuntimeSettings_HostApplication</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>MWRuntimeSettings_EnvVars</NAME><VALUE></VALUE></SETTING>\n";

  // <!-- Settings for "Target Settings" panel -->
  fout << "<SETTING><NAME>Linker</NAME><VALUE>MacOS PPC Linker</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PreLinker</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PostLinker</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>Targetname</NAME><VALUE>" << tgtName 
       << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>OutputDirectory</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>SaveEntriesUsingRelativePaths</NAME><VALUE>false</VALUE></SETTING>\n";

  // add the cxx file type
  fout << "<SETTING><NAME>FileMappings</NAME>\n";
  this->AddFileMapping(fout,"TEXT",".cxx","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".c","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".cc","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".cp","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".h","MW C/C++ PPC","C/C++",
                       false,false,false,true);
  this->AddFileMapping(fout,"TEXT",".hpp","MW C/C++ PPC","C/C++",
                       false,false,false,true);
  this->AddFileMapping(fout,"TEXT",".m","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"TEXT",".mm","MW C/C++ PPC","C/C++",
                       false,false,false,false);
  fout << "</SETTING>\n";
  
  // <!-- Settings for "MacOS Merge Panel" panel -->
  fout << "<SETTING><NAME>MWProject_PPC_type</NAME><VALUE>";
  switch (l->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      fout << "Library" << "</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>MWProject_PPC_outfile</NAME><VALUE>";
      fout << tgtName << ".lib";
      fout << "</VALUE></SETTING>\n";
      break;
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      // m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
      fout << "Shared Library" << "</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>MWProject_PPC_outfile</NAME><VALUE>";
      fout << tgtName << ".dylib";
      fout << "</VALUE></SETTING>\n";
      break;
    case cmTarget::EXECUTABLE:
      fout << "Application" << "</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>MWProject_PPC_outfile</NAME><VALUE>";
      fout << tgtName << cmSystemTools::GetExecutableExtension();
      fout << "</VALUE></SETTING>\n";
      break;
    }
  
  fout << "<SETTING><NAME>MWProject_PPC_filecreator</NAME><VALUE>????" << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_filetype</NAME><VALUE>APPL</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_size</NAME><VALUE>384</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_minsize</NAME><VALUE>384</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_stacksize</NAME><VALUE>64</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_flags</NAME><VALUE>22720</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_symfilename</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcname</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcheader</NAME><VALUE>Native</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrctype</NAME><VALUE>????" << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcid</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcflags</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcstore</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_rsrcmerge</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_flatrsrc</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_flatrsrcoutputdir</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>MWProject_PPC_flatrsrcfilename</NAME><VALUE></VALUE></SETTING>\n";

  /*                 
  fout << "<SETTING><NAME>MWMerge_MacOS_outputCreator</NAME><VALUE>????</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_outputType</NAME><VALUE>APPL</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_suppressWarning</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_copyFragments</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_copyResources</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_flattenResource</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_flatFileName</NAME><VALUE>a.rsrc</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_flatFileOutputPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>MWMerge_MacOS_skipResources</NAME><VALUE></VALUE></SETTING>\n";
  */
  
  fout << "</SETTINGLIST>\n";
}

void cmLocalCodeWarriorGenerator::WriteFileList(std::ostream& fout,
                                                const char */*tgtName*/,
                                                cmTarget const *l)
{
  fout << "<FILELIST>\n";

  // for each file
  std::vector<cmSourceFile*> const& classes = l->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
    fout << "<PATHROOT>Absolute</PATHROOT>\n";
    //fout << "<ACCESSPATH>common</ACCESSPATH>\n";
    fout << "<PATH>" << source << "</PATH>\n";
    fout << "<PATHFORMAT>Generic</PATHFORMAT>\n";
    fout << "<FILEKIND>Text</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    }
  
  // now add in the libraries we depend on
  
  
  // now add in the system libs (for an executable)
  if (l->GetType() == cmTarget::EXECUTABLE)
    {
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL RuntimePPC.Lib</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "<FILEKIND>Library</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    }

  fout << "</FILELIST>\n";
}


void cmLocalCodeWarriorGenerator::WriteLinkOrder(std::ostream& fout,
                                                 const char */*tgtName*/,
                                                 cmTarget const *l)
{
  fout << "<LINKORDER>\n";

  // for each file
  std::vector<cmSourceFile*> const& classes = l->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
    fout << "<PATHROOT>Absolute</PATHROOT>\n";
    //fout << "<ACCESSPATH>common</ACCESSPATH>\n";
    fout << "<PATH>" << source << "</PATH>\n";
    fout << "<PATHFORMAT>Generic</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }

  // now add in the system libs (for an executable)
  if (l->GetType() == cmTarget::EXECUTABLE)
    {
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL RuntimePPC.Lib</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }
  
  fout << "</LINKORDER>\n";
}

void cmLocalCodeWarriorGenerator::WriteGroups(std::ostream& fout)
{
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    this->WriteGroup(fout,l->first.c_str(),&(l->second));
    }

}

void cmLocalCodeWarriorGenerator::WriteGroup(std::ostream& fout,
                                             const char *tgtName,
                                             cmTarget const *l)
{
  fout << "<GROUP><NAME>" << tgtName << "</NAME>\n";

  // for each file
  std::vector<cmSourceFile*> const& classes = l->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    std::string source = (*i)->GetFullPath();
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << tgtName << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
    fout << "<PATHROOT>Absolute</PATHROOT>\n";
    fout << "<PATH>" << source << "</PATH>\n";
    fout << "<PATHFORMAT>Generic</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }

    // write out the libraries groups
/*
  <FILEREF>
  <TARGETNAME>Classic Release</TARGETNAME>
  <PATHTYPE>Name</PATHTYPE>
  <PATH>console.stubs.c</PATH>
  <PATHFORMAT>MacOS</PATHFORMAT>
  </FILEREF>
*/

  if (l->GetType() == cmTarget::EXECUTABLE)
    {
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << tgtName << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL RuntimePPC.Lib</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }
  
/*
  <FILEREF>
     <TARGETNAME>Classic Release</TARGETNAME>
     <PATHTYPE>Name</PATHTYPE>
     <PATH>MSL C++.PPC.Lib</PATH>
     <PATHFORMAT>MacOS</PATHFORMAT>
     </FILEREF>
     <FILEREF>
     <TARGETNAME>Classic Release</TARGETNAME>
     <PATHTYPE>Name</PATHTYPE>
     <PATH>MSL C.PPC.Lib</PATH>
     <PATHFORMAT>MacOS</PATHFORMAT>
     </FILEREF>
     <FILEREF>
     <TARGETNAME>Carbon Debug</TARGETNAME>
     <PATHTYPE>Name</PATHTYPE>
     <PATH>MSL C.CARBON.Lib</PATH>
     <PATHFORMAT>MacOS</PATHFORMAT>
     </FILEREF>
*/

  fout << "</GROUP>\n";
}
