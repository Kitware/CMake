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
#include "cmGlobalCodeWarriorGenerator.h"
#include "cmLocalCodeWarriorGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmake.h"

#include <cmsys/RegularExpression.hxx>

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
  // collect up the output names
  // we do this here so any dependencies between targets will work,
  // even if they are forward declared in the previous targets
  cmTargets &tgts = m_Makefile->GetTargets();

  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    std::string targetOutputName = l->first;
    switch (l->second.GetType())
      {
      case cmTarget::STATIC_LIBRARY:
        targetOutputName += ".lib";
        break;
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        targetOutputName += ".dylib";
        break;
      case cmTarget::EXECUTABLE:
        targetOutputName +=  cmSystemTools::GetExecutableExtension();
        break;
      default:;
      }
  
    m_TargetOutputFiles[l->first] = targetOutputName;
    }

  // write out the individual targets
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
  if( strlen( ftype ) > 0 )
    {
    fout << "<SETTING><NAME>FileType</NAME><VALUE>" << ftype << 
      "</VALUE></SETTING>\n";
    }
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
  
  fout << "<SETTING><NAME>UserSearchPaths</NAME>\n";
  
  // project relative path
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  
  // list the include paths
/*  std::vector<std::string>& includes = l->GetIncludeDirectories();
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
    }*/

  // library paths

  // now add in the libraries we depend on
  cmsys::RegularExpression isAframework("[ \t]*\\-framework");
  cmsys::RegularExpression isEnvironment("\\${");
  cmsys::RegularExpression isUNIX("[ \t]*\\-l([^ \t]+)");
  const cmTarget::LinkLibraries& libs = l->GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator lib = libs.begin();
  for(; lib != libs.end(); ++lib)
    {
    // no frameworks!
    if( isAframework.find( lib->first.c_str() ) ) continue;
    
    // no environment variables!
    if( isEnvironment.find( lib->first.c_str() ) ) continue;
    
    std::string libPath = lib->first + "_CMAKE_PATH";
    // is this lib part of this project? Look in the cache
    const char* cacheValue
      = GetGlobalGenerator()->GetCMakeInstance()
      ->GetCacheDefinition(libPath.c_str());

    if( cacheValue && *cacheValue )
      {
      // just tack it on
      fout << "<SETTING>\n";
      fout << "<SETTING><NAME>SearchPath</NAME>\n";
      fout << "<SETTING><NAME>Path</NAME><VALUE>" << cacheValue << "</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
      fout << "</SETTING>\n";
      fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
      fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
      fout << "</SETTING>\n";
      }
    }
  
  const std::vector<std::string>& links = l->GetLinkDirectories();
  std::vector<std::string>::const_iterator j = links.begin();
  for(;j != links.end(); ++j)
    {
    fout << "<SETTING>\n";
    fout << "<SETTING><NAME>SearchPath</NAME>\n";
    fout << "<SETTING><NAME>Path</NAME><VALUE>" << j->c_str() << "</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    }
  fout << "</SETTING>\n";

  // system include and library paths
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
  fout << "<SETTING><NAME>Path</NAME><VALUE>:MacOS X Support:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>CodeWarrior</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:usr:include:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>OS X Volume</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:usr:lib:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>OS X Volume</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:System:Library:Frameworks:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>OS X Volume</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING>\n";
  fout << "<SETTING><NAME>SearchPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:Library:Frameworks:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>OS X Volume</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>true</VALUE></SETTING>\n";
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
  if( l->GetType() == cmTarget::SHARED_LIBRARY ||
      l->GetType() == cmTarget::MODULE_LIBRARY )
    {
    fout << "<SETTING><NAME>Linker</NAME><VALUE>Mach-O PPC Linker</VALUE></SETTING>\n";
    }
  else
    {
    fout << "<SETTING><NAME>Linker</NAME><VALUE>MacOS X PPC Linker</VALUE></SETTING>\n";
    }
  fout << "<SETTING><NAME>PreLinker</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PostLinker</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>Targetname</NAME><VALUE>" << tgtName 
       << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>OutputDirectory</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Unix</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>SaveEntriesUsingRelativePaths</NAME><VALUE>false</VALUE></SETTING>\n";

  // add the cxx file type, and others
  fout << "<SETTING><NAME>FileMappings</NAME>\n";

  if( l->GetType() == cmTarget::SHARED_LIBRARY ||
      l->GetType() == cmTarget::MODULE_LIBRARY )
    {
    this->AddFileMapping(fout,"TEXT",".cxx","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".c","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cc","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cp","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".h","MW C/C++ MachPPC","C/C++",
                         false,false,false,true);
    this->AddFileMapping(fout,"TEXT",".hpp","MW C/C++ MachPPC","C/C++",
                         false,false,false,true);
    this->AddFileMapping(fout,"TEXT",".m","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".mm","MW C/C++ MachPPC","C/C++",
                         false,false,false,false);
    }
  else
    {
    this->AddFileMapping(fout,"TEXT",".cxx","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".c","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cc","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cp","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".cpp","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".h","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,true);
    this->AddFileMapping(fout,"TEXT",".hpp","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,true);
    this->AddFileMapping(fout,"TEXT",".m","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    this->AddFileMapping(fout,"TEXT",".mm","MW C/C++ PPC Mac OS X","C/C++",
                         false,false,false,false);
    }
  this->AddFileMapping(fout,"",".lib","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"",".dylib","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"",".a","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"",".o","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"MDYL","","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"MLIB","","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"MMLB","","MachO Importer","C/C++",
                       false,false,false,false);
  this->AddFileMapping(fout,"MPLF","","MachO Importer","C/C++",
                       false,false,false,false);
  fout << "</SETTING>\n";
  
  // <!-- Settings for "Build Extras" panel -->
  fout << "<SETTING><NAME>CacheModDates</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>DumpBrowserInfo</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>CacheSubprojects</NAME><VALUE>true</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>UseThirdPartyDebugger</NAME><VALUE>false</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>BrowserGenerator</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>DebuggerAppPath</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>DebuggerCmdLineArgs</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>DebuggerWorkingDir</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>CodeCompletionPrefixFileName</NAME>"
       << "<VALUE>MacHeadersMach-O.c</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>CodeCompletionMacroFileName</NAME>"
       << "<VALUE>MacOSX_MSL_C++_Macros.h</VALUE></SETTING>\n";

  fout << "<SETTING><NAME>MWFrontEnd_C_prefixname</NAME>"
       << "<VALUE>MSLCarbonPrefix.h</VALUE></SETTING>";

  // <!-- Settings for "PPC Mac OS X Linker" panel -->
  fout << "<SETTING><NAME>MWLinker_MacOSX_linksym</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_symfullpath</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_nolinkwarnings</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_linkmap</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_dontdeadstripinitcode</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_permitmultdefs</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_use_objectivec_semantics</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_strip_debug_symbols</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_split_segs</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_report_msl_overloads</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_objects_follow_linkorder</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_linkmode</NAME><VALUE>Fast</VALUE></SETTING>\n";
  switch (l->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      fout << "<SETTING><NAME>MWLinker_MacOSX_exports</NAME><VALUE>All</VALUE></SETTING>\n";
      break;
    
    default:
      fout << "<SETTING><NAME>MWLinker_MacOSX_exports</NAME><VALUE>ReferencedGlobals</VALUE></SETTING>\n";
    }
  fout << "<SETTING><NAME>MWLinker_MacOSX_sortcode</NAME><VALUE>None</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MacOSX_mainname</NAME><VALUE>start</VALUE></SETTING>\n";

  // <!-- Settings for "PPC Mac OS X Project" panel -->
  fout << "<SETTING><NAME>MWProject_MacOSX_type</NAME><VALUE>";

  std::string targetOutputType;
  switch (l->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      targetOutputType = "MMLB";
      fout << "Library";
      break;
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      targetOutputType = "MDYL";
      fout << "SharedLibrary";
      break;
    case cmTarget::EXECUTABLE:
      targetOutputType = "APPL";
      fout << "ApplicationPackage";
      break;
    default:;
    }
  fout << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_outfile</NAME><VALUE>";
  fout << m_TargetOutputFiles[std::string(tgtName)];
  fout << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_filetype</NAME><VALUE>";
  fout << targetOutputType << "</VALUE></SETTING>\n";

  fout << "<SETTING><NAME>MWProject_MacOSX_vmaddress</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_usedefaultvmaddr</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_flatrsrc</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_flatrsrcfilename</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_flatrsrcoutputdir</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_installpath</NAME><VALUE>./</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_dont_prebind</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_flat_namespace</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_frameworkversion</NAME><VALUE>A</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_currentversion</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MacOSX_flat_oldimpversion</NAME><VALUE>0</VALUE></SETTING>\n";

  // <!-- Settings for "PPC Mach-O Linker" panel -->
  fout << "<SETTING><NAME>MWLinker_MachO_exports</NAME><VALUE>All</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_mainname</NAME><VALUE>start</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_currentversion</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_compatibleversion</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_symfullpath</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_supresswarnings</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_multisymerror</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_prebind</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_deadstrip</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_objectivecsemantics</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_whichfileloaded</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_whyfileloaded</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_readonlyrelocs</NAME><VALUE>Errors</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_undefinedsymbols</NAME><VALUE>Errors</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_twolevelnamespace</NAME><VALUE>1</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWLinker_MachO_stripdebugsymbols</NAME><VALUE>0</VALUE></SETTING>\n";

  // <!-- Settings for "PPC Mach-O Target" panel -->
  fout << "<SETTING><NAME>MWProject_MachO_type</NAME><VALUE>";
  switch (l->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      targetOutputType = "MMLB";
      fout << "Library";
      break;
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      targetOutputType = "MDYL";
      fout << "SharedLibrary";
      break;
    case cmTarget::EXECUTABLE:
      targetOutputType = "APPL";
      fout << "ApplicationPackage";
      break;
    default:;
    }
  fout << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_outfile</NAME><VALUE>";
  fout << m_TargetOutputFiles[std::string(tgtName)];
  fout << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_filecreator</NAME><VALUE>????" << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_filetype</NAME><VALUE>";
  fout << targetOutputType;
  fout << "</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_vmaddress</NAME><VALUE>4096</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_flatrsrc</NAME><VALUE>0</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_flatrsrcfilename</NAME><VALUE></VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_flatrsrcoutputdir</NAME>\n";
  fout << "<SETTING><NAME>Path</NAME><VALUE>:</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n";
  fout << "</SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_installpath</NAME><VALUE>./</VALUE></SETTING>\n";
  fout << "<SETTING><NAME>MWProject_MachO_frameworkversion</NAME><VALUE></VALUE></SETTING>\n";
  
  fout << "</SETTINGLIST>\n";
}

void cmLocalCodeWarriorGenerator::WriteFileList(std::ostream& fout,
                                                const char* /*tgtName*/,
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
  cmsys::RegularExpression isAframework("[ \t]*\\-framework");
  cmsys::RegularExpression isEnvironment("\\${");
  cmsys::RegularExpression isUNIX("[ \t]*\\-l([^ \t]+)");
  const cmTarget::LinkLibraries& libs = l->GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator lib = libs.begin();
  for(; lib != libs.end(); ++lib)
    {
    // no frameworks!
    if( isAframework.find( lib->first.c_str() ) ) continue;
    
    // no environment variables!
    if( isEnvironment.find( lib->first.c_str() ) ) continue;
    
    std::string libPath = lib->first + "_CMAKE_PATH";
    // is this lib part of this project? Look in the cache
    const char* cacheValue
      = GetGlobalGenerator()->GetCMakeInstance()
      ->GetCacheDefinition(libPath.c_str());

    if( cacheValue && *cacheValue )
      {
      // just tack it on
      fout << "<FILE>\n";
      fout << "<PATHTYPE>RootRelative</PATHTYPE>\n";
      fout << "<PATHROOT>Project</PATHROOT>\n";
      fout << "<PATH>" << m_TargetOutputFiles[lib->first] << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "<FILEKIND>Library</FILEKIND>\n";
      fout << "<FILEFLAGS>Debug, TargetOutputFile</FILEFLAGS>\n";
      fout << "</FILE>\n";
      }
    else if( lib->first.find('/') != std::string::npos )
      {
      // it's a path-based library, so we'll include it directly by path
      fout << "<FILE>\n";
      fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
      fout << "<PATHROOT>Absolute</PATHROOT>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "<FILEKIND>Text</FILEKIND>\n";
      fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
      fout << "</FILE>\n";
      }
    else if( isUNIX.find( lib->first.c_str() ) )
      {
      // now we need to search the library directories for this
      // library name, and if we don't find it, we have to search
      // in the cache to see if there's a target defining that lib.
      // for the first search, we have to check for 
      //    [lib]<name>[.<a|lib|so|dylib|dll>]
      std::string libName = isUNIX.match(1);
      }
    else
      {
      // just tack it on
      fout << "<FILE>\n";
      fout << "<PATHTYPE>Name</PATHTYPE>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "<FILEKIND>Library</FILEKIND>\n";
      fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
      fout << "</FILE>\n";
      }
    }
  
  // now add in the system libs (for an executable)
  if (l->GetType() == cmTarget::EXECUTABLE)
    {
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>crt1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "<FILEKIND>Library</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL_All_Mach-O.lib</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "<FILEKIND>Library</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>console_OS_X.c</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "<FILEKIND>Text</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    }
  // or a dynamic library
/*  else if (l->GetType() == cmTarget::SHARED_LIBRARY ||
    l->GetType() == cmTarget::MODULE_LIBRARY)
    {
    fout << "<FILE>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>dylib1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "<FILEKIND>Library</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    }*/

  fout << "</FILELIST>\n";
}


void cmLocalCodeWarriorGenerator::WriteLinkOrder(std::ostream& fout,
                                                 const char* tgtName,
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

  // now add in the libraries we depend on
  cmsys::RegularExpression isAframework("[ \t]*\\-framework");
  cmsys::RegularExpression isEnvironment("\\${");
  cmsys::RegularExpression isUNIX("[ \t]*\\-l([^ \t]+)");
  const cmTarget::LinkLibraries& libs = l->GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator lib = libs.begin();
  
  std::map<std::string, std::string> referencedTargets;
  
  for(; lib != libs.end(); ++lib)
    {
    // no frameworks!
    if( isAframework.find( lib->first.c_str() ) ) continue;
    
    // no environment variables!
    if( isEnvironment.find( lib->first.c_str() ) ) continue;
    
    std::string libPath = lib->first + "_CMAKE_PATH";
    // is this lib part of this project? Look in the cache
    const char* cacheValue
      = GetGlobalGenerator()->GetCMakeInstance()
      ->GetCacheDefinition(libPath.c_str());

    if( cacheValue && *cacheValue ) 
      {
      // just tack it on
      fout << "<FILEREF>\n";
      fout << "<PATHTYPE>RootRelative</PATHTYPE>\n";
      fout << "<PATHROOT>Project</PATHROOT>\n";
      fout << "<PATH>" << m_TargetOutputFiles[lib->first] << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      referencedTargets[lib->first] = m_TargetOutputFiles[lib->first];
      if( m_TargetReferencingList.count(m_TargetOutputFiles[lib->first]) == 0 )
        {
        m_TargetReferencingList[m_TargetOutputFiles[lib->first]] = std::string(tgtName);
        }
      }
    else if( lib->first.find('/') != std::string::npos )
      {
      // it's a path-based library, so we'll include it directly by path
      fout << "<FILEREF>\n";
      fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
      fout << "<PATHROOT>Absolute</PATHROOT>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      }
    else if( isUNIX.find( lib->first.c_str() ) )
      {
      // now we need to search the library directories for this
      // library name, and if we don't find it, we have to search
      // in the cache to see if there's a target defining that lib.
      // for the first search, we have to check for 
      //    [lib]<name>[.<a|lib|so|dylib|dll>]
      std::string libName = isUNIX.match(1);
      }
    else
      {
      // just tack it on
      fout << "<FILEREF>\n";
      fout << "<PATHTYPE>Name</PATHTYPE>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      }
    }

  // now add in the system libs (for an executable)
  if (l->GetType() == cmTarget::EXECUTABLE)
    {
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>crt1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL_All_Mach-O.lib</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>console_OS_X.c</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }
  // or a shared library
/*  else if (l->GetType() == cmTarget::SHARED_LIBRARY ||
    l->GetType() == cmTarget::MODULE_LIBRARY)
    {
    fout << "<FILEREF>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>dylib1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    }*/
  
  fout << "</LINKORDER>\n";

  if( referencedTargets.size() )
    {
    fout << "<SUBTARGETLIST>\n";
    // output subtarget list
    std::map<std::string, std::string>::const_iterator target = referencedTargets.begin();
    for( ; target != referencedTargets.end(); target++ )
      {
      fout << "<SUBTARGET>\n";
      fout << "<TARGETNAME>" << target->first << "</TARGETNAME>\n";
      fout << "<ATTRIBUTES>LinkAgainst</ATTRIBUTES>\n";
      fout << "<FILEREF>\n<PATHTYPE>RootRelative</PATHTYPE>\n"
           << "<PATHROOT>Project</PATHROOT>\n"
           << "<PATH>" << target->second << "</PATH>\n"
           << "<PATHFORMAT>Unix</PATHFORMAT></FILEREF>\n";
      fout << "</SUBTARGET>\n";
      }
    fout << "</SUBTARGETLIST>\n";
    }

  // we need at least one framework for the XML to be valid
  // generate framework list
  cmsys::RegularExpression reg("[ \t]*\\-framework[ \t]+([^ \t]+)");
  std::vector<std::string> frameworks;
  
  lib = libs.begin();
  for(; lib != libs.end(); ++lib)
    {
    if( reg.find( lib->first.c_str() ) )
      {
      frameworks.push_back( reg.match(1) );
      }
    }

  if( frameworks.size() > 0 || l->GetType() == cmTarget::EXECUTABLE )
    {
    fout << "<FRAMEWORKLIST>\n";
          
    std::vector<std::string>::const_iterator framework = frameworks.begin();
    for(; framework != frameworks.end(); ++framework)
      {
      fout << "<FRAMEWORK>\n";
      fout << "<FILEREF>\n";
      fout << "<PATHTYPE>Name</PATHTYPE>\n";
      fout << "<PATH>" << framework->c_str() << ".framework</PATH>\n";
      fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      // this isn't strictly always true, I believe, but Apple adheres to it
      fout << "<DYNAMICLIBRARY>" << framework->c_str() << "</DYNAMICLIBRARY>\n";
      fout << "</FRAMEWORK>\n";
      }
          
    // if it's an executable, it needs to link into System.framework
    if (l->GetType() == cmTarget::EXECUTABLE)
      {
      fout << "<FRAMEWORK>\n";
      fout << "<FILEREF>\n";
      fout << "<PATHTYPE>Name</PATHTYPE>\n";
      fout << "<PATH>System.framework</PATH>\n";
      fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      fout << "<DYNAMICLIBRARY>System</DYNAMICLIBRARY>\n";
      fout << "</FRAMEWORK>\n";
      }
      
    fout << "</FRAMEWORKLIST>\n";
    }
}

void cmLocalCodeWarriorGenerator::WriteGroups(std::ostream& fout)
{
  bool hasExecutableTarget = false, hasDynamicLibTarget = false;
  char *firstExecutableTarget = 0, *firstDynamicLibTarget = 0;
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    this->WriteGroup(fout,l->first.c_str(),&(l->second));
    if (l->second.GetType() == cmTarget::EXECUTABLE)
      {
      hasExecutableTarget = true;
      if (firstExecutableTarget == 0)
        {
        firstExecutableTarget = const_cast<char*>(l->first.c_str());
        }
      }
    else if (l->second.GetType() == cmTarget::SHARED_LIBRARY ||
             l->second.GetType() == cmTarget::MODULE_LIBRARY)
      {
      hasDynamicLibTarget = true;
      if (firstDynamicLibTarget == 0)
        {
        firstDynamicLibTarget = const_cast<char*>(l->first.c_str());
        }
      }
    }

  // write out the libraries groups
  if( hasExecutableTarget )
    {
    fout << "<GROUP><NAME>" << "App Support" << "</NAME>\n";
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << firstExecutableTarget << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>crt1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << firstExecutableTarget << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>MSL_All_Mach-O.lib</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << firstExecutableTarget << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>console_OS_X.c</PATH>\n";
    fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "</GROUP>\n";
    }
/*  if (hasDynamicLibTarget)
    {
    fout << "<GROUP><NAME>" << "dylib Support" << "</NAME>\n";
    fout << "<FILEREF>\n";
    fout << "<TARGETNAME>" << firstDynamicLibTarget << "</TARGETNAME>\n";
    fout << "<PATHTYPE>Name</PATHTYPE>\n";
    fout << "<PATH>dylib1.o</PATH>\n";
    fout << "<PATHFORMAT>MacOS</PATHFORMAT>\n";
    fout << "</FILEREF>\n";
    fout << "</GROUP>\n";
    }*/
  
  // write out the referenced targets group
  if( m_TargetReferencingList.size() > 0 )
    {
    fout << "<GROUP><NAME>Subtarget Files</NAME>\n";
    
    std::map<std::string, std::string>::const_iterator subtarget = m_TargetReferencingList.begin();
    for( ; subtarget != m_TargetReferencingList.end(); subtarget++ )
      {
      fout << "<FILEREF>\n"
           << "<TARGETNAME>" << subtarget->second << "</TARGETNAME>\n";
      fout << "<PATHTYPE>RootRelative</PATHTYPE>\n<PATHROOT>Project</PATHROOT>\n";
      fout << "<PATH>" << subtarget->first << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>";
      }
    
    fout << "</GROUP>";
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

  // now add in the libraries we depend on
  cmsys::RegularExpression isAframework("[ \t]*\\-framework");
  cmsys::RegularExpression isEnvironment("\\${");
  cmsys::RegularExpression isUNIX("[ \t]*\\-l([^ \t]+)");
  const cmTarget::LinkLibraries& libs = l->GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator lib = libs.begin();
  for(; lib != libs.end(); ++lib)
    {
    // no frameworks!
    if( isAframework.find( lib->first.c_str() ) ) continue;
    
    // no environment variables!
    if( isEnvironment.find( lib->first.c_str() ) ) continue;
    
    std::string libPath = lib->first + "_CMAKE_PATH";
    // is this lib part of this project? Look in the cache
    const char* cacheValue
      = GetGlobalGenerator()->GetCMakeInstance()
      ->GetCacheDefinition(libPath.c_str());

    if( cacheValue && *cacheValue )
      {
      // this is a subtarget reference, it will be taken care of later
      continue;
      }
    else if( lib->first.find('/') != std::string::npos )
      {
      // it's a path-based library, so we'll include it directly by path
      fout << "<FILEREF>\n";
      fout << "<TARGETNAME>" << tgtName << "</TARGETNAME>\n";
      fout << "<PATHTYPE>Absolute</PATHTYPE>\n";
      fout << "<PATHROOT>Absolute</PATHROOT>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      }
    else if( isUNIX.find( lib->first.c_str() ) )
      {
      // now we need to search the library directories for this
      // library name, and if we don't find it, we have to search
      // in the cache to see if there's a target defining that lib.
      // for the first search, we have to check for 
      //    [lib]<name>[.<a|lib|so|dylib|dll>]
      std::string libName = isUNIX.match(1);
      }
    else
      {
      // just tack it on
      fout << "<FILEREF>\n";
      fout << "<TARGETNAME>" << tgtName << "</TARGETNAME>\n";
      fout << "<PATHTYPE>Name</PATHTYPE>\n";
      fout << "<PATH>" << lib->first.c_str() << "</PATH>\n";
      fout << "<PATHFORMAT>Unix</PATHFORMAT>\n";
      fout << "</FILEREF>\n";
      }
    }

  fout << "</GROUP>\n";
}
