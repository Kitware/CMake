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
#include "cmGlobalGenerator.h"
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"

cmLocalUnixMakefileGenerator::cmLocalUnixMakefileGenerator()
  :m_SharedLibraryExtension("$(SHLIB_SUFFIX)"),
   m_ObjectFileExtension(".o"),
   m_ExecutableExtension(cmSystemTools::GetExecutableExtension()),
   m_StaticLibraryExtension(".a"),
   m_LibraryPrefix("lib")
{
}

cmLocalUnixMakefileGenerator::~cmLocalUnixMakefileGenerator()
{
}


void cmLocalUnixMakefileGenerator::Generate(bool fromTheTop)
{
  // for backwards compatibility if niether c or cxx is
  // enabled, the enable cxx
  if(! (m_GlobalGenerator->GetLanguageEnabled("C") || 
        m_GlobalGenerator->GetLanguageEnabled("CXX")))
    {
    m_GlobalGenerator->EnableLanguage("CXX",m_Makefile);
    }
  

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

  this->m_Makefile->GenerateMakefile();
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
          for(cmDependInformation::DependencySet::const_iterator d = 
                info->m_DependencySet.begin();
              d != info->m_DependencySet.end(); ++d)
            {
            // Make sure the full path is given.  If not, the dependency was
            // not found.
            if((*d)->m_FullPath != "")
              {
              (*i)->GetDepends().push_back((*d)->m_FullPath);
              }
            }
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
  fout << "# Suppresses display of executed commands\n";
  fout << ".SILENT:\n";
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
  // Set up the default target as the VERY first target, so that make with no arguments will run it
  this->
    OutputMakeRule(fout, 
                   "Default target executed when no arguments are given to make, first make sure cmake.depends exists, cmake.check_depends is up-to-date, check the sources, then build the all target",
                   "default_target",
                   0,
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
    this->OutputIncludeMakefile(fout, "cmake.depends");
    }
}

void cmLocalUnixMakefileGenerator::OutputIncludeMakefile(std::ostream& fout,
                                                    const char* file)
{
  fout << "include " << file << "\n";
}


std::string 
cmLocalUnixMakefileGenerator::GetOutputExtension(const char*)
{
  return m_ObjectFileExtension;
}



// Output the rules for any targets
void cmLocalUnixMakefileGenerator::OutputTargetRules(std::ostream& fout)
{
  // for each target add to the list of targets
  fout << "TARGETS = ";
  const cmTargets &tgts = m_Makefile->GetTargets();
  // list libraries first
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.IsInAll())
      {
      std::string path = m_LibraryOutputPath + m_LibraryPrefix;
      if(l->second.GetType() == cmTarget::STATIC_LIBRARY)
        {
        path = path + l->first + m_StaticLibraryExtension;
        fout << " \\\n" 
             << cmSystemTools::ConvertToOutputPath(path.c_str());
        }
      else if(l->second.GetType() == cmTarget::SHARED_LIBRARY)
        {
        path = path + l->first + 
          m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        fout << " \\\n" 
             << cmSystemTools::ConvertToOutputPath(path.c_str());
        }
      else if(l->second.GetType() == cmTarget::MODULE_LIBRARY)
        {
        path = path + l->first + 
          m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
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
      std::string path = m_ExecutableOutputPath + l->first +
        m_ExecutableExtension;
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
        if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
          {
          std::string outExt(this->GetOutputExtension((*i)->GetSourceExtension().c_str()));
          if(outExt.size())
            {
            fout << "\\\n" << cmSystemTools::ConvertToOutputPath((*i)->GetSourceName().c_str())
                 << outExt.c_str() << " ";
            }
          }
        }
      fout << "\n\n";
      fout << this->CreateMakeVariable(l->first.c_str(), "_SRC_OBJS_QUOTED") << " = ";
      for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
          i != classes.end(); i++)
        {
        if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
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

  bool cxx = tgt.HasCxx(); 
  if(!cxx )
    {
    // if linking a c executable use the C runtime flag as cc
    // may not be the same program that creates shared libaries
    // and may have different flags
    if( tgt.GetType() == cmTarget::EXECUTABLE)
      {
      if(m_Makefile->GetDefinition("CMAKE_C_SHLIB_RUNTIME_FLAG"))
        {
        runtimeFlag = m_Makefile->GetDefinition("CMAKE_C_SHLIB_RUNTIME_FLAG");
        }
      }
    else
      {
      if(m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_FLAG"))
        {
        runtimeFlag = m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_FLAG");
        }
      }
    if(m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_SEP"))
      {
      runtimeSep = m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_SEP");
      }    
    }
  else
    {
    if(m_Makefile->GetDefinition("CMAKE_CXX_SHLIB_RUNTIME_FLAG"))
      {
      runtimeFlag = m_Makefile->GetDefinition("CMAKE_CXX_SHLIB_RUNTIME_FLAG");
      }
  
    if(m_Makefile->GetDefinition("CMAKE_CXX_SHLIB_RUNTIME_SEP"))
      {
      runtimeSep = m_Makefile->GetDefinition("CMAKE_CXX_SHLIB_RUNTIME_SEP");
      }    
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

  // collect all the flags needed for linking libraries
  std::string linkLibs;
  const std::vector<std::string>& libdirs = tgt.GetLinkDirectories();
  for(std::vector<std::string>::const_iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    { 
    std::string libpath = cmSystemTools::ConvertToOutputPath(libDir->c_str());
    if(emitted.insert(libpath).second)
      {
      std::string::size_type pos = libDir->find("-L");
      if((pos == std::string::npos || pos > 0)
         && libDir->find("${") == std::string::npos)
        {
        linkLibs += "-L";
        if(outputRuntime)
          {
          runtimeDirs.push_back( libpath );
          }
        }
      linkLibs += libpath;
      linkLibs += " ";
      }
    }

  std::string librariesLinked;
  const cmTarget::LinkLibraries& libs = tgt.GetLinkLibraries();
  for(cmTarget::LinkLibraries::const_iterator lib = libs.begin();
      lib != libs.end(); ++lib)
    {
    // Don't link the library against itself!
    if(targetLibrary && (lib->first == targetLibrary)) continue;
    // don't look at debug libraries
    if (lib->second == cmTarget::DEBUG) continue;
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() == 0) continue;
    // if it is a full path break it into -L and -l
    cmRegularExpression reg("([ \t]*\\-l)|([ \t]*\\-framework)|(\\${)");
    if(lib->first.find('/') != std::string::npos
       && !reg.find(lib->first))
      {
      std::string dir, file;
      cmSystemTools::SplitProgramPath(lib->first.c_str(),
                                      dir, file);
      std::string libpath = cmSystemTools::ConvertToOutputPath(dir.c_str());
      if(emitted.insert(libpath).second)
        {
        linkLibs += "-L";
        linkLibs += libpath;
        linkLibs += " ";
        if(outputRuntime)
          {
          runtimeDirs.push_back( libpath );
          }
        }
      cmRegularExpression libname("lib(.*)(\\.so|\\.sl|\\.a|\\.dylib).*");
      cmRegularExpression libname_noprefix("(.*)(\\.so|\\.sl|\\.a|\\.dylib).*");
      if(libname.find(file))
        {
        librariesLinked += "-l";
        file = libname.match(1);
	librariesLinked += file;
        librariesLinked += " ";
        }
      else if(libname_noprefix.find(file))
        {
        librariesLinked += "-l";
        file = libname_noprefix.match(1);
	librariesLinked += file;
        librariesLinked += " ";
        }
      }
    // not a full path, so add -l name
    else
      {
      if(!reg.find(lib->first))
        {
        librariesLinked += "-l";
        }
      librariesLinked += lib->first;
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
}


std::string cmLocalUnixMakefileGenerator::CreateTargetRules(const cmTarget &target,
                                                       const char* targetName)
{
  std::string customRuleCode = "";
  bool initNext = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetCustomCommands().begin(); 
       cr != target.GetCustomCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if (cc.GetSourceName() == targetName)
      {
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
    }
  return customRuleCode;
}


void cmLocalUnixMakefileGenerator::OutputSharedLibraryRule(std::ostream& fout,  
                                                      const char* name, 
                                                      const cmTarget &t)
{
  std::string target = m_LibraryOutputPath + "lib" + name + "$(SHLIB_SUFFIX)";
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS");
  depend += ") $(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string command = "$(RM) lib";
  command += name;
  command += "$(SHLIB_SUFFIX)";
  std::string command2;
  if(t.HasCxx())
    {
    command2 = "$(CMAKE_CXX_LINK_SHARED)  $(CMAKE_CXX_SHLIB_LINK_FLAGS) "
      "$(CMAKE_CXX_SHLIB_BUILD_FLAGS) $(CMAKE_CXX_FLAGS) -o \\\n";
    }
  else
    {
    command2 = "$(CMAKE_C_LINK_SHARED)  $(CMAKE_SHLIB_LINK_FLAGS) "
      "$(CMAKE_SHLIB_BUILD_FLAGS)  -o \\\n";
    }
  command2 += "\t  ";
  std::string libName = m_LibraryOutputPath + "lib" + std::string(name) + "$(SHLIB_SUFFIX)";
  libName = cmSystemTools::ConvertToOutputPath(libName.c_str());
  command2 += libName + " \\\n";
  command2 += "\t  $(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  cmStringStream linklibs;
  this->OutputLinkLibraries(linklibs, name, t);
  command2 += linklibs.str();
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout, "rules for a shared library",
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(),
                       command2.c_str(),
                       cc);
}

void cmLocalUnixMakefileGenerator::OutputModuleLibraryRule(std::ostream& fout, 
                                                      const char* name, 
                                                      const cmTarget &t)
{
  std::string target = m_LibraryOutputPath + "lib" + std::string(name) + "$(MODULE_SUFFIX)";
  std::string depend =  "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") 
    + ") $(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string command = "$(RM) lib" + std::string(name) + "$(MODULE_SUFFIX)";
  std::string command2;
  if(t.HasCxx())
    {
    command2 = "$(CMAKE_CXX_LINK_SHARED)  $(CMAKE_CXX_MODULE_LINK_FLAGS) "
      "$(CMAKE_CXX_MODULE_BUILD_FLAGS) $(CMAKE_CXX_FLAGS) -o \\\n";
    }
  else
    {
    command2 = "$(CMAKE_C_LINK_SHARED)  $(CMAKE_SHLIB_LINK_FLAGS) "
      "$(CMAKE_SHLIB_BUILD_FLAGS) -o \\\n";
    }
  command2 += "\t  ";
  std::string libName = m_LibraryOutputPath + "lib" + std::string(name) + "$(MODULE_SUFFIX)";
  libName = cmSystemTools::ConvertToOutputPath(libName.c_str());
  command2 += libName + " \\\n";
  command2 += "\t  $(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  cmStringStream linklibs;
  this->OutputLinkLibraries(linklibs, std::string(name).c_str(), t);
  command2 += linklibs.str();
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout, "rules for a shared module library",
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(),
                       command2.c_str(),
                       cc); 
}


void cmLocalUnixMakefileGenerator::OutputStaticLibraryRule(std::ostream& fout,
                                                      const char* name, 
                                                      const cmTarget &t)
{
  std::string target = m_LibraryOutputPath + "lib" + std::string(name) + ".a";
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") + ")";
  std::string command;
  if(t.HasCxx())
    {
    command = "$(CMAKE_CXX_AR) $(CMAKE_CXX_AR_ARGS) ";
    }
  else
    {
    command = "$(CMAKE_AR) $(CMAKE_AR_ARGS) ";
    }
  command += target;
  command += " $(";
  command += this->CreateMakeVariable(name, "_SRC_OBJS") + ")";
  std::string command2 = "$(CMAKE_RANLIB) ";
  command2 += target;
  std::string comment = "rule to build static library: ";
  comment += name;
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout,
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(),
                       command2.c_str(),
                       cc);
}

void cmLocalUnixMakefileGenerator::OutputExecutableRule(std::ostream& fout,
                                                   const char* name,
                                                   const cmTarget &t)
{
  std::string target = m_ExecutableOutputPath + name + m_ExecutableExtension;
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") 
    + ") $(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string command;
  if(t.HasCxx())
    {
    command = 
      "$(CMAKE_CXX_COMPILER) $(CMAKE_CXX_SHLIB_LINK_FLAGS) $(CMAKE_CXX_FLAGS) ";
    }
  else
    {
    command = 
      "$(CMAKE_C_COMPILER) $(CMAKE_C_SHLIB_LINK_FLAGS) $(CMAKE_C_FLAGS) ";
    }
  command += "$(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  cmStringStream linklibs;
  this->OutputLinkLibraries(linklibs, 0, t);
  command += linklibs.str();
  std::string outputFile = m_ExecutableOutputPath + name;
  command += " -o " + cmSystemTools::ConvertToOutputPath(outputFile.c_str());
  std::string comment = "rule to build executable: ";
  comment += name;
  
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout, 
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(),
                       cc);
}



void cmLocalUnixMakefileGenerator::OutputUtilityRule(std::ostream& fout,
                                                const char* name,
                                                const cmTarget &t)
{
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  std::string comment = "Rule to build Utility ";
  comment += name;
  std::string depends;
  std::string replaceVars;
  const std::vector<cmCustomCommand> &ccs = t.GetCustomCommands();
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
        || (l->second.GetType() == cmTarget::EXECUTABLE)
        || (l->second.GetType() == cmTarget::WIN32_EXECUTABLE))
      {
      fout << this->CreateMakeVariable(l->first.c_str(), "_DEPEND_LIBS") << " = ";
      
      // A library should not depend on itself!
      emitted.insert(l->first);
      
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
        std::string library = m_LibraryPrefix;
        library += *lib;
        std::string libpath = cacheValue;
        if(libType && std::string(libType) == "SHARED")
          {
          library += m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
          }
        else if(libType && std::string(libType) == "MODULE")
          {
          library += m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
          }
        else if(libType && std::string(libType) == "STATIC")
          {
          library += m_StaticLibraryExtension;
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
        exepath += *lib;
        this->OutputBuildTargetInDir(fout,
                                      cacheValue,
                                      lib->c_str(),
                                      exepath.c_str(),
                                      m_Makefile->
                                     GetDefinition("EXECUTABLE_OUTPUT_PATH")
                                      );
        }
      }
    }
}

void cmLocalUnixMakefileGenerator::OutputBuildTargetInDir(std::ostream& fout,
                                                     const char* path,
                                                     const char* library,
                                                     const char* fullpath,
                                                     const char* outputPath)
{
  const char* makeTarget = library;
  if(outputPath && strcmp( outputPath, "" ) != 0)
    {
    makeTarget = fullpath;
    }
  fout << cmSystemTools::ConvertToOutputPath(fullpath)
       << ":\n\tcd " << cmSystemTools::ConvertToOutputPath(path)
       << "; $(MAKE) $(MAKESILENT) cmake.depends"
       << "; $(MAKE) $(MAKESILENT) cmake.check_depends"
       << "; $(MAKE) $(MAKESILENT) -f cmake.check_depends"
       << "; $(MAKE) $(MAKESILENT) "
       << cmSystemTools::ConvertToOutputPath(makeTarget) << "\n\n"; 
}


bool cmLocalUnixMakefileGenerator::SamePath(const char* path1, const char* path2)
{
  return strcmp(path1, path2) == 0;
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
        libpath += m_LibraryPrefix;
        }
      else
        {
        libpath = cacheValue;
        libpath += "/";
        libpath += m_LibraryPrefix;
        }
      }
    else
      {
      // library is in current Makefile so use lib as a prefix
      libpath = m_LibraryOutputPath;
      libpath += m_LibraryPrefix;
      }
    // add the library name
    libpath += name;
    // add the correct extension
    std::string ltname = name;
    ltname += "_LIBRARY_TYPE";
    const char* libType = m_Makefile->GetDefinition(ltname.c_str());
    if(libType && std::string(libType) == "SHARED")
      {
      libpath += m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
      }
    else if (libType && std::string(libType) == "MODULE")
      {
      libpath += m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
      }
    else if (libType && std::string(libType) == "STATIC")
      {
      libpath += m_StaticLibraryExtension;
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
    exepath += m_ExecutableExtension;
    fout << cmSystemTools::ConvertToOutputPath(exepath.c_str()) << " ";
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


void cmLocalUnixMakefileGenerator::BuildInSubDirectory(std::ostream& fout,
                                                  const char* dir,
                                                  const char* target1,
                                                  const char* target2,
                                                  bool silent)
{
  std::string directory = cmSystemTools::ConvertToOutputPath(dir);
  if(target1)
    {
    fout << "\t@if test ! -d " << directory 
         << "; then $(MAKE) rebuild_cache; fi\n";
    if (!silent) 
      {
      fout << "\techo " << directory << ": building " << target1 << "\n";
      }
    fout << "\t@cd " << directory
         << "; $(MAKE) " << target1 << "\n";
    }
  if(target2)
    {
    if (!silent) 
      {
      fout << "\techo " << directory << ": building " << target2 << "\n";
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
  unsigned int i;
  for(i =0; i < SubDirectories.size(); i++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << target << "_" << subdir.c_str();
    if(i == SubDirectories.size()-1)
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
  for(unsigned int i =0; i < SubDirectories.size(); i++)
    {
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << target << "_" << subdir.c_str() << ": " << depend;
    
    // Make each subdirectory depend on previous one.  This forces
    // parallel builds (make -j 2) to build in same order as a single
    // threaded build to avoid dependency problems.
    if(i > 0)
      {
      fout << " " << target << "_" << last.c_str();
      }
    
    fout << "\n";
    last = subdir;
    std::string dir = m_Makefile->GetCurrentOutputDirectory();
    dir += "/";
    dir += SubDirectories[i];
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
          fout << (*source)->GetSourceName() << m_ObjectFileExtension << " :";
          // Iterate through all the dependencies for this source.
          for(std::vector<std::string>::const_iterator dep =
                (*source)->GetDepends().begin();
              dep != (*source)->GetDepends().end(); ++dep)
            {
            fout << " \\\n" 
                 << cmSystemTools::ConvertToOutputPath(dep->c_str());
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
            std::string dependfile = 
              cmSystemTools::ConvertToOutputPath(cmSystemTools::CollapseFullPath(dep->c_str()).c_str());
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
  for(std::set<std::string>::iterator i = emitted.begin();
      i != emitted.end(); ++i)
    {
    fout << *i << ":\n"
         << "\t$(MAKE) $(MAKESILENT) dependlocal\n\n";
    }
}

// Output each custom rule in the following format:
// output: source depends...
//   (tab)   command...
void cmLocalUnixMakefileGenerator::OutputCustomRules(std::ostream& fout)
{
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();
  
  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator tgt = tgts.begin(); 
      tgt != tgts.end(); ++tgt)
    {
    // add any custom rules to the source groups
    for (std::vector<cmCustomCommand>::const_iterator cr = 
           tgt->second.GetCustomCommands().begin(); 
         cr != tgt->second.GetCustomCommands().end(); ++cr)
      {
      // if the source for the custom command is the same name
      // as the target, then to not create a rule in the makefile for
      // the custom command, as the command will be fired when the other target 
      // is built.  
      if ( cr->GetSourceName().compare(tgt->first) !=0)
	{
	cmSourceGroup& sourceGroup = 
	  m_Makefile->FindSourceGroup(cr->GetSourceName().c_str(),
				      sourceGroups);
	cmCustomCommand cc(*cr);
	cc.ExpandVariables(*m_Makefile);
	sourceGroup.AddCustomCommand(cc);
	}
      }
    }

  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg =
        sourceGroups.begin(); sg != sourceGroups.end(); ++sg)
    {
    const cmSourceGroup::BuildRules& buildRules = sg->GetBuildRules();
    if(buildRules.empty())
      { continue; }
    
    std::string name = sg->GetName();
    if(name != "")
      {
      fout << "# Start of source group \"" << name.c_str() << "\"\n";
      }
    
    // Loop through each source in the source group.
    for(cmSourceGroup::BuildRules::const_iterator cc =
          buildRules.begin(); cc != buildRules.end(); ++ cc)
      {
      std::string source = cc->first;
      const cmSourceGroup::Commands& commands = cc->second.m_Commands;
      // Loop through every command generating code from the current source.
      for(cmSourceGroup::Commands::const_iterator c = commands.begin();
          c != commands.end(); ++c)
        {
        // escape spaces and convert to native slashes path for
        // the command
        std::string command = 
          cmSystemTools::ConvertToOutputPath(c->second.m_Command.c_str());
        command += " ";
        // now add the arguments
        command += c->second.m_Arguments;
        const cmSourceGroup::CommandFiles& commandFiles = c->second;
        // if the command has no outputs, then it is a utility command
        // with no outputs
        if(commandFiles.m_Outputs.size() == 0)
          {
          std::string depends;
          // collect out all the dependencies for this rule.
          for(std::set<std::string>::const_iterator d =
                commandFiles.m_Depends.begin();
              d != commandFiles.m_Depends.end(); ++d)
            {
            std::string dep = cmSystemTools::ConvertToOutputPath(d->c_str());
            depends +=  " ";
            depends += dep;
            }
          // output rule
          this->OutputMakeRule(fout,
                               "Custom command",
                               source.c_str(),
                               depends.c_str(),
                               command.c_str());
          }
        // Write a rule for every output generated by this command.
        for(std::set<std::string>::const_iterator output =
              commandFiles.m_Outputs.begin();
            output != commandFiles.m_Outputs.end(); ++output)
          {
          std::string src = cmSystemTools::ConvertToOutputPath(source.c_str());
          std::string depends;
          depends +=  src;
          // Collect out all the dependencies for this rule.
          for(std::set<std::string>::const_iterator d =
                commandFiles.m_Depends.begin();
              d != commandFiles.m_Depends.end(); ++d)
            {
            std::string dep = cmSystemTools::ConvertToOutputPath(d->c_str());
            depends += " ";
            depends += dep;
            } 
          // output rule
          this->OutputMakeRule(fout,
                               "Custom command",
                               output->c_str(),
                               depends.c_str(),
                               command.c_str());
          }
        }
      }
    if(name != "")
      {
      fout << "# End of source group \"" << name.c_str() << "\"\n\n";
      }
    }  
}

void cmLocalUnixMakefileGenerator::OutputMakeVariables(std::ostream& fout)
{
  const char* variables = 
    "# the standard shell for make\n"
    "SHELL = /bin/sh\n"
    "\n"
    "CMAKE_RANLIB        = @CMAKE_RANLIB@\n"
    "CMAKE_AR            = @CMAKE_AR@\n"
    "CMAKE_AR_ARGS       = @CMAKE_AR_ARGS@\n"
    "CMAKE_CXX_AR            = @CMAKE_CXX_AR@\n"
    "CMAKE_CXX_AR_ARGS       = @CMAKE_CXX_AR_ARGS@\n"
    "CMAKE_C_FLAGS    =    @CMAKE_C_FLAGS@\n"
    "CMAKE_C_COMPILER    = @CMAKE_C_COMPILER@\n"
    "CMAKE_C_LINK_SHARED    = @CMAKE_C_LINK_SHARED@\n"
    "CMAKE_CXX_LINK_SHARED       = @CMAKE_CXX_LINK_SHARED@\n"
    "CMAKE_SHLIB_CFLAGS  = @CMAKE_SHLIB_CFLAGS@\n"
    
    "CMAKE_CXX_SHLIB_CFLAGS = @CMAKE_CXX_SHLIB_CFLAGS@\n"
    "CMAKE_CXX_SHLIB_BUILD_FLAGS = @CMAKE_CXX_SHLIB_BUILD_FLAGS@\n"
    "CMAKE_CXX_SHLIB_LINK_FLAGS = @CMAKE_CXX_SHLIB_LINK_FLAGS@\n"
    "CMAKE_CXX_MODULE_BUILD_FLAGS = @CMAKE_CXX_MODULE_BUILD_FLAGS@\n"
    "CMAKE_CXX_MODULE_LINK_FLAGS = @CMAKE_CXX_MODULE_LINK_FLAGS@\n"
    "CMAKE_CXX_SHLIB_RUNTIME_FLAG = @CMAKE_CXX_SHLIB_RUNTIME_FLAG@\n"
    "CMAKE_CXX_SHLIB_RUNTIME_SEP = @CMAKE_CXX_SHLIB_RUNTIME_SEP@\n"

    "\n"
    "CMAKE_CXX_COMPILER  = @CMAKE_CXX_COMPILER@\n"
    "CMAKE_CXX_FLAGS     = @CMAKE_CXX_FLAGS@\n"
    "\n"
    "CMAKE_SHLIB_BUILD_FLAGS  = @CMAKE_SHLIB_BUILD_FLAGS@\n"
    "CMAKE_SHLIB_LINK_FLAGS   = @CMAKE_SHLIB_LINK_FLAGS@\n"
    "CMAKE_C_SHLIB_LINK_FLAGS   = @CMAKE_C_SHLIB_LINK_FLAGS@\n"
    "CMAKE_MODULE_BUILD_FLAGS = @CMAKE_MODULE_BUILD_FLAGS@\n"
    "CMAKE_MODULE_LINK_FLAGS  = @CMAKE_MODULE_LINK_FLAGS@\n"
    "CMAKE_C_SHLIB_RUNTIME_FLAG = @CMAKE_C_SHLIB_RUNTIME_FLAG@\n"
    "CMAKE_SHLIB_RUNTIME_FLAG = @CMAKE_SHLIB_RUNTIME_FLAG@\n"
    "CMAKE_SHLIB_RUNTIME_SEP = @CMAKE_SHLIB_RUNTIME_SEP@\n"
    "DL_LIBS                  = @CMAKE_DL_LIBS@\n"
    "SHLIB_LD_LIBS            = @CMAKE_SHLIB_LD_LIBS@\n"
    "SHLIB_SUFFIX             = @CMAKE_SHLIB_SUFFIX@\n"
    "MODULE_SUFFIX            = @CMAKE_MODULE_SUFFIX@\n"
    "THREAD_LIBS              = @CMAKE_THREAD_LIBS@\n"
    "RM = rm -f\n"
    "\n";
  std::string replaceVars = variables;
  m_Makefile->ExpandVariablesInString(replaceVars);
  fout << replaceVars.c_str();
  fout << "CMAKE_COMMAND = "
       << cmSystemTools::ConvertToOutputPath(m_Makefile->GetDefinition("CMAKE_COMMAND"))
       << "\n";
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    fout << "CMAKE_EDIT_COMMAND = "
         << cmSystemTools::ConvertToOutputPath(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
         << "\n";
    }

  fout << "CMAKE_CURRENT_SOURCE = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartDirectory()) << "\n";
  fout << "CMAKE_CURRENT_BINARY = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartOutputDirectory()) << "\n";
  fout << "CMAKE_SOURCE_DIR = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeDirectory()) << "\n";
  fout << "CMAKE_BINARY_DIR = " << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeOutputDirectory()) << "\n";
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  fout << "-I" << 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartDirectory()) << " ";
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    // Don't output a -I for the standard include path "/usr/include".
    // This can cause problems with certain standard library
    // implementations because the wrong headers may be found first.
    if(include != "/usr/include")
      {
      fout << "-I" << cmSystemTools::ConvertToOutputPath(i->c_str()) << " ";
      }
    }
  fout << m_Makefile->GetDefineFlags();
  fout << "\n\n";
}


void cmLocalUnixMakefileGenerator::OutputInstallRules(std::ostream& fout)
{
  const char* root
    = m_Makefile->GetDefinition("CMAKE_ROOT");
  fout << "INSTALL = " << root << "/Templates/install-sh -c\n";
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
      fout << "\t@if [ ! -d $(DESTDIR)" << prefix << l->second.GetInstallPath() << 
	" ] ; then \\\n";
      fout << "\t   echo \"Making directory $(DESTDIR)" << prefix 
	   << l->second.GetInstallPath() << " \"; \\\n";
      fout << "\t   mkdir -p $(DESTDIR)" << prefix << l->second.GetInstallPath() 
	   << "; \\\n";
      fout << "\t   chmod 755 $(DESTDIR)" <<  prefix << l->second.GetInstallPath() 
	   << "; \\\n";
      fout << "\t else true; \\\n";
      fout << "\t fi\n";
      // now install the target
      switch (l->second.GetType())
	{
	case cmTarget::STATIC_LIBRARY:
	  fout << "\t$(INSTALL_DATA) " << m_LibraryOutputPath << "lib" 
               << l->first;
          fout << ".a";
	  fout << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::SHARED_LIBRARY:
	  fout << "\t$(INSTALL_DATA) " << m_LibraryOutputPath << "lib" 
               << l->first;
          fout << m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
	  fout << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::MODULE_LIBRARY:
	  fout << "\t$(INSTALL_DATA) " << m_LibraryOutputPath << "lib" 
               << l->first;
          fout << m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
	  fout << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::WIN32_EXECUTABLE:
	case cmTarget::EXECUTABLE:
          fout << "\t$(INSTALL_PROGRAM) " << m_ExecutableOutputPath 
               << l->first
               << cmSystemTools::GetExecutableExtension()
	       << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::INSTALL_FILES:
	  {
	  const std::vector<std::string> &sf = l->second.GetSourceLists();
	  std::vector<std::string>::const_iterator i;
	  for (i = sf.begin(); i != sf.end(); ++i)
	    {
	    fout << "\t@ echo \"Installing " << *i << " \"\n"; 
	    fout << "\t@if [ -f " << *i << " ] ; then \\\n";
            // avoid using install-sh to install install-sh
            // does not work on windows.... 
           if(*i == "install-sh")
              {
              fout << "\t   cp ";
              }
            else
              {
              fout << "\t   $(INSTALL_DATA) ";
              }
	    fout << *i
		 << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\t elif [ -f $(CMAKE_CURRENT_SOURCE)/" << *i << " ] ; then \\\n";
            // avoid using install-sh to install install-sh
            // does not work on windows....
            if(*i == "install-sh")
              {
              fout << "\t   cp ";
              }
            else
              {
              fout << "\t   $(INSTALL_DATA) ";
              }
	    fout << "$(CMAKE_CURRENT_SOURCE)/" << *i 
		 << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\telse \\\n";
	    fout << "\t   echo \" ERROR!!! Unable to find: " << *i 
		 << " \"; \\\n";	 
	    fout << "\t fi\n";
	    }
	  }
	  break;
	case cmTarget::INSTALL_PROGRAMS:
	  {
	  const std::vector<std::string> &sf = l->second.GetSourceLists();
	  std::vector<std::string>::const_iterator i;
	  for (i = sf.begin(); i != sf.end(); ++i)
	    {
	    fout << "\t@ echo \"Installing " << *i << " \"\n"; 
	    fout << "\t@if [ -f " << *i << " ] ; then \\\n";
            // avoid using install-sh to install install-sh
            // does not work on windows.... 
           if(*i == "install-sh")
              {
              fout << "\t   cp ";
              }
            else
              {
              fout << "\t   $(INSTALL_PROGRAM) ";
              }
	    fout << *i
		 << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\t elif [ -f $(CMAKE_CURRENT_SOURCE)/" << *i << " ] ; then \\\n";
            // avoid using install-sh to install install-sh
            // does not work on windows....
            if(*i == "install-sh")
              {
              fout << "\t   cp ";
              }
            else
              {
              fout << "\t   $(INSTALL_PROGRAM) ";
              }
	    fout << "$(CMAKE_CURRENT_SOURCE)/" << *i 
		 << " $(DESTDIR)" << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\telse \\\n";
	    fout << "\t   echo \" ERROR!!! Unable to find: " << *i 
		 << " \"; \\\n";	 
	    fout << "\t fi\n";
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
                       "Default build rule",
                       "all",
                       "cmake.depends $(TARGETS) $(SUBDIR_BUILD)",
                       0);
  this->OutputMakeRule(fout, 
                       "remove generated files",
                       "clean",
                       "$(SUBDIR_CLEAN)",
                       "-@ $(RM) $(CLEAN_OBJECT_FILES) "
                       " $(TARGETS) $(GENERATED_QT_FILES) $(GENERATED_FLTK_FILES)");

  // collect up all the sources
  std::string allsources;
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
          allsources += " \\\n";
          allsources += cmSystemTools::ConvertToOutputPath((*source)->GetFullPath().c_str());
        }
      }
    }

  this->OutputMakeRule(fout, 
                       "Rule to build the cmake.depends and Makefile as side effect, if a source cmakelist file is out of date.",
                       "cmake.depends",
                       "$(CMAKE_MAKEFILE_SOURCES)",
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)"
    );
  this->OutputMakeRule(fout, 
                       "Rule to build the cmake.check_depends and Makefile as side effect, if any source file has changed.",
                       "cmake.check_depends",
                       allsources.c_str(),
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)"
    );
  
  this->OutputMakeRule(fout, 
                       "Rule to force the build of cmake.depends",
                       "depend",
                       "$(SUBDIR_DEPEND)",
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");  
  this->OutputMakeRule(fout, 
                       "Rule to force the build of cmake.depends "
                       "in the current directory only.",
                       "dependlocal",
                       0,
                       "$(CMAKE_COMMAND) "
                       "-S$(CMAKE_CURRENT_SOURCE) -O$(CMAKE_CURRENT_BINARY) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");  

  this->OutputMakeRule(fout, 
                       "Rebuild CMakeCache.txt file",
                       "rebuild_cache",
                       "$(CMAKE_BINARY_DIR)/CMakeCache.txt",
                       "$(CMAKE_COMMAND) "
                       "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
  // if CMAKE_EDIT_COMMAND is defined then add a rule to run it
  // called edit_cache
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    this->OutputMakeRule(fout, 
                         "Edit the CMakeCache.txt file with ccmake or CMakeSetup",
                         "edit_cache",
                         0,
                         "$(CMAKE_EDIT_COMMAND) "
                         "-H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
    }
  
  this->OutputMakeRule(fout, 
                       "Create CMakeCache.txt file",
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
  if (cmSystemTools::FileExists(ctest.c_str()))
    {
    this->OutputMakeRule(fout, 
                         "run any tests",
                         "test",
                         "",
                         cmSystemTools::ConvertToOutputPath(ctest.c_str()).c_str());
    }
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

  std::string comment = "Build ";
  std::string objectFile = std::string(shortName) + m_ObjectFileExtension;
  objectFile = cmSystemTools::ConvertToOutputPath(objectFile.c_str());
  comment += objectFile + "  From ";
  comment += source.GetFullPath();
  std::string compileCommand;
  std::string ext = source.GetSourceExtension();
  if(ext == "c" )
    {
    compileCommand = "$(CMAKE_C_COMPILER) $(CMAKE_C_FLAGS) ";
    compileCommand += extraCompileFlags;
    if(shared)
      {
      compileCommand += "$(CMAKE_SHLIB_CFLAGS) ";
      }
    compileCommand += "$(INCLUDE_FLAGS) -c ";
    compileCommand += 
      cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str());
    compileCommand += " -o ";
    compileCommand += objectFile;
    }
  else
    {
    compileCommand = "$(CMAKE_CXX_COMPILER) $(CMAKE_CXX_FLAGS) ";
    compileCommand += extraCompileFlags;
    if(shared)
      {
      compileCommand += "$(CMAKE_SHLIB_CFLAGS) ";
      }
    compileCommand += "$(INCLUDE_FLAGS) -c ";
    compileCommand += 
      cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str());
    compileCommand += " -o ";
    compileCommand += objectFile;
    }
  this->OutputMakeRule(fout,
                       comment.c_str(),
                       objectFile.c_str(),
                       cmSystemTools::ConvertToOutputPath(source.GetFullPath().
                                                 c_str()).c_str(),
                       compileCommand.c_str());
}



void cmLocalUnixMakefileGenerator::OutputSourceObjectBuildRules(std::ostream& fout)
{
  fout << "# Rules to build " << m_ObjectFileExtension 
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
      exportsDef = "-D"+target->first+"_EXPORTS ";
      }
    // Iterate over every source for this target.
    const std::vector<cmSourceFile*>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        std::string shortName;
        std::string sourceName;
        // If the full path to the source file includes this
        // directory, we want to use the relative path for the
        // filename of the object file.  Otherwise, we will use just
        // the filename portion.
        if((cmSystemTools::GetFilenamePath((*source)->GetFullPath()).find(m_Makefile->GetCurrentDirectory()) == 0)
           || (cmSystemTools::GetFilenamePath((*source)->GetFullPath()).find(m_Makefile->
                                                                          GetCurrentOutputDirectory()) == 0))
          {
          sourceName = (*source)->GetSourceName()+"."+(*source)->GetSourceExtension();
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
        if(rules.find(shortNameWithExt) == rules.end())
          {
          if((*source)->GetProperty("COMPILE_FLAGS"))
            {
            exportsDef += (*source)->GetProperty("COMPILE_FLAGS");
            exportsDef += " ";
            }
          this->OutputBuildObjectFromSource(fout,
                                            shortName.c_str(),
                                            *(*source),
                                            exportsDef.c_str(),
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
  fout << cmSystemTools::ConvertToOutputPath(replace.c_str()) << ": ";

  if(depends)
    {
    replace = depends;
    m_Makefile->ExpandVariablesInString(replace);
    fout << replace.c_str();
    }
  fout << "\n";

  const char* commands[] = { command, command2, command3, command4 };

  for (unsigned int i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) 
    {
    if(commands[i])
      {
      replace = commands[i];
      m_Makefile->ExpandVariablesInString(replace);
      if(replace[0] != '-' && replace.find("echo") != 0  
         && replace.find("$(MAKE)") != 0)
        {
        std::string echostring = replace;
        // for unix we want to quote the output of echo
        // for nmake and borland, the echo should not be quoted
        if(strcmp(m_GlobalGenerator->GetName(), "Unix Makefiles") == 0)
          {
	  cmSystemTools::ReplaceString(echostring, "\\\n", " ");
          cmSystemTools::ReplaceString(echostring, " \t", "   ");
          cmSystemTools::ReplaceString(echostring, "\n\t", "\"\n\techo \"");
          fout << "\techo \"" << echostring.c_str() << "\"\n";
          }
        else
          {
	  cmSystemTools::ReplaceString(echostring, "\n\t", "\n\techo ");
          fout << "\techo " << echostring.c_str() << "\n";
          }
        }
      fout << "\t" << replace.c_str() << "\n";
      }
    }
  fout << "\n";
}

