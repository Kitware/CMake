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
#include "cmGlobalGenerator.h"
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/RegularExpression.hxx>
#include <stdio.h>

cmLocalUnixMakefileGenerator::cmLocalUnixMakefileGenerator()
{
  m_WindowsShell = false;
  m_IncludeDirective = "include";
  m_MakefileVariableSize = 0;
  m_IgnoreLibPrefix = false;
}

cmLocalUnixMakefileGenerator::~cmLocalUnixMakefileGenerator()
{
}


void cmLocalUnixMakefileGenerator::Generate(bool fromTheTop)
{
  // suppoirt override in output directories
  if (m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    m_LibraryOutputPath = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    if(m_LibraryOutputPath.size())
      {
      if(m_LibraryOutputPath[m_LibraryOutputPath.size() -1] != '/')
        {
        m_LibraryOutputPath += "/";
        }
      if(!cmSystemTools::MakeDirectory(m_LibraryOutputPath.c_str()))
        {
        cmSystemTools::Error("Error failed create "
                             "LIBRARY_OUTPUT_PATH directory:",
                             m_LibraryOutputPath.c_str());
        }
      m_Makefile->AddLinkDirectory(m_LibraryOutputPath.c_str());
      }
    }
  if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    m_ExecutableOutputPath =
      m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    if(m_ExecutableOutputPath.size())
      {
      if(m_ExecutableOutputPath[m_ExecutableOutputPath.size() -1] != '/')
        {
        m_ExecutableOutputPath += "/";
        }
      if(!cmSystemTools::MakeDirectory(m_ExecutableOutputPath.c_str()))
        {
        cmSystemTools::Error("Error failed to create " 
                             "EXECUTABLE_OUTPUT_PATH directory:",
                             m_ExecutableOutputPath.c_str());
        }
      m_Makefile->AddLinkDirectory(m_ExecutableOutputPath.c_str());
      }
    }

  if (!fromTheTop)
    {
    // Generate depends 
    cmMakeDepend md;
    md.SetMakefile(m_Makefile);
    md.GenerateMakefileDependencies();
    this->ProcessDepends(md);
    }
  // output the makefile fragment
  std::string dest = m_Makefile->GetStartOutputDirectory();
  dest += "/Makefile";
  this->OutputMakefile(dest.c_str(), !fromTheTop); 
}

void 
cmLocalUnixMakefileGenerator::AddDependenciesToSourceFile(cmDependInformation const *info,
                                                          cmSourceFile *i,
                                                          std::set<cmDependInformation const*> *visited)
{
  // add info to the visited set
  visited->insert(info);
    
  // add this dependency and the recurse
  // now recurse with info's dependencies
  for(cmDependInformation::DependencySet::const_iterator d = 
        info->m_DependencySet.begin();
      d != info->m_DependencySet.end(); ++d)
    {
    if (visited->find(*d) == visited->end())
      { 
      if((*d)->m_FullPath != "")
        {
        i->GetDepends().push_back((*d)->m_FullPath);
        }
      this->AddDependenciesToSourceFile(*d,i,visited);
      }
    }
}

void cmLocalUnixMakefileGenerator::ProcessDepends(const cmMakeDepend &md)
{
  // Now create cmDependInformation objects for files in the directory
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        // get the depends
        const cmDependInformation *info = 
          md.GetDependInformationForSourceFile(*(*i));
        
        // Delete any hints from the source file's dependencies.
        (*i)->GetDepends().erase((*i)->GetDepends().begin(), (*i)->GetDepends().end());

        // Now add the real dependencies for the file.
        if (info)
          {
          // create visited set
          std::set<cmDependInformation const*> visited;
          this->AddDependenciesToSourceFile(info,*i, &visited);
          }
        }
      }
    }
}


// This is where CMakeTargets.make is generated
void cmLocalUnixMakefileGenerator::OutputMakefile(const char* file, 
                                                  bool withDepends)
{
  // Create sub directories fro aux source directories
  std::vector<std::string>& auxSourceDirs = 
    m_Makefile->GetAuxSourceDirectories();
  if( auxSourceDirs.size() )
    {
    // For the case when this is running as a remote build
    // on unix, make the directory
    for(std::vector<std::string>::iterator i = auxSourceDirs.begin();
        i != auxSourceDirs.end(); ++i)
      {
      if(i->c_str()[0] != '/')
        {
        std::string dir = m_Makefile->GetCurrentOutputDirectory();
        if(dir.size() && dir[dir.size()-1] != '/')
          {
          dir += "/";
          }
        dir += *i;
        cmSystemTools::MakeDirectory(dir.c_str());
        }
      else
        {
        cmSystemTools::MakeDirectory(i->c_str());
        }
      }
    }
  // Create a stream that writes to a temporary file
  // then does a copy at the end.   This is to allow users
  // to hit control-c during the make of the makefile
  cmGeneratedFileStream tempFile(file);
  tempFile.SetAlwaysCopy(true);
  std::ostream&  fout = tempFile.GetStream();
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", file);
    return;
    }
  fout << "# CMAKE generated Makefile, DO NOT EDIT!\n"
       << "# Generated by \"" << m_GlobalGenerator->GetName() << "\""
       << " Generator, CMake Version " 
       << cmMakefile::GetMajorVersion() << "." 
       << cmMakefile::GetMinorVersion() << "\n"
       << "# Generated from the following files:\n# "
       << m_Makefile->GetHomeOutputDirectory() << "/CMakeCache.txt\n";
  std::vector<std::string> lfiles = m_Makefile->GetListFiles();
  // sort the array
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
  // remove duplicates
  std::vector<std::string>::iterator new_end = 
    std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());

  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    fout << "# " << i->c_str() << "\n";
    }
  fout << "\n\n";
  if(!m_Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    fout << "# Suppresses display of executed commands\n";
    fout << ".SILENT:\n";
    }
  fout << "# disable some common implicit rules to speed things up\n";
  fout << ".SUFFIXES:\n";
  fout << ".SUFFIXES:.hpuxmakemusthaverule\n";
  // create a make variable with all of the sources for this Makefile
  // for depend purposes.
  fout << "CMAKE_MAKEFILE_SOURCES = ";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    fout << " " << cmSystemTools::ConvertToOutputPath(i->c_str());
    }
  // Add the cache to the list
  std::string cacheFile = m_Makefile->GetHomeOutputDirectory();
  cacheFile += "/CMakeCache.txt";
  fout << " " << cmSystemTools::ConvertToOutputPath(cacheFile.c_str());
  fout << "\n\n\n";
  this->OutputMakeVariables(fout);  
  std::string checkCache = m_Makefile->GetHomeOutputDirectory();
  checkCache += "/cmake.check_cache";
  checkCache = cmSystemTools::ConvertToOutputPath(checkCache.c_str());
  // Set up the default target as the VERY first target, so that make with no arguments will run it
  this->
    OutputMakeRule(fout, 
                   "Default target executed when no arguments are given to make, first make sure cmake.depends exists, cmake.check_depends is up-to-date, check the sources, then build the all target",
                   "default_target",
                   checkCache.c_str(),
                   "$(MAKE) $(MAKESILENT) cmake.depends",
                   "$(MAKE) $(MAKESILENT) cmake.check_depends",
                   "$(MAKE) $(MAKESILENT) -f cmake.check_depends",
                   "$(MAKE) $(MAKESILENT) all");
  
  this->OutputTargetRules(fout);
  this->OutputDependLibs(fout);
  this->OutputTargets(fout);
  this->OutputSubDirectoryRules(fout);
  std::string dependName = m_Makefile->GetStartOutputDirectory();
  dependName += "/cmake.depends";
  if(withDepends)
    {
    std::ofstream dependout(dependName.c_str());
    if(!dependout)
      {
       cmSystemTools::Error("Error can not open for write: ", dependName.c_str());
       return;
      }
    dependout << "# .o dependencies in this directory." << std::endl;

    std::string checkDepend = m_Makefile->GetStartOutputDirectory();
    checkDepend += "/cmake.check_depends";
    std::ofstream checkdependout(checkDepend.c_str());
    if(!checkdependout)
      {
       cmSystemTools::Error("Error can not open for write: ", checkDepend.c_str());
       return;
      }
    checkdependout << "# This file is used as a tag file, that all sources depend on.  If a source changes, then the rule to rebuild this file will cause cmake.depends to be rebuilt." << std::endl;
    // if there were any depends output, then output the check depends
    // information inot checkdependout
    if(this->OutputObjectDepends(dependout))
      {
      this->OutputCheckDepends(checkdependout);
      }
    else
      {
      checkdependout << "all:\n\t@echo cmake.depends is up-to-date\n";
      }
    }
  this->OutputCustomRules(fout);
  this->OutputMakeRules(fout);
  this->OutputInstallRules(fout);
  // only add the depend include if the depend file exists
  if(cmSystemTools::FileExists(dependName.c_str()))
    {
    fout << m_IncludeDirective << " cmake.depends\n";
    }
}



#if defined(_WIN32) && ! defined(__CYGWIN__) 
std::string cmLocalUnixMakefileGenerator::GetOutputExtension(const char* s)
{
  std::string sourceExtension = s;
  if(sourceExtension == "def")
    {
    return "";
    }
  if(sourceExtension == "ico" || sourceExtension == "rc2")
    {
    return "";
    }
  if(sourceExtension == "rc")
    {
    return ".res";
    }
  return ".obj";
}
#else
std::string cmLocalUnixMakefileGenerator::GetOutputExtension(const char*)
{
  return ".o";
}
#endif

std::string cmLocalUnixMakefileGenerator::GetFullTargetName(const char* n,
                                                            const cmTarget& t)
{
  const char* targetPrefix = t.GetProperty("PREFIX");
  const char* targetSuffix = t.GetProperty("SUFFIX");
  const char* prefixVar = 0;
  const char* suffixVar = 0;
  switch(t.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      prefixVar = "CMAKE_STATIC_LIBRARY_PREFIX";
      suffixVar = "CMAKE_STATIC_LIBRARY_SUFFIX";
      break;
    case cmTarget::SHARED_LIBRARY:
      prefixVar = "CMAKE_SHARED_LIBRARY_PREFIX";
      suffixVar = "CMAKE_SHARED_LIBRARY_SUFFIX";
      break;
    case cmTarget::MODULE_LIBRARY:
      prefixVar = "CMAKE_SHARED_MODULE_PREFIX";
      suffixVar = "CMAKE_SHARED_MODULE_SUFFIX";
      break;
    case cmTarget::EXECUTABLE:
    case cmTarget::WIN32_EXECUTABLE:
      targetSuffix = cmSystemTools::GetExecutableExtension();
    case cmTarget::UTILITY:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->GetSafeDefinition(suffixVar);
    }
  std::string name = targetPrefix?targetPrefix:"";
  name += n;
  name += targetSuffix?targetSuffix:"";
  return name;
}

// Output the rules for any targets
void cmLocalUnixMakefileGenerator::OutputEcho(std::ostream& fout, 
                                              const char *msg)
{
  std::string echostring = msg;
  // for unix we want to quote the output of echo
  // for nmake and borland, the echo should not be quoted
  if(strcmp(m_GlobalGenerator->GetName(), "Unix Makefiles") == 0)
    {
    cmSystemTools::ReplaceString(echostring, "\\\n", " ");
    cmSystemTools::ReplaceString(echostring, " \t", "   ");
    cmSystemTools::ReplaceString(echostring, "\n\t", "\"\n\t@echo \"");
    fout << "\t@echo \"" << echostring.c_str() << "\"\n";
    }
  else
    {
    cmSystemTools::ReplaceString(echostring, "\n\t", "\n\t@echo ");
    fout << "\t@echo " << echostring.c_str() << "\n";
    }
}

// Output the rules for any targets
void cmLocalUnixMakefileGenerator::OutputTargetRules(std::ostream& fout)
{
  const cmTargets &tgts = m_Makefile->GetTargets();
  
  // add the help target
  fout << "help:\n";
  this->OutputEcho(fout,"The following are some of the valid targets for this Makefile:");
  this->OutputEcho(fout,"... all (the default if no target is provided)");
  this->OutputEcho(fout,"... clean");
  this->OutputEcho(fout,"... depend");
  this->OutputEcho(fout,"... rebuild_cache");

  // libraries
  std::string path;
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if((l->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (l->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (l->second.GetType() == cmTarget::MODULE_LIBRARY))
      {
      std::string path2 = m_LibraryOutputPath;
      path2 += this->GetFullTargetName(l->first.c_str(), l->second);
      path = "... ";
      path += cmSystemTools::ConvertToOutputPath(path2.c_str());
      this->OutputEcho(fout,path.c_str());
      }
    }
  // executables
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if ((l->second.GetType() == cmTarget::EXECUTABLE ||
         l->second.GetType() == cmTarget::WIN32_EXECUTABLE))
      {
      path = "... ";
      path += l->first + cmSystemTools::GetExecutableExtension();
      this->OutputEcho(fout,path.c_str());
      }
    }
  // list utilities last
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::UTILITY)
      {
      path = "... ";
      path += l->first;
      this->OutputEcho(fout,path.c_str());
      }
    }
  fout << "\n\n";
    
  
  // for each target add to the list of targets
  fout << "TARGETS = ";
  // list libraries first
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.IsInAll())
      {
      if((l->second.GetType() == cmTarget::STATIC_LIBRARY) ||
         (l->second.GetType() == cmTarget::SHARED_LIBRARY) ||
         (l->second.GetType() == cmTarget::MODULE_LIBRARY))
        {
        std::string path = m_LibraryOutputPath;
        path += this->GetFullTargetName(l->first.c_str(), l->second);
        fout << " \\\n" 
             << cmSystemTools::ConvertToOutputPath(path.c_str());
        }
      }
    }
  // executables
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if ((l->second.GetType() == cmTarget::EXECUTABLE ||
         l->second.GetType() == cmTarget::WIN32_EXECUTABLE) &&
        l->second.IsInAll())
      {
      std::string path = m_ExecutableOutputPath;
      path += this->GetFullTargetName(l->first.c_str(), l->second);
      fout << " \\\n" << cmSystemTools::ConvertToOutputPath(path.c_str());
      }
    }
  // list utilities last
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::UTILITY &&
        l->second.IsInAll())
      {
      fout << " \\\n" << l->first.c_str();
      }
    }
  fout << "\n\n";
  // get the classes from the source lists then add them to the groups
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    std::vector<cmSourceFile*> classes = l->second.GetSourceFiles();
    if (classes.begin() != classes.end())
      {
      fout << this->CreateMakeVariable(l->first.c_str(), "_SRC_OBJS") << " = ";
      for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
          i != classes.end(); i++)
        {
        if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY") && 
           !(*i)->GetCustomCommand())
          {
          std::string outExt(
            this->GetOutputExtension((*i)->GetSourceExtension().c_str()));
          if(outExt.size())
            {
            fout << "\\\n" 
                 << cmSystemTools::ConvertToOutputPath((*i)->GetSourceName().c_str())
                 << outExt.c_str() << " ";
            }
          }
        }
      fout << "\n\n";
      fout << this->CreateMakeVariable(l->first.c_str(), "_SRC_OBJS_QUOTED") << " = ";
      for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
          i != classes.end(); i++)
        {
        if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY") &&
           !(*i)->GetCustomCommand())
          {
          std::string outExt(this->GetOutputExtension((*i)->GetSourceExtension().c_str()));
          if(outExt.size())
            {
            fout << "\\\n\"" << cmSystemTools::ConvertToOutputPath((*i)->GetSourceName().c_str())
                 << outExt.c_str() << "\" ";
            }
          }
        }
      fout << "\n\n";
      }
    }
  fout << "CLEAN_OBJECT_FILES = ";
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    std::vector<cmSourceFile*> classes = l->second.GetSourceFiles();
    if (classes.begin() != classes.end())
      {
      fout << "$(" << this->CreateMakeVariable(l->first.c_str(), "_SRC_OBJS")
           << ") ";
      }
    }
  fout << "\n\n";
  const char * qt_files = m_Makefile->GetDefinition("GENERATED_QT_FILES");
  if (qt_files != NULL && 
      strlen(m_Makefile->GetDefinition("GENERATED_QT_FILES"))>0)
    {
    fout << "GENERATED_QT_FILES = ";
    fout << qt_files;
    fout << "\n\n";
    }
}


/**
 * Output the linking rules on a command line.  For executables,
 * targetLibrary should be a NULL pointer.  For libraries, it should point
 * to the name of the library.  This will not link a library against itself.
 */
void cmLocalUnixMakefileGenerator::OutputLinkLibraries(std::ostream& fout,
                                                  const char* targetLibrary,
                                                  const cmTarget &tgt)
{
  // Try to emit each search path once
  std::set<std::string> emitted;

  // Embed runtime search paths if possible and if required.
  bool outputRuntime = true;
  std::string runtimeFlag;
  std::string runtimeSep;
  std::vector<std::string> runtimeDirs;

  std::string buildType =  this->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);

  bool cxx = tgt.HasCxx(); 
  if(!cxx )
    {
    // if linking a c executable use the C runtime flag as cc
    // may not be the same program that creates shared libaries
    // and may have different flags
    runtimeFlag = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_RUNTIME_FLAG");
    runtimeSep = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP");
    }
  else
    { 
    runtimeFlag = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG");
    runtimeSep = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP");
    }
  
  // concatenate all paths or no?
  bool runtimeConcatenate = ( runtimeSep!="" );
  if(runtimeFlag == "" || m_Makefile->IsOn("CMAKE_SKIP_RPATH") )
    {
    outputRuntime = false;
    }

  // Some search paths should never be emitted
  emitted.insert("");
  emitted.insert("/usr/lib");
  std::string libPathFlag = m_Makefile->GetDefinition("CMAKE_LIBRARY_PATH_FLAG");
  std::string libLinkFlag = this->GetSafeDefinition("CMAKE_LINK_LIBRARY_FLAG");
  // collect all the flags needed for linking libraries
  std::string linkLibs;
  
  // Flags to link an executable to shared libraries.
  if( tgt.GetType() == cmTarget::EXECUTABLE ||
      tgt.GetType() == cmTarget::WIN32_EXECUTABLE )
    {
    if(cxx)
      {
      linkLibs = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS");
      }
    else
      {
      linkLibs = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_LINK_FLAGS");
      }
    linkLibs += " ";
    }
  
  const std::vector<std::string>& libdirs = tgt.GetLinkDirectories();
  for(std::vector<std::string>::const_iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    { 
    std::string libpath = this->ConvertToOutputForExisting(libDir->c_str());
    if(emitted.insert(libpath).second)
      {
      std::string::size_type pos = libDir->find(libPathFlag.c_str());
      if((pos == std::string::npos || pos > 0)
         && libDir->find("${") == std::string::npos)
        {
        linkLibs += libPathFlag;
        if(outputRuntime)
          {
          runtimeDirs.push_back( libpath );
          }
        }
      linkLibs += libpath;
      linkLibs += " ";
      }
    }

  std::string linkSuffix = this->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");
  std::string regexp = ".*\\";
  regexp += linkSuffix;
  regexp += "$";
  cmsys::RegularExpression hasSuffix(regexp.c_str());
  std::string librariesLinked;
  const cmTarget::LinkLibraries& libs = tgt.GetLinkLibraries();
  for(cmTarget::LinkLibraries::const_iterator lib = libs.begin();
      lib != libs.end(); ++lib)
    {
    // Don't link the library against itself!
    if(targetLibrary && (lib->first == targetLibrary)) continue;
    // use the correct lib for the current configuration
    if (lib->second == cmTarget::DEBUG && buildType != "DEBUG")
      {
      continue;
      }
    if (lib->second == cmTarget::OPTIMIZED && buildType == "DEBUG")
      {
      continue;
      }
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() == 0) continue;
    // if it is a full path break it into -L and -l
    cmsys::RegularExpression reg("([ \t]*\\-l)|([ \t]*\\-framework)|(\\${)|([ \t]*\\-pthread)");
    if(lib->first.find('/') != std::string::npos
       && !reg.find(lib->first))
      {
      std::string dir, file;
      cmSystemTools::SplitProgramPath(lib->first.c_str(),
                                      dir, file);
      std::string libpath = this->ConvertToOutputForExisting(dir.c_str());
      if(emitted.insert(libpath).second)
        {
        linkLibs += libPathFlag;
        linkLibs += libpath;
        linkLibs += " ";
        if(outputRuntime)
          {
          runtimeDirs.push_back( libpath );
          }
        }  
      cmsys::RegularExpression libname("^lib([^/]*)(\\.so|\\.lib|\\.dll|\\.sl|\\.a|\\.dylib).*");
      cmsys::RegularExpression libname_noprefix("([^/]*)(\\.so|\\.lib|\\.dll|\\.sl|\\.a|\\.dylib).*");
      if(libname.find(file))
        {
        // Library had "lib" prefix.
        librariesLinked += libLinkFlag;
        file = libname.match(1);
        // if ignore libprefix is on,
        // then add the lib prefix back into the name
        if(m_IgnoreLibPrefix)
          {
          std::cout << "m_IgnoreLibPrefix\n";
          file = "lib" + file;
          }
        librariesLinked += file;
        if(linkSuffix.size() && !hasSuffix.find(file))
          {
          librariesLinked += linkSuffix;
          }
        librariesLinked += " ";
        }
      else if(libname_noprefix.find(file))
        {
        // Library had no "lib" prefix.
        librariesLinked += libLinkFlag;
        file = libname_noprefix.match(1);
        librariesLinked += file;
        if(linkSuffix.size() && !hasSuffix.find(file))
          {
          librariesLinked +=  linkSuffix;
          }
        librariesLinked += " ";
        }
      else
        {
        // Error parsing the library name.  Just use the full path.
        // The linker will give an error if it is invalid.
        librariesLinked += lib->first;
        librariesLinked += " ";
        }
      }
    // not a full path, so add -l name
    else
      {
      if(!reg.find(lib->first))
        {
        librariesLinked += libLinkFlag;
        }
      librariesLinked += lib->first;
      if(linkSuffix.size() && !hasSuffix.find(lib->first))
        {
        librariesLinked += linkSuffix;
        }
      librariesLinked += " ";
      }
    }

  linkLibs += librariesLinked;

  fout << linkLibs;

  if(outputRuntime && runtimeDirs.size()>0)
    {
    // For the runtime search directories, do a "-Wl,-rpath,a:b:c" or
    // a "-R a -R b -R c" type link line
    fout << runtimeFlag;
    std::vector<std::string>::iterator itr = runtimeDirs.begin();
    fout << *itr;
    ++itr;
    for( ; itr != runtimeDirs.end(); ++itr )
      {
      if(runtimeConcatenate)
        {
        fout << runtimeSep << *itr;
        }
      else
        {
        fout << " " << runtimeFlag << *itr;
        }
      }
    fout << " ";
    }
  if(m_Makefile->GetDefinition("CMAKE_STANDARD_LIBRARIES"))
    {
    fout << m_Makefile->GetDefinition("CMAKE_STANDARD_LIBRARIES") << " ";
    }
}

std::string cmLocalUnixMakefileGenerator::CreatePreBuildRules(
  const cmTarget &target, const char* /* targetName */)
{
  std::string customRuleCode = "";
  bool initNext = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPreBuildCommands().begin(); 
       cr != target.GetPreBuildCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(initNext)
      {
      customRuleCode += "\n\t";
      }
    else
      {
      initNext = true;
      }
    std::string command = cmSystemTools::ConvertToOutputPath(cc.GetCommand().c_str());
    customRuleCode += command + " " + cc.GetArguments();
    }
  return customRuleCode;
}

std::string cmLocalUnixMakefileGenerator::CreatePreLinkRules(
  const cmTarget &target, const char* /* targetName */)
{
  std::string customRuleCode = "";
  bool initNext = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPreLinkCommands().begin(); 
       cr != target.GetPreLinkCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(initNext)
      {
      customRuleCode += "\n\t";
      }
    else
      {
      initNext = true;
      }
    std::string command = cmSystemTools::ConvertToOutputPath(cc.GetCommand().c_str());
    customRuleCode += command + " " + cc.GetArguments();
    }
  return customRuleCode;
}

std::string cmLocalUnixMakefileGenerator::CreatePostBuildRules(
  const cmTarget &target, const char* /* targetName */)
{
  std::string customRuleCode = "";
  bool initNext = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPostBuildCommands().begin(); 
       cr != target.GetPostBuildCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(initNext)
      {
      customRuleCode += "\n\t";
      }
    else
      {
      initNext = true;
      }
    std::string command = cmSystemTools::ConvertToOutputPath(cc.GetCommand().c_str());
    customRuleCode += command + " " + cc.GetArguments();
    }
  return customRuleCode;
}

struct RuleVariables
{
  const char* replace;
  const char* lookup;
};

static RuleVariables ruleReplaceVars[] =
{
  {"<CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS>", "CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS"},
  {"<CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS>", "CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS"}, 
  {"<CMAKE_SHARED_MODULE_C_FLAGS>", "CMAKE_SHARED_MODULE_C_FLAGS"},
  {"<CMAKE_SHARED_MODULE_CXX_FLAGS>", "CMAKE_SHARED_MODULE_CXX_FLAGS"},
  {"<CMAKE_SHARED_LIBRARY_C_FLAGS>", "CMAKE_SHARED_LIBRARY_C_FLAGS"},
  {"<CMAKE_SHARED_LIBRARY_CXX_FLAGS>", "CMAKE_SHARED_LIBRARY_CXX_FLAGS"},
  {"<CMAKE_CXX_LINK_FLAGS>", "CMAKE_CXX_LINK_FLAGS"},

  {"<CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS>", "CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS"},
  {"<CMAKE_SHARED_MODULE_CREATE_C_FLAGS>", "CMAKE_SHARED_MODULE_CREATE_C_FLAGS"}, 
  {"<CMAKE_C_LINK_FLAGS>", "CMAKE_C_LINK_FLAGS"},

  {"<CMAKE_AR>", "CMAKE_AR"},
  {"<CMAKE_RANLIB>", "CMAKE_RANLIB"},
  {0, 0}
};



 
void 
cmLocalUnixMakefileGenerator::ExpandRuleVariables(std::string& s,
                                                  const char* objects,
                                                  const char* target,
                                                  const char* linkLibs,
                                                  const char* source,
                                                  const char* object,
                                                  const char* flags,
                                                  const char* objectsquoted,
                                                  const char* targetBase,
                                                  const char* linkFlags)
{ 
  std::string cxxcompiler = this->ConvertToOutputForExisting(
    this->GetSafeDefinition("CMAKE_CXX_COMPILER"));
  std::string ccompiler = this->ConvertToOutputForExisting(
    this->GetSafeDefinition("CMAKE_C_COMPILER"));
  cmSystemTools::ReplaceString(s, "<CMAKE_CXX_COMPILER>", cxxcompiler.c_str());
  cmSystemTools::ReplaceString(s, "<CMAKE_C_COMPILER>", ccompiler.c_str());
  if(linkFlags)
    {
    cmSystemTools::ReplaceString(s, "<LINK_FLAGS>", linkFlags);
    }
  if(flags)
    {
    cmSystemTools::ReplaceString(s, "<FLAGS>", flags);
    }
    
  if(source)
    {
    cmSystemTools::ReplaceString(s, "<SOURCE>", source);
    }
  if(object)
    {
    cmSystemTools::ReplaceString(s, "<OBJECT>", object);
    }
  if(objects)
    {
    cmSystemTools::ReplaceString(s, "<OBJECTS>", objects);
    }
  if(objectsquoted)
    {
    cmSystemTools::ReplaceString(s, "<OBJECTS_QUOTED>", objectsquoted);
    }
  if(target)
    {
    cmSystemTools::ReplaceString(s, "<TARGET>", target);
    }
  if(targetBase)
    {
    // special case for quoted paths with spaces 
    // if you see <TARGET_BASE>.lib then put the .lib inside
    // the quotes, same for .dll
    if((strlen(targetBase) > 1) && targetBase[0] == '\"')
      {
      std::string base = targetBase;
      base[base.size()-1] = '.';
      std::string baseLib = base + "lib\"";
      std::string baseDll = base + "dll\"";
      cmSystemTools::ReplaceString(s, "<TARGET_BASE>.lib", baseLib.c_str());
      cmSystemTools::ReplaceString(s, "<TARGET_BASE>.dll", baseDll.c_str());
      }
    cmSystemTools::ReplaceString(s, "<TARGET_BASE>", targetBase);
    }
  if(linkLibs)
    {
    cmSystemTools::ReplaceString(s, "<LINK_LIBRARIES>", linkLibs);
    }
  
  RuleVariables* rv = ruleReplaceVars;
  while(rv->replace)
    {
    cmSystemTools::ReplaceString(s, rv->replace,
                                 this->GetSafeDefinition(rv->lookup));
    rv++;
    }  
}

  
void cmLocalUnixMakefileGenerator::OutputLibraryRule(std::ostream& fout,  
                                                     const char* name, 
                                                     const cmTarget &t,
                                                     const char* prefix,
                                                     const char* suffix,
                                                     const char* createVariable,
                                                     const char* comment,
                                                     const char* linkFlags
                                                     )
{
  // create the library name
  std::string targetNameBase = prefix;
  targetNameBase += name;
  
  std::string targetName = prefix;
  targetName += name;
  targetName +=  suffix;
  // create the target full path name
  std::string targetFullPath = m_LibraryOutputPath + targetName;
  std::string targetBaseFullPath = m_LibraryOutputPath + targetNameBase;
  targetBaseFullPath =
    cmSystemTools::ConvertToOutputPath(targetBaseFullPath.c_str());
  targetFullPath = cmSystemTools::ConvertToOutputPath(targetFullPath.c_str());
  // get the objects that are used to link this library
  std::string objs = "$(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  std::string objsQuoted = "$(" + this->CreateMakeVariable(name, "_SRC_OBJS_QUOTED") + ") ";
  // create a variable with the objects that this library depends on
  std::string depend = objs + " $(" 
    + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  // collect up the build rules
  std::vector<std::string> rules;
  std::string command = "$(RM) " +  targetFullPath;
  rules.push_back(command);
  rules.push_back(m_Makefile->GetDefinition(createVariable));
  // expand multi-command semi-colon separated lists
  // of commands into separate commands
  std::vector<std::string> commands;
  // collect custom commands for this target and add them to the list
  std::string customCommands = this->CreatePreBuildRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands);
    }
  // collect custom commands for this target and add them to the list
  customCommands = this->CreatePreLinkRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands);
    }
  cmSystemTools::ExpandList(rules, commands);
  // collect custom commands for this target and add them to the list
  customCommands = this->CreatePostBuildRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands);
    }
  // collect up the link libraries
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, name, t);
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i, 
                              objs.c_str(), 
                              targetFullPath.c_str(),
                              linklibs.str().c_str(),
                              0, 0, 0, objsQuoted.c_str(),
                              targetBaseFullPath.c_str(),
                              linkFlags);
    }
  this->OutputMakeRule(fout, comment,
                       targetFullPath.c_str(),
                       depend.c_str(),
                       commands);
}

void cmLocalUnixMakefileGenerator::OutputSharedLibraryRule(std::ostream& fout,  
                                                           const char* name, 
                                                           const cmTarget &t)
{
  const char* createRule;
  if(t.HasCxx())
    {
    createRule = "CMAKE_CXX_CREATE_SHARED_LIBRARY";
    }
  else
    {
    createRule = "CMAKE_C_CREATE_SHARED_LIBRARY";
    }
  std::string buildType =  this->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType); 
  std::string linkFlags = this->GetSafeDefinition("CMAKE_SHARED_LINKER_FLAGS");
  linkFlags += " ";
  if(buildType.size())
    {
    std::string build = "CMAKE_SHARED_LINKER_FLAGS_";
    build += buildType;
    linkFlags += this->GetSafeDefinition(build.c_str());
    linkFlags += " ";
    }
#ifdef _WIN32
  const std::vector<cmSourceFile*>& sources = t.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
      i != sources.end(); ++i)
    {
    if((*i)->GetSourceExtension() == "def")
      {
      linkFlags += this->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
      linkFlags += cmSystemTools::ConvertToOutputPath((*i)->GetFullPath().c_str());
      linkFlags += " ";
      }
    }
#endif
  const char* targetPrefix = t.GetProperty("PREFIX");
  if(!targetPrefix)
    {
    targetPrefix = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_PREFIX");
    }
  const char* targetSuffix = t.GetProperty("SUFFIX");
  if(!targetSuffix)
    {
    targetSuffix = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
    }
  const char* targetLinkFlags = t.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    linkFlags += targetLinkFlags;
    linkFlags += " ";
    }
  this->OutputLibraryRule(fout, name, t,
                          targetPrefix,
                          targetSuffix,
                          createRule,
                          "shared library",
                          linkFlags.c_str());
}

void cmLocalUnixMakefileGenerator::OutputModuleLibraryRule(std::ostream& fout, 
                                                      const char* name, 
                                                      const cmTarget &t)
{
  const char* createRule;
  if(t.HasCxx())
    {
    createRule = "CMAKE_CXX_CREATE_SHARED_MODULE";
    }
  else
    {
    createRule = "CMAKE_C_CREATE_SHARED_MODULE";
    }
  std::string buildType =  this->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType); 
  std::string linkFlags = this->GetSafeDefinition("CMAKE_MODULE_LINKER_FLAGS");
  linkFlags += " ";
  if(buildType.size())
    {
    std::string build = "CMAKE_MODULE_LINKER_FLAGS_";
    build += buildType;
    linkFlags += this->GetSafeDefinition(build.c_str());
    linkFlags += " ";
    }
  const char* targetPrefix = t.GetProperty("PREFIX");
  if(!targetPrefix)
    {
    targetPrefix = this->GetSafeDefinition("CMAKE_SHARED_MODULE_PREFIX");
    }
  const char* targetSuffix = t.GetProperty("SUFFIX");
  if(!targetSuffix)
    {
    targetSuffix = this->GetSafeDefinition("CMAKE_SHARED_MODULE_SUFFIX");
    }
  const char* targetLinkFlags = t.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    linkFlags += targetLinkFlags;
    linkFlags += " ";
    }
  this->OutputLibraryRule(fout, name, t,
                          targetPrefix,
                          targetSuffix,
                          createRule,
                          "shared module",
                          linkFlags.c_str());
}


void cmLocalUnixMakefileGenerator::OutputStaticLibraryRule(std::ostream& fout,
                                                      const char* name, 
                                                      const cmTarget &t)
{
  const char* createRule;
  if(t.HasCxx())
    {
    createRule = "CMAKE_CXX_CREATE_STATIC_LIBRARY";
    }
  else
    {
    createRule = "CMAKE_C_CREATE_STATIC_LIBRARY";
    }  
  const char* targetPrefix = t.GetProperty("PREFIX");
  if(!targetPrefix)
    {
    targetPrefix = this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
    }
  const char* targetSuffix = t.GetProperty("SUFFIX");
  if(!targetSuffix)
    {
    targetSuffix = this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
    }
  std::string linkFlags;
  const char* targetLinkFlags = t.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    linkFlags += targetLinkFlags;
    linkFlags += " ";
    }
  this->OutputLibraryRule(fout, name, t,
                          targetPrefix,
                          targetSuffix,
                          createRule,
                          "static library",
                          linkFlags.c_str());
  
}

void cmLocalUnixMakefileGenerator::OutputExecutableRule(std::ostream& fout,
                                                        const char* name,
                                                        const cmTarget &t)
{
  std::string linkFlags;

  std::string buildType =  this->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);
  std::string flags;
  
  // Construct the full path to the executable that will be generated.
  std::string target = m_ExecutableOutputPath;
  if(target.length() == 0)
    {
    target = m_Makefile->GetCurrentOutputDirectory();
    if(target.size() && target[target.size()-1] != '/')
      {
      target += "/";
      }
    }
  target += name;
  target += cmSystemTools::GetExecutableExtension();
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  
  std::string objs = "$(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") 
    + ") $(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::vector<std::string> rules;
  linkFlags += this->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
  linkFlags += " ";
  if(buildType.size())
    {
    std::string build = "CMAKE_EXE_LINKER_FLAGS_";
    build += buildType;
    linkFlags += this->GetSafeDefinition(build.c_str());
    linkFlags += " ";
    }

  if(t.HasCxx())
    {
    rules.push_back(m_Makefile->GetDefinition("CMAKE_CXX_LINK_EXECUTABLE"));
    flags += this->GetSafeDefinition("CMAKE_CXX_FLAGS");
    flags += " ";
    flags += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_CXX_FLAGS");
    flags += " ";
    }
  else
    {
    rules.push_back(m_Makefile->GetDefinition("CMAKE_C_LINK_EXECUTABLE"));
    flags += this->GetSafeDefinition("CMAKE_C_FLAGS");
    flags += " ";
    flags += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_C_FLAGS");
    flags += " ";
    }
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, 0, t);
  std::string comment = "executable";
  
  std::vector<std::string> commands;
  std::string customCommands = this->CreatePreBuildRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands.c_str());
    }
  customCommands = this->CreatePreLinkRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands.c_str());
    }
  cmSystemTools::ExpandList(rules, commands);
  customCommands = this->CreatePostBuildRules(t, name);
  if(customCommands.size() > 0)
    {
    commands.push_back(customCommands.c_str());
    }
  if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    linkFlags += this->GetSafeDefinition("CMAKE_SHARED_BUILD_CXX_FLAGS");
    linkFlags += " ";
   }
  
  if(t.GetType() == cmTarget::WIN32_EXECUTABLE)
    {
    linkFlags +=  this->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE");
    linkFlags += " ";
    }
  else
    {
    linkFlags +=  this->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE");
    linkFlags += " ";
    }
  const char* targetLinkFlags = t.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    linkFlags += targetLinkFlags;
    linkFlags += " ";
    }
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i, 
                              objs.c_str(), 
                              target.c_str(),
                              linklibs.str().c_str(),
                              0,
                              0,
                              flags.c_str(),
                              0,
                              0,
                              linkFlags.c_str());
    }
  this->OutputMakeRule(fout, 
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       commands);
  
  // If there is no executable output path, add a rule with the
  // relative path to the executable.  This is necessary for
  // try-compile to work in this case.
  depend = target;
  target = name;
  target += cmSystemTools::GetExecutableExtension();
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  commands.resize(0);
  this->OutputMakeRule(fout, 
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       commands);
}



void cmLocalUnixMakefileGenerator::OutputUtilityRule(std::ostream& fout,
                                                const char* name,
                                                const cmTarget &t)
{
  const char* cc = 0;
  std::string customCommands = this->CreatePreBuildRules(t, name);
  std::string customCommands2 = this->CreatePreLinkRules(t, name);
  if(customCommands2.size() > 0)
    {
    customCommands += customCommands2;
    }
  customCommands2 = this->CreatePostBuildRules(t, name);
  if(customCommands2.size() > 0)
    {
    customCommands += customCommands2;
    }
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  std::string comment = "Utility";
  std::string depends;
  depends = "$(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string replaceVars;
  const std::vector<cmCustomCommand> &ccs = t.GetPostBuildCommands();
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    const std::vector<std::string>  & dep = i->GetDepends();
    for(std::vector<std::string>::const_iterator d = dep.begin();
        d != dep.end(); ++d)
      {
      depends +=  " \\\n";
      replaceVars = *d;
      m_Makefile->ExpandVariablesInString(replaceVars);
      depends += cmSystemTools::ConvertToOutputPath(replaceVars.c_str());
      }
    }
  this->OutputMakeRule(fout, comment.c_str(), name,
                       depends.c_str(), cc);
}

  

void cmLocalUnixMakefileGenerator::OutputTargets(std::ostream& fout)
{
  // for each target
  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    switch(l->second.GetType())
      {
      case cmTarget::STATIC_LIBRARY:
        this->OutputStaticLibraryRule(fout, l->first.c_str(), l->second);
        break;
      case cmTarget::SHARED_LIBRARY:
        this->OutputSharedLibraryRule(fout, l->first.c_str(), l->second);
        break;
      case cmTarget::MODULE_LIBRARY:
        this->OutputModuleLibraryRule(fout, l->first.c_str(), l->second);
        break;
      case cmTarget::EXECUTABLE:
      case cmTarget::WIN32_EXECUTABLE:
        this->OutputExecutableRule(fout, l->first.c_str(), l->second);
        break;
      case cmTarget::UTILITY:
        this->OutputUtilityRule(fout, l->first.c_str(), l->second);
        break;
        // This is handled by the OutputCustomRules method
      case cmTarget::INSTALL_FILES:
        // This is handled by the OutputInstallRules method
      case cmTarget::INSTALL_PROGRAMS:
        // This is handled by the OutputInstallRules method
        break;
      }
    }
}



// For each target that is an executable or shared library, generate
// the "<name>_DEPEND_LIBS" variable listing its library dependencies.
void cmLocalUnixMakefileGenerator::OutputDependLibs(std::ostream& fout)
{
  // Build a set of libraries that will be linked into any target in
  // this directory.
  std::set<std::string> used;
  
  // for each target
  const cmTargets &tgts = m_Makefile->GetTargets();
  
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    // Each dependency should only be emitted once per target.
    std::set<std::string> emitted;
    if ((l->second.GetType() == cmTarget::SHARED_LIBRARY)
        || (l->second.GetType() == cmTarget::MODULE_LIBRARY)
        || (l->second.GetType() == cmTarget::STATIC_LIBRARY)
        || (l->second.GetType() == cmTarget::EXECUTABLE)
        || (l->second.GetType() == cmTarget::UTILITY)
        || (l->second.GetType() == cmTarget::WIN32_EXECUTABLE))
      {
      fout << this->CreateMakeVariable(l->first.c_str(), "_DEPEND_LIBS") << " = ";
      
      // A library should not depend on itself!
      emitted.insert(l->first);
      // for static libraries do not depend on other libraries
      if(l->second.GetType() != cmTarget::STATIC_LIBRARY)
        {
        // Now, look at all link libraries specific to this target.
        const cmTarget::LinkLibraries& tlibs = l->second.GetLinkLibraries();
        for(cmTarget::LinkLibraries::const_iterator lib = tlibs.begin();
            lib != tlibs.end(); ++lib)
          {
          // Record that this library was used.
          used.insert(lib->first);
          
          // Don't emit the same library twice for this target.
          if(emitted.insert(lib->first).second)
            {
            // Output this dependency.
            this->OutputLibDepend(fout, lib->first.c_str());
            }
          }
        }
      // for all targets depend on utilities
      // Now, look at all utilities specific to this target.
      const std::set<cmStdString>& tutils = l->second.GetUtilities();
      for(std::set<cmStdString>::const_iterator util = tutils.begin();
          util != tutils.end(); ++util)
        { 
        // Record that this utility was used.
        used.insert(*util);

        // Don't emit the same utility twice for this target.
        if(emitted.insert(*util).second)
          {
          // Output this dependency.
          this->OutputExeDepend(fout, util->c_str());
          }
        }
      fout << "\n";
      }
    }

  fout << "\n";
  
  // Loop over the libraries used and make sure there is a rule to
  // build them in this makefile.  If the library is in another
  // directory, add a rule to jump to that directory and make sure it
  // exists.
  for(std::set<std::string>::const_iterator lib = used.begin();
      lib != used.end(); ++lib)
    {
    // loop over the list of directories that the libraries might
    // be in, looking for an ADD_LIBRARY(lib...) line. This would
    // be stored in the cache
    std::string libPath = *lib + "_CMAKE_PATH";
    const char* cacheValue = m_Makefile->GetDefinition(libPath.c_str());

    // if cache and not the current directory add a rule, to
    // jump into the directory and build for the first time
    if(cacheValue &&
       (!this->SamePath(m_Makefile->GetCurrentOutputDirectory(), cacheValue)))
      {
      // add the correct extension
      std::string ltname = *lib+"_LIBRARY_TYPE";
      const char* libType
        = m_Makefile->GetDefinition(ltname.c_str());
      // if it was a library..
      if (libType)
        {
        std::string library;
        std::string libpath = cacheValue;
        if(libType && std::string(libType) == "SHARED")
          {
          library = this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_PREFIX");
          library += *lib;
          library += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
          }
        else if(libType && std::string(libType) == "MODULE")
          {
          library = this->GetSafeDefinition("CMAKE_SHARED_MODULE_PREFIX");
          library += *lib;
          library += this->GetSafeDefinition("CMAKE_SHARED_MODULE_SUFFIX");
          }
        else if(libType && std::string(libType) == "STATIC")
          {
          library = this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
          library += *lib;
          library += this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
          }
        else
          {
          cmSystemTools::Error("Unknown library type!");
          return;
          }
        if(m_LibraryOutputPath.size())
          {
          libpath = m_LibraryOutputPath;
          }
        else
          {
          libpath += "/";
          }
        libpath += library;
        // put out a rule to build the library if it does not exist
        this->OutputBuildTargetInDir(fout,
                                     cacheValue,
                                     library.c_str(),
                                     libpath.c_str(),
                                     m_Makefile->
                                     GetDefinition("LIBRARY_OUTPUT_PATH")
                                     );
        }
      // something other than a library...
      else
        {
        std::string exepath = cacheValue;
        if(m_ExecutableOutputPath.size())
          {
          exepath = m_ExecutableOutputPath;
          }
        else
          {
          exepath += "/";
          }
        std::string fullName = lib->c_str();
        fullName += cmSystemTools::GetExecutableExtension();
        exepath += fullName;
        this->OutputBuildTargetInDir(fout,
                                     cacheValue,
                                     fullName.c_str(),
                                     exepath.c_str(),
                                     m_Makefile->
                                     GetDefinition("EXECUTABLE_OUTPUT_PATH")
          );
        }
      }
    }
}

void cmLocalUnixMakefileGenerator::OutputBuildTargetInDirWindows(std::ostream& fout,
                                                                 const char* path,
                                                                 const char* library,
                                                                 const char* fullpath,
                                                                 const char* libOutPath)
{
  const char* makeTarget = library;
  std::string currentDir = 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetCurrentOutputDirectory());
  std::string wpath = this->ConvertToOutputForExisting(path);
  std::string wfullpath = this->ConvertToOutputForExisting(fullpath);
  if(libOutPath && strcmp( libOutPath, "" ) != 0)
    {
    makeTarget = wfullpath.c_str();
    }
  fout << wfullpath
       << ":\n\tcd " << wpath  << "\n"
       << "\t$(MAKE) -$(MAKEFLAGS) $(MAKESILENT) cmake.depends\n"
       << "\t$(MAKE) -$(MAKEFLAGS) $(MAKESILENT) cmake.check_depends\n"
       << "\t$(MAKE) -$(MAKEFLAGS) $(MAKESILENT) -f cmake.check_depends\n"
       << "\t$(MAKE) $(MAKESILENT) " << makeTarget
       << "\n\tcd " << currentDir << "\n";
}

void cmLocalUnixMakefileGenerator::OutputBuildTargetInDir(std::ostream& fout,
                                                     const char* path,
                                                     const char* library,
                                                     const char* fullpath,
                                                     const char* outputPath)
{
  if(m_WindowsShell)
    {
    this->OutputBuildTargetInDirWindows(fout, path, library, fullpath, outputPath);
    return;
    }
  const char* makeTarget = library;
  if(outputPath && strcmp( outputPath, "" ) != 0)
    {
    makeTarget = fullpath;
    }
  fout << this->ConvertToOutputForExisting(fullpath)
       << ":\n\tcd " << this->ConvertToOutputForExisting(path)
       << "; $(MAKE) $(MAKESILENT) cmake.depends"
       << "; $(MAKE) $(MAKESILENT) cmake.check_depends"
       << "; $(MAKE) $(MAKESILENT) -f cmake.check_depends"
       << "; $(MAKE) $(MAKESILENT) "
       << cmSystemTools::ConvertToOutputPath(makeTarget) << "\n\n"; 
}


bool cmLocalUnixMakefileGenerator::SamePath(const char* path1, const char* path2)
{
  if (strcmp(path1, path2) == 0)
    {
    return true;
    }
#if defined(_WIN32) || defined(__APPLE__)
  return 
    cmSystemTools::LowerCase(this->ConvertToOutputForExisting(path1)) ==
    cmSystemTools::LowerCase(this->ConvertToOutputForExisting(path2));
#else
  return false;
#endif
}

void cmLocalUnixMakefileGenerator::OutputLibDepend(std::ostream& fout,
                                              const char* name)
{
  std::string libPath = name;
  libPath += "_CMAKE_PATH";
  const char* cacheValue = m_Makefile->GetDefinition(libPath.c_str());
  if(cacheValue )
    {
    // if there is a cache value, then this is a library that cmake
    // knows how to build, so we can depend on it
    std::string libpath;
    if (!this->SamePath(m_Makefile->GetCurrentOutputDirectory(), cacheValue))
      {
      // if the library is not in the current directory, then get the full
      // path to it
      if(m_LibraryOutputPath.size())
        {
        libpath = m_LibraryOutputPath;
        }
      else
        {
        libpath = cacheValue;
        libpath += "/";
        }
      }
    else
      {
      // library is in current Makefile so use lib as a prefix
      libpath = m_LibraryOutputPath;
      }
    // add the correct extension
    std::string ltname = name;
    ltname += "_LIBRARY_TYPE";
    const char* libType = m_Makefile->GetDefinition(ltname.c_str());
    if(libType && std::string(libType) == "SHARED")
      {
      libpath += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_PREFIX");
      libpath += name;
      libpath += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
      }
    else if (libType && std::string(libType) == "MODULE")
      {
      libpath += this->GetSafeDefinition("CMAKE_SHARED_MODULE_PREFIX");
      libpath += name;
      libpath += this->GetSafeDefinition("CMAKE_SHARED_MODULE_SUFFIX");
      }
    else if (libType && std::string(libType) == "STATIC")
      {
      libpath += this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
      libpath += name;
      libpath += this->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
      }
    fout << cmSystemTools::ConvertToOutputPath(libpath.c_str()) << " ";
    }
}


void cmLocalUnixMakefileGenerator::OutputExeDepend(std::ostream& fout,
                                              const char* name)
{
  std::string exePath = name;
  exePath += "_CMAKE_PATH";
  const char* cacheValue = m_Makefile->GetDefinition(exePath.c_str());
  if(cacheValue )
    {
    // if there is a cache value, then this is a executable/utility that cmake
    // knows how to build, so we can depend on it
    std::string exepath;
    if (!this->SamePath(m_Makefile->GetCurrentOutputDirectory(), cacheValue))
      {
      // if the exe/utility is not in the current directory, then get the full
      // path to it
      if(m_ExecutableOutputPath.size())
        {
        exepath = m_ExecutableOutputPath;
        }
      else
        {
        exepath = cacheValue;
        exepath += "/";
        }
      }
    else
      {
      // library is in current Makefile
      exepath = m_ExecutableOutputPath;
      }
    // add the library name
    exepath += name;
    // add the correct extension
    exepath += cmSystemTools::GetExecutableExtension();
    fout << cmSystemTools::ConvertToOutputPath(exepath.c_str()) << " ";
    }
  // if it isn't in the cache, it might still be a utility target
  // so check for that
  else
    {
    std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
    if (targets.find(name) != targets.end())
      {
      fout << name << " ";
      }
    }
  
}



// fix up names of directories so they can be used
// as targets in makefiles.
inline std::string FixDirectoryName(const char* dir)
{
  std::string s = dir;
  // replace ../ with 3 under bars
  size_t pos = s.find("../");
  if(pos != std::string::npos)
    {
    s.replace(pos, 3, "___");
    }
  // replace / directory separators with a single under bar 
  pos = s.find("/");
  while(pos != std::string::npos)
    {
    s.replace(pos, 1, "_");
    pos = s.find("/");
    }
  return s;
}


void cmLocalUnixMakefileGenerator::
BuildInSubDirectoryWindows(std::ostream& fout,
                           const char* directory,
                           const char* target1,
                           const char* target2,
                           bool silent)
{
  if(target1)
    {
    std::string dir = this->ConvertToOutputForExisting(directory);
    if(dir[0] == '\"')
      {
      dir = dir.substr(1, dir.size()-2);
      }
    fout << "\tif not exist \"" << dir << "\\$(NULL)\""
         << " " 
         << "$(MAKE) $(MAKESILENT) rebuild_cache\n";
    if (!silent) 
      {
      fout << "\t@echo " << directory << ": building " << target1 << "\n";
      }
    fout << "\tcd " << dir << "\n"
         << "\t$(MAKE) -$(MAKEFLAGS) $(MAKESILENT) " << target1 << "\n";
    }
  if(target2)
    {
    if (!silent) 
      {
      fout << "\t@echo " << directory << ": building " << target2 << "\n";
      }
    fout << "\t$(MAKE) -$(MAKEFLAGS) $(MAKESILENT) " << target2 << "\n";
    }
  std::string currentDir = m_Makefile->GetCurrentOutputDirectory();
  fout << "\tcd " << this->ConvertToOutputForExisting(currentDir.c_str()) << "\n\n";
}


void cmLocalUnixMakefileGenerator::BuildInSubDirectory(std::ostream& fout,
                                                  const char* dir,
                                                  const char* target1,
                                                  const char* target2,
                                                  bool silent)
{
  if(m_WindowsShell)
    {
    this->BuildInSubDirectoryWindows(fout, dir, target1, target2, silent);
    return;
    }
  
  std::string directory = cmSystemTools::ConvertToOutputPath(dir);
  if(target1)
    {
    fout << "\t@if test ! -d " << directory 
         << "; then $(MAKE) rebuild_cache; fi\n";
    if (!silent) 
      {
      fout << "\t@echo " << directory << ": building " << target1 << "\n";
      }
    fout << "\t@cd " << directory
         << "; $(MAKE) " << target1 << "\n";
    }
  if(target2)
    {
    if (!silent) 
      {
      fout << "\t@echo " << directory << ": building " << target2 << "\n";
      }
    fout << "\t@cd " << directory
         << "; $(MAKE) " << target2 << "\n";
    }
  fout << "\n";
}


void 
cmLocalUnixMakefileGenerator::
OutputSubDirectoryVars(std::ostream& fout,
                       const char* var,
                       const char* target,
                       const char* target1,
                       const char* target2,
                       const char* depend,
                       const std::vector<std::string>& SubDirectories,
                       bool silent)
{
  if(!depend)
    {
    depend = "";
    }
  if( SubDirectories.size() == 0)
    {
    return;
    }
  fout << "# Variable for making " << target << " in subdirectories.\n";
  fout << var << " = \\\n";
  unsigned int ii;
  for(ii =0; ii < SubDirectories.size(); ii++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[ii].c_str());
    fout << target << "_" << subdir.c_str();
    if(ii == SubDirectories.size()-1)
      {
      fout << " \n\n";
      }
    else
      {
      fout << " \\\n";
      }
    }
  fout << "# Targets for making " << target << " in subdirectories.\n";
  std::string last = "";
  for(unsigned int cc =0; cc < SubDirectories.size(); cc++)
    {
    std::string subdir = FixDirectoryName(SubDirectories[cc].c_str());
    fout << target << "_" << subdir.c_str() << ": " << depend;
    
    // Make each subdirectory depend on previous one.  This forces
    // parallel builds (make -j 2) to build in same order as a single
    // threaded build to avoid dependency problems.
    if(cc > 0)
      {
      fout << " " << target << "_" << last.c_str();
      }
    
    fout << "\n";
    last = subdir;
    std::string dir = m_Makefile->GetCurrentOutputDirectory();
    dir += "/";
    dir += SubDirectories[cc];
    this->BuildInSubDirectory(fout, dir.c_str(),
                              target1, target2, silent);
    }
  fout << "\n\n";
}


// output rules for decending into sub directories
void cmLocalUnixMakefileGenerator::OutputSubDirectoryRules(std::ostream& fout)
{
    // Output Sub directory build rules
  const std::vector<std::string>& SubDirectories
    = m_Makefile->GetSubDirectories();
    
  if( SubDirectories.size() == 0)
    {
    return;
    }
  
  this->OutputSubDirectoryVars(fout, 
                               "SUBDIR_BUILD",
                               "default_target",
                               "default_target",
                               0, "$(TARGETS)",
                               SubDirectories,
                               false);
  this->OutputSubDirectoryVars(fout, "SUBDIR_CLEAN", "clean",
                               "clean",
                               0, 0,
                               SubDirectories);
  this->OutputSubDirectoryVars(fout, "SUBDIR_DEPEND", "depend",
                               "depend",
                               0, 0,
                               SubDirectories);
  this->OutputSubDirectoryVars(fout, "SUBDIR_INSTALL", "install",
                               "install",
                               0, 0,
                               SubDirectories);
}

// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend GenerateMakefile
bool cmLocalUnixMakefileGenerator::OutputObjectDepends(std::ostream& fout)
{
  bool ret = false;
  // Iterate over every target.
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  for(std::map<cmStdString, cmTarget>::const_iterator target = targets.begin(); 
      target != targets.end(); ++target)
    {
    // Iterate over every source for this target.
    const std::vector<cmSourceFile*>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        if(!(*source)->GetDepends().empty())
          {
          // Iterate through all the dependencies for this source.
          for(std::vector<std::string>::const_iterator dep =
                (*source)->GetDepends().begin();
              dep != (*source)->GetDepends().end(); ++dep)
            {
            std::string s = (*source)->GetSourceName();
            s += this->GetOutputExtension((*source)->GetSourceExtension().c_str());
            fout << cmSystemTools::ConvertToOutputPath(s.c_str()) << " : "
                 << cmSystemTools::ConvertToOutputPath(dep->c_str()) << "\n";
            ret = true;
            }
          fout << "\n\n";
          }
        }
      }
    }
  return ret;
}



// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend GenerateMakefile
void cmLocalUnixMakefileGenerator::OutputCheckDepends(std::ostream& fout)
{
  std::set<std::string> emittedLowerPath;
  std::set<std::string> emitted;
  // Iterate over every target.
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  fout << "# Suppresses display of executed commands\n";
  fout << ".SILENT:\n";
  fout << "# disable some common implicit rules to speed things up\n";
  fout << ".SUFFIXES:\n";
  fout << ".SUFFIXES:.hpuxmakemusthaverule\n";
  this->OutputMakeVariables(fout);
  fout << "default:\n";
  fout << "\t$(MAKE) $(MAKESILENT) -f cmake.check_depends all\n"
       << "\t$(MAKE) $(MAKESILENT) -f cmake.check_depends cmake.depends\n\n";
  for(std::map<cmStdString, cmTarget>::const_iterator target = targets.begin(); 
      target != targets.end(); ++target)
    {
    // Iterate over every source for this target.
    const std::vector<cmSourceFile*>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        if(!(*source)->GetDepends().empty())
          {
          for(std::vector<std::string>::const_iterator dep =
                (*source)->GetDepends().begin();
              dep != (*source)->GetDepends().end(); ++dep)
            {
            // do not call CollapseFullPath on dep here, because it already
            // has been done because m_FullPath on cmDependInformation
            // always is it called.  If it is called here, network builds are
            // very slow because of the number of stats
            std::string dependfile = cmSystemTools::ConvertToOutputPath(dep->c_str());
            // use the lower path function to create uniqe names
            std::string lowerpath = this->LowerCasePath(dependfile.c_str());
            if(emittedLowerPath.insert(lowerpath).second)
              {
              emitted.insert(dependfile);
              fout << "all:: " << dependfile << "\n";
              }
            }
          }
        }
      }
    }
  fout << "\n\n# if any of these files changes run make dependlocal\n";
  std::set<std::string>::iterator i;
  for(i = emitted.begin(); i != emitted.end(); ++i)
    {
    fout << "cmake.depends:: " << *i << 
      "\n\t$(MAKE) $(MAKESILENT) dependlocal\n\n";
    }
  fout << "\n\n";
  fout << "# if a .h file is removed then run make dependlocal\n\n";
  for(std::set<std::string>::iterator it = emitted.begin();
      it != emitted.end(); ++it)
    {
    fout << *it << ":\n"
         << "\t$(MAKE) $(MAKESILENT) dependlocal\n\n";
    }
}

// Output each custom rule in the following format:
// output: source depends...
//   (tab)   command...
void cmLocalUnixMakefileGenerator::OutputCustomRules(std::ostream& fout)
{
  // we cannot provide multiple rules for a single output
  // so we will keep track of outputs to make sure we don't write
  // two rules. First found wins
  std::set<std::string> processedOutputs;
  
  // first output all custom rules
  const std::vector<cmSourceFile*>& sources = m_Makefile->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
      i != sources.end(); ++i)
    {
    if ((*i)->GetCustomCommand())
      {
      cmCustomCommand *c = (*i)->GetCustomCommand();
      // escape spaces and convert to native slashes path for
      // the command
      std::string comment = c->GetComment();
      std::string command = c->GetCommand();
      cmSystemTools::ReplaceString(command, "/./", "/");
      command = cmSystemTools::ConvertToOutputPath(command.c_str());
      command += " ";
      // now add the arguments
      command += c->GetArguments();
      std::string depends;
      // Collect out all the dependencies for this rule.
      for(std::vector<std::string>::const_iterator d =
            c->GetDepends().begin();
          d != c->GetDepends().end(); ++d)
        {
        std::string dep = *d;
        m_Makefile->ExpandVariablesInString(dep);

        // watch for target dependencies,
        std::string libPath = dep + "_CMAKE_PATH";
        const char* cacheValue = m_Makefile->GetDefinition(libPath.c_str());
        if (cacheValue)
          {
          libPath = cacheValue;
          if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH") && 
              m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH")[0] != '\0')
            {
            libPath = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
            }
          libPath += "/";
          libPath += dep;
          libPath += cmSystemTools::GetExecutableExtension();
          dep = libPath;
          }
        cmSystemTools::ReplaceString(dep, "/./", "/");
        cmSystemTools::ReplaceString(dep, "/$(IntDir)/", "/");
        dep = cmSystemTools::ConvertToOutputPath(dep.c_str());
        depends += " ";
        depends += dep;
        } 
      // output rule
      if (processedOutputs.find(c->GetOutput()) == processedOutputs.end())
        {
        this->OutputMakeRule(fout,
                             (comment.size()?comment.c_str():"Custom command"),
                             c->GetOutput().c_str(),
                             depends.c_str(),
                             command.c_str());
        processedOutputs.insert(c->GetOutput());
        }
      else
        {
        cmSystemTools::Error("An output was found with multiple rules on how to build it for output: ",
                             c->GetOutput().c_str());
        }
      }
    }
}

std::string 
cmLocalUnixMakefileGenerator::ConvertToOutputForExisting(const char* p)
{
  std::string ret = cmSystemTools::ConvertToOutputPath(p);
  // if there are spaces in the path, then get the short path version
  // if there is one
  if(ret.find(' ') != std::string::npos)
    {
    if(cmSystemTools::FileExists(p))
      {
      if(!cmSystemTools::GetShortPath(ret.c_str(), ret))
        {
        ret = p;
        }
      }
    }
  return ret;
}


void cmLocalUnixMakefileGenerator::OutputMakeVariables(std::ostream& fout)
{
  const char* variables = 
    "# the standard shell for make\n"
    "SHELL = /bin/sh\n"
    "\n";
  if(!m_WindowsShell)
    {
    fout << variables;
    }
  else
    {
    fout << 
      "!IF \"$(OS)\" == \"Windows_NT\"\n"
      "NULL=\n"
      "!ELSE \n"
      "NULL=nul\n"
      "!ENDIF \n";
    }
  if(m_MakeSilentFlag.size())
    {
    fout << "MAKESILENT                             = " << m_MakeSilentFlag << "\n";
    }
  
  std::string cmakecommand = this->ConvertToOutputForExisting(
    m_Makefile->GetDefinition("CMAKE_COMMAND"));
  fout << "CMAKE_COMMAND = "
       << cmakecommand
       << "\n";
  fout << "RM = " << cmakecommand.c_str() << " -E remove -f\n"; 

  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    fout << "CMAKE_EDIT_COMMAND = "
         << this->ConvertToOutputForExisting(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
         << "\n";
    }

  fout << "CMAKE_CURRENT_SOURCE = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartDirectory()) 
       << "\n";
  fout << "CMAKE_CURRENT_BINARY = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartOutputDirectory())
       << "\n";
  fout << "CMAKE_SOURCE_DIR = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeDirectory())
       << "\n";
  fout << "CMAKE_BINARY_DIR = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeOutputDirectory())
       << "\n";
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  fout << "-I" << 
    this->ConvertToOutputForExisting(m_Makefile->GetStartDirectory()) << " ";
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    // Don't output a -I for the standard include path "/usr/include".
    // This can cause problems with certain standard library
    // implementations because the wrong headers may be found first.
    if(include != "/usr/include")
      {
      fout << "-I" << this->ConvertToOutputForExisting(i->c_str()) << " ";
      }
    }
  fout << m_Makefile->GetDefineFlags();
  fout << "\n\n";
}


void cmLocalUnixMakefileGenerator::OutputInstallRules(std::ostream& fout)
{
  // Install is not currently supported on windows
  if(m_WindowsShell)
    {
    return;
    }
  const char* root
    = m_Makefile->GetDefinition("CMAKE_ROOT");
  fout << "INSTALL = \"" << root << "/Templates/install-sh\" -c\n";
  fout << "INSTALL_PROGRAM = $(INSTALL)\n";
  fout << "INSTALL_DATA =    $(INSTALL) -m 644\n";
  
  const cmTargets &tgts = m_Makefile->GetTargets();
  fout << "install: $(SUBDIR_INSTALL)\n";
  fout << "\t@echo \"Installing ...\"\n";
  
  const char* prefix
    = m_Makefile->GetDefinition("CMAKE_INSTALL_PREFIX");
  if (!prefix)
    {
    prefix = "/usr/local";
    }

  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetInstallPath() != "")
      {
      // first make the directories for each target 
      fout << "\t@if [ ! -d \"$(DESTDIR)" << prefix << l->second.GetInstallPath() << 
        "\" ] ; then \\\n";
      fout << "\t   echo \"Making directory \\\"$(DESTDIR)" << prefix 
           << l->second.GetInstallPath() << "\\\" \"; \\\n";
      fout << "\t   mkdir -p \"$(DESTDIR)" << prefix << l->second.GetInstallPath() 
           << "\"; \\\n";
      fout << "\t   chmod 755 \"$(DESTDIR)" <<  prefix << l->second.GetInstallPath() 
           << "\"; \\\n";
      fout << "\t else true; \\\n";
      fout << "\t fi\n";
      std::string fname;
      // now install the target
      switch (l->second.GetType())
        {
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
          fname = m_LibraryOutputPath;
          fname += this->GetFullTargetName(l->first.c_str(), l->second);
          fout << "\t$(INSTALL_DATA) " << cmSystemTools::ConvertToOutputPath(fname.c_str())
               << " \"$(DESTDIR)" << prefix << l->second.GetInstallPath() << "\"\n";
          break;
        case cmTarget::WIN32_EXECUTABLE:
        case cmTarget::EXECUTABLE:
          fname = m_ExecutableOutputPath;
          fname += this->GetFullTargetName(l->first.c_str(), l->second);
          fout << "\t$(INSTALL_PROGRAM) " << cmSystemTools::ConvertToOutputPath(fname.c_str())
               << " \"$(DESTDIR)" << prefix << l->second.GetInstallPath() << "\"\n";
          break;
        case cmTarget::INSTALL_FILES:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }
            fout << "\t@echo \"Installing " << f.c_str() << " \"\n"; 
            // avoid using install-sh to install install-sh
            // does not work on windows.... 
            if(*i == "install-sh")
              {
              fout << "\tcp ";
              }
            else
              {
              fout << "\t$(INSTALL_DATA) ";
              }
            
            fout << cmSystemTools::ConvertToOutputPath(i->c_str())
                 << " \"$(DESTDIR)" << prefix << l->second.GetInstallPath() << "\"\n";
            }
          }
          break;
        case cmTarget::INSTALL_PROGRAMS:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }
            fout << "\t@echo \"Installing " << f.c_str() << " \"\n"; 
            // avoid using install-sh to install install-sh
            // does not work on windows.... 
            if(*i == "install-sh")
              {
              fout << "\t   @cp ";
              }
            else
              {
              fout << "\t   @$(INSTALL_PROGRAM) ";
              }
            fout << cmSystemTools::ConvertToOutputPath(i->c_str())
                 << " \"$(DESTDIR)" << prefix << l->second.GetInstallPath() << "\"\n";
            }
          }
          break;
        case cmTarget::UTILITY:
        default:
          break;
        }
      }
    }
}

void cmLocalUnixMakefileGenerator::OutputMakeRules(std::ostream& fout)
{
  this->OutputMakeRule(fout, 
                       "default build rule",
                       "all",
                       "cmake.depends $(TARGETS) $(SUBDIR_BUILD)",
                       0);
  this->OutputMakeRule(fout, 
                       "clean generated files",
                       "clean",
                       "$(SUBDIR_CLEAN)",
                       "-@ $(RM) $(CLEAN_OBJECT_FILES) "
                       " $(TARGETS) $(GENERATED_QT_FILES) $(GENERATED_FLTK_FILES)");

  // collect up all the sources
  std::vector<std::string> allsources;
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  for(std::map<cmStdString,cmTarget>::const_iterator target = targets.begin(); 
      target != targets.end(); ++target)
    {
    // Iterate over every source for this target.
    const std::vector<cmSourceFile*>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
          allsources.push_back(cmSystemTools::ConvertToOutputPath((*source)->GetFullPath().c_str()));
        }
      }
    }

  std::string checkCache = m_Makefile->GetHomeOutputDirectory();
  checkCache += "/cmake.check_cache";
  checkCache = cmSystemTools::ConvertToOutputPath(checkCache.c_str());
  std::vector<std::string> cmake_depends;
  cmake_depends.push_back(checkCache);
  cmake_depends.push_back("$(CMAKE_MAKEFILE_SOURCES)");
  
  this->OutputMakeRule(fout, 
                       "dependencies.",
                       "cmake.depends",
                       cmake_depends,
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
  this->OutputMakeRule(fout, 
                       "dependencies",
                       "cmake.check_depends",
                       allsources,
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
  
  this->OutputMakeRule(fout, 
                       "dependencies",
                       "depend",
                       "$(SUBDIR_DEPEND)",
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");  
  this->OutputMakeRule(fout, 
                       "dependencies",
                       "dependlocal",
                       0,
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");  

  this->OutputMakeRule(fout,
                       "CMakeCache.txt",
                       "rebuild_cache",
                       "$(CMAKE_BINARY_DIR)/CMakeCache.txt",
                       "$(CMAKE_COMMAND) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");  

  std::vector<std::string> check_cache_depends;
  std::string CMakeCache = m_Makefile->GetHomeOutputDirectory();
  CMakeCache += "/CMakeCache.txt";
  CMakeCache = cmSystemTools::ConvertToOutputPath(CMakeCache.c_str());
  check_cache_depends.push_back(CMakeCache);
  check_cache_depends.push_back("$(CMAKE_MAKEFILE_SOURCES)");

  this->OutputMakeRule(fout, 
                       "CMakeCache.txt because out-of-date:",
                       checkCache.c_str(),
                       check_cache_depends,
                       "$(CMAKE_COMMAND) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
  // if CMAKE_EDIT_COMMAND is defined then add a rule to run it
  // called edit_cache
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    this->OutputMakeRule(fout, 
                         "edit CMakeCache.txt",
                         "edit_cache",
                         0,
                         "$(CMAKE_EDIT_COMMAND) "
                         "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
    }
  else
    {
    this->OutputMakeRule(fout, 
                         "edit CMakeCache.txt",
                         "edit_cache",
                         0,
                         "$(CMAKE_COMMAND) "
                         "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) -i");
    }
  
  this->OutputMakeRule(fout, 
                       "CMakeCache.txt",
                       "$(CMAKE_BINARY_DIR)/CMakeCache.txt",
                       0,
                       "$(CMAKE_COMMAND) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");

  this->OutputMakeRule(fout, 
                       "Rule to keep make from removing Makefiles "
                       "if control-C is hit during a run of cmake.",
                       ".PRECIOUS",
                       "Makefile cmake.depends",
                       0);
  
  this->OutputSourceObjectBuildRules(fout);
  // find ctest
  std::string ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Release/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if (!cmSystemTools::FileExists(ctest.c_str()))
    {
    if ( !m_Makefile->GetDefinition("PROJECT_NAME") ||
      strcmp(m_Makefile->GetDefinition("PROJECT_NAME"), "CMake") )
      {
      return;
      }
    ctest = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    ctest += "/ctest";
    }
  fout << "ARGS=\n";
  std::string cmd = this->ConvertToOutputForExisting(ctest.c_str());
  cmd += " $(ARGS)";
  this->OutputMakeRule(fout, 
                       "tests",
                       "test",
                       "",
                       cmd.c_str());
}

void 
cmLocalUnixMakefileGenerator::
OutputBuildObjectFromSource(std::ostream& fout,
                            const char* shortName,
                            const cmSourceFile& source,
                            const char* extraCompileFlags,
                            bool shared)
{
  // Header files shouldn't have build rules.
  if(source.GetPropertyAsBool("HEADER_FILE_ONLY"))
    {
    return;
    }

  std::string comment = "object file";
  std::string objectFile = std::string(shortName) + 
    this->GetOutputExtension(source.GetSourceExtension().c_str());
  objectFile = cmSystemTools::ConvertToOutputPath(objectFile.c_str());
  cmSystemTools::FileFormat format = 
    cmSystemTools::GetFileFormat(source.GetSourceExtension().c_str());
  std::vector<std::string> rules;
  std::string flags;
  if(extraCompileFlags)
    {
    flags += extraCompileFlags;
    }
  std::string sourceFile = 
    cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str()); 
  std::string buildType =  this->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);
  switch(format)
    {
    case cmSystemTools::C_FILE_FORMAT:
      {
      rules.push_back(m_Makefile->GetDefinition("CMAKE_C_COMPILE_OBJECT"));
      flags += this->GetSafeDefinition("CMAKE_C_FLAGS");
      flags += " ";
      if(buildType.size())
        {
        std::string build = "CMAKE_C_FLAGS_";
        build += buildType;
        flags +=  this->GetSafeDefinition(build.c_str());
        flags += " ";
        }
      if(shared)
        {
        flags += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_C_FLAGS");
        flags += " ";
        }  
      if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
        {
        flags += this->GetSafeDefinition("CMAKE_SHARED_BUILD_C_FLAGS");
        flags += " ";
        }
      break;
      }
    case cmSystemTools::CXX_FILE_FORMAT:
      {
      rules.push_back(m_Makefile->GetDefinition("CMAKE_CXX_COMPILE_OBJECT"));
      flags += this->GetSafeDefinition("CMAKE_CXX_FLAGS");
      flags += " "; 
      if(buildType.size())
        {
        std::string build = "CMAKE_CXX_FLAGS_";
        build += buildType;
        flags +=  this->GetSafeDefinition(build.c_str());
        flags += " ";
        }
      if(shared)
        {
        flags += this->GetSafeDefinition("CMAKE_SHARED_LIBRARY_CXX_FLAGS");
        flags += " ";
        }
      if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
        {
        flags += this->GetSafeDefinition("CMAKE_SHARED_BUILD_CXX_FLAGS");
        flags += " ";
        }
      break;
      }
    case cmSystemTools::HEADER_FILE_FORMAT:
      return;
    case cmSystemTools::DEFINITION_FILE_FORMAT:
      return;
    case cmSystemTools::RESOURCE_FILE_FORMAT:
      {
      flags = " $(INCLUDE_FLAGS) ";
      // use rc rule here if it is defined
      const char* rule = m_Makefile->GetDefinition("CMAKE_COMPILE_RESOURCE");
      if(rule)
        {
        rules.push_back(rule);
        }
      }
      break;
    case cmSystemTools::NO_FILE_FORMAT:
    case cmSystemTools::JAVA_FILE_FORMAT:
    case cmSystemTools::STATIC_LIBRARY_FILE_FORMAT:
    case cmSystemTools::SHARED_LIBRARY_FILE_FORMAT:
    case cmSystemTools::MODULE_FILE_FORMAT:
    case cmSystemTools::OBJECT_FILE_FORMAT:
    case cmSystemTools::UNKNOWN_FILE_FORMAT:
      cmSystemTools::Error("Unexpected file type ",
                           sourceFile.c_str());
      break;
    } 
  flags += "$(INCLUDE_FLAGS) ";
  // expand multi-command semi-colon separated lists
  // of commands into separate commands
  std::vector<std::string> commands;
  cmSystemTools::ExpandList(rules, commands);
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i,
                              0, // no objects
                              0, // no target
                              0, // no link libs
                              sourceFile.c_str(),
                              objectFile.c_str(),
                              flags.c_str());
    }
  
  std::vector<std::string> sourceAndDeps;
  sourceAndDeps.push_back(sourceFile);
  
  // Check for extra object-file dependencies.
  std::vector<std::string> depends;
  const char* additionalDeps = source.GetProperty("OBJECT_DEPENDS");
  if(additionalDeps)
    {
    cmSystemTools::ExpandListArgument(additionalDeps, depends);
    for(std::vector<std::string>::iterator i = depends.begin();
        i != depends.end(); ++i)
      {
      sourceAndDeps.push_back(cmSystemTools::ConvertToOutputPath(i->c_str()));
      }
    }
  
  this->OutputMakeRule(fout,
                       comment.c_str(),
                       objectFile.c_str(),
                       sourceAndDeps,
                       commands);
}



void cmLocalUnixMakefileGenerator::OutputSourceObjectBuildRules(std::ostream& fout)
{
  fout << "# Rules to build " << this->GetOutputExtension("")
       << " files from their sources:\n";

  std::set<std::string> rules;
  
  // Iterate over every target.
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  for(std::map<cmStdString, cmTarget>::const_iterator target = targets.begin(); 
      target != targets.end(); ++target)
    {
    bool shared = ((target->second.GetType() == cmTarget::SHARED_LIBRARY) ||
                   (target->second.GetType() == cmTarget::MODULE_LIBRARY));
    std::string exportsDef = "";
    if(shared)
      {
      std::string export_symbol;
      if (const char* custom_export_name = target->second.GetProperty("DEFINE_SYMBOL"))
        {
        export_symbol = custom_export_name;
        }
      else
        {
        std::string in = target->first + "_EXPORTS";
        export_symbol = cmSystemTools::MakeCindentifier(in.c_str());
        }
    
      exportsDef = "-D"+ export_symbol;
      }
    // Iterate over every source for this target.
    const std::vector<cmSourceFile*>& sources = 
      target->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY") && 
         !(*source)->GetCustomCommand())
        {
        std::string shortName;
        std::string sourceName;
        // If the full path to the source file includes this
        // directory, we want to use the relative path for the
        // filename of the object file.  Otherwise, we will use just
        // the filename portion.
        if((cmSystemTools::GetFilenamePath(
              (*source)->GetFullPath()).find(
                m_Makefile->GetCurrentDirectory()) == 0)
           || (cmSystemTools::GetFilenamePath(
                 (*source)->GetFullPath()).find(
                   m_Makefile->GetCurrentOutputDirectory()) == 0))
          {
          sourceName = (*source)->GetSourceName()+"."+
            (*source)->GetSourceExtension();
          shortName = (*source)->GetSourceName();
          
          // The path may be relative.  See if a directory needs to be
          // created for the output file.  This is a ugly, and perhaps
          // should be moved elsewhere.
          std::string relPath =
            cmSystemTools::GetFilenamePath((*source)->GetSourceName());
          if(relPath != "")
            {
            std::string outPath = m_Makefile->GetCurrentOutputDirectory();
            outPath += "/"+relPath;
            cmSystemTools::MakeDirectory(outPath.c_str());
            }
          }
        else
          {
          sourceName = (*source)->GetFullPath();
          shortName = cmSystemTools::GetFilenameName((*source)->GetSourceName());
          }
        std::string shortNameWithExt = shortName +
          (*source)->GetSourceExtension();
        // Only output a rule for each .o once.
        std::string compileFlags = exportsDef;
        compileFlags += " ";
        if(rules.find(shortNameWithExt) == rules.end())
          {
          
          if((*source)->GetProperty("COMPILE_FLAGS"))
            {
            compileFlags += (*source)->GetProperty("COMPILE_FLAGS");
            compileFlags += " ";
            }
          this->OutputBuildObjectFromSource(fout,
                                            shortName.c_str(),
                                            *(*source),
                                            compileFlags.c_str(),
                                            shared);
          rules.insert(shortNameWithExt);
          }
        }
      }
    }
}


void cmLocalUnixMakefileGenerator::OutputMakeRule(std::ostream& fout, 
                                                  const char* comment,
                                                  const char* target,
                                                  const char* depends, 
                                                  const char* command,
                                                  const char* command2,
                                                  const char* command3,
                                                  const char* command4)
{
  std::vector<std::string> commands;
  if(command)
    {
    commands.push_back(command);
    }
  if(command2)
    {
    commands.push_back(command2);
    }
  if(command3)
    {
    commands.push_back(command3);
    }
  if(command4)
    {
    commands.push_back(command4);
    }
  this->OutputMakeRule(fout, comment, target, depends, commands);
}


void cmLocalUnixMakefileGenerator::OutputMakeRule(std::ostream& fout, 
                                                  const char* comment,
                                                  const char* target,
                                                  const char* depends, 
                                                  const std::vector<std::string>& commands)
{
  std::vector<std::string> depend;
  if(depends)
    {
    depend.push_back(depends);
    }
  this->OutputMakeRule(fout, comment, target, depend, commands);
}

void cmLocalUnixMakefileGenerator::OutputMakeRule(std::ostream& fout, 
                                                  const char* comment,
                                                  const char* target,
                                                  const std::vector<std::string>& depends,
                                                  const char* command)
{
  std::vector<std::string> commands;
  if(command)
    {
    commands.push_back(command);
    }
  this->OutputMakeRule(fout, comment, target, depends, commands);
}

void cmLocalUnixMakefileGenerator::OutputMakeRule(std::ostream& fout, 
                                                  const char* comment,
                                                  const char* target,
                                                  const std::vector<std::string>& depends,
                                                  const std::vector<std::string>& commands)
{
  if(!target)
    {
    cmSystemTools::Error("no target for OutputMakeRule");
    return;
    }
  
  std::string replace;
  if(comment)
    {
    replace = comment;
    m_Makefile->ExpandVariablesInString(replace);
    fout << "#---------------------------------------------------------\n";
    fout << "# " << replace;
    fout << "\n#\n";
    }
  fout << "\n";

  replace = target;
  m_Makefile->ExpandVariablesInString(replace);
  
  std::string tgt = cmSystemTools::ConvertToOutputPath(replace.c_str());
  if(depends.empty())
    {
    fout << tgt.c_str() << ":\n";
    }
  else
    {
    // Split dependencies into multiple rule lines.  This allows for
    // very long dependency lists.
    for(std::vector<std::string>::const_iterator dep = depends.begin();
        dep != depends.end(); ++dep)
      {
      replace = *dep;
      m_Makefile->ExpandVariablesInString(replace);
      fout << tgt.c_str() << ": " << replace.c_str() << "\n";
      }
    }
  
  int count = 0;
  for (std::vector<std::string>::const_iterator i = commands.begin();
       i != commands.end(); ++i) 
    {
    replace = *i;
    m_Makefile->ExpandVariablesInString(replace);
    if(count == 0 && replace.find_first_not_of(" \t\n\r") != std::string::npos && 
       replace[0] != '-' && replace.find("echo") != 0  
       && replace.find("$(MAKE)") != 0)
      {
      std::string echostring = "Building ";
      echostring += comment;
      echostring += " ";
      echostring += target;
      echostring += "...";
      this->OutputEcho(fout,echostring.c_str());
      }
    fout << "\t" << replace.c_str() << "\n";
    count++;
    }
  fout << "\n";
}

const char* cmLocalUnixMakefileGenerator::GetSafeDefinition(const char* def)
{
  const char* ret = m_Makefile->GetDefinition(def);
  if(!ret)
    {
    return "";
    }
  return ret;
}

std::string cmLocalUnixMakefileGenerator::LowerCasePath(const char* path)
{
#ifdef _WIN32
   return cmSystemTools::LowerCase(path);
#else
   return std::string(path);
#endif
}
  
std::string
cmLocalUnixMakefileGenerator::CreateMakeVariable(const char* sin, const char* s2in)
{
  std::string s = sin;
  std::string s2 = s2in;
  std::string unmodified = s;
  unmodified += s2;
  // if there is no restriction on the length of make variables
  // and there are no "." charactors in the string, then return the
  // unmodified combination.
  if(!m_MakefileVariableSize && unmodified.find('.') == s.npos)
    {
    return unmodified;
    }
  
  // see if the variable has been defined before and return 
  // the modified version of the variable
  std::map<cmStdString, cmStdString>::iterator i = m_MakeVariableMap.find(unmodified);
  if(i != m_MakeVariableMap.end())
    {
    return i->second;
    }
  // start with the unmodified variable
  std::string ret = unmodified;
  // if this there is no value for m_MakefileVariableSize then
  // the string must have bad characters in it
  if(!m_MakefileVariableSize)
    {
    cmSystemTools::ReplaceString(ret, ".", "_");
    int ni = 0;
    char buffer[5];
    // make sure the _ version is not already used, if
    // it is used then add number to the end of the variable
    while(m_ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = unmodified + buffer;
      }
    m_ShortMakeVariableMap[ret] = "1";
    m_MakeVariableMap[unmodified] = ret;
    return ret;
    }
  
  // if the string is greater the 32 chars it is an invalid vairable name
  // for borland make
  if(static_cast<int>(ret.size()) > m_MakefileVariableSize)
    {
    int keep = m_MakefileVariableSize - 8;
    int size = keep + 3;
    std::string str1 = s;
    std::string str2 = s2;
    // we must shorten the combined string by 4 charactors
    // keep no more than 24 charactors from the second string
    if(static_cast<int>(str2.size()) > keep)
      {
      str2 = str2.substr(0, keep);
      }
    if(static_cast<int>(str1.size()) + static_cast<int>(str2.size()) > size)
      {
      str1 = str1.substr(0, size - str2.size());
      }
    char buffer[5];
    int ni = 0;
    sprintf(buffer, "%04d", ni);
    ret = str1 + str2 + buffer;
    while(m_ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = str1 + str2 + buffer;
      }
    if(ni == 1000)
      {
      cmSystemTools::Error("Borland makefile variable length too long");
      return unmodified;
      }
    // once an unused variable is found 
    m_ShortMakeVariableMap[ret] = "1";
    }
  // always make an entry into the unmodified to variable map
  m_MakeVariableMap[unmodified] = ret;
  return ret;

}
