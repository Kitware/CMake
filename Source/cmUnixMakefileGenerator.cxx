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
#include "cmUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"

cmUnixMakefileGenerator::cmUnixMakefileGenerator()
{
  m_CacheOnly = false;
  m_Recurse = false;
}

void cmUnixMakefileGenerator::GenerateMakefile()
{
  if(m_CacheOnly)
    {
    // Generate the cache only stuff
    this->GenerateCacheOnly();
    // if recurse then generate for all sub- makefiles
    if(m_Recurse)
      {
      this->RecursiveGenerateCacheOnly();
      }
    }
  // normal makefile output
  else
    {
    // Generate depends 
    cmMakeDepend md;
    md.SetMakefile(m_Makefile);
    md.DoDepends();
    // output the makefile fragment
    this->OutputMakefile("Makefile"); 
    }
}


// This is where CMakeTargets.make is generated
void cmUnixMakefileGenerator::OutputMakefile(const char* file)
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
      cmSystemTools::MakeDirectory(i->c_str());
      }
    }
  std::ofstream fout(file);
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", file);
    return;
    }
  fout << "# CMAKE generated Makefile, DO NOT EDIT!\n"
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
  // create a make variable with all of the sources for this Makefile
  // for depend purposes.
  fout << "CMAKE_MAKEFILE_SOURCES = ";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    fout << " " << i->c_str();
    }
  // Add the cache to the list
  fout << " " << m_Makefile->GetHomeOutputDirectory() << "/CMakeCache.txt\n";
  fout << "\n\n";
  this->OutputMakeVariables(fout);
  this->OutputMakeFlags(fout);
  this->OutputTargetRules(fout);
  this->OutputDependencies(fout);
  this->OutputTargets(fout);
  this->OutputSubDirectoryRules(fout);
  std::string dependName;
  if(!this->m_CacheOnly)
    {
    dependName = m_Makefile->GetStartOutputDirectory();
    dependName += "/cmake.depends";
    std::ofstream dependout(dependName.c_str());
    if(!dependout)
      {
       cmSystemTools::Error("Error can not open for write: ", dependName.c_str());
       return;
      }
    this->OutputObjectDepends(dependout);
    }
  this->OutputCustomRules(fout);
  this->OutputMakeRules(fout);
  // only add the depend include if the depend file exists
  if(cmSystemTools::FileExists(dependName.c_str()))
    {
    fout << "include cmake.depends\n";
    }
}


// Output the rules for any targets
void cmUnixMakefileGenerator::OutputTargetRules(std::ostream& fout)
{
  // for each target add to the list of targets
  fout << "TARGETS = ";
  const cmTargets &tgts = m_Makefile->GetTargets();
  // list libraries first
  bool dll = cmCacheManager::GetInstance()->IsOn("BUILD_SHARED_LIBS");
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::LIBRARY &&
	l->second.IsInAll())
      {
      fout << " \\\nlib" << l->first.c_str();
      if(dll)
        {
        fout << m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        }
      else
        {
        fout << ".a";
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
      fout << " \\\n" << l->first.c_str();
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
    std::vector<cmSourceFile> classes = l->second.GetSourceFiles();
    fout << l->first << "_SRC_OBJS = ";
    for(std::vector<cmSourceFile>::iterator i = classes.begin(); 
        i != classes.end(); i++)
      {
      if(!i->IsAHeaderFileOnly())
        {
        fout << "\\\n" << i->GetSourceName() << ".o ";
        }
      }
    fout << "\n\n";
    }
  fout << "CLEAN_OBJECT_FILES = ";
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    fout << "${" << l->first << "_SRC_OBJS} ";
    }
  fout << "\n";
}


/**
 * Output the linking rules on a command line.  For executables,
 * targetLibrary should be a NULL pointer.  For libraries, it should point
 * to the name of the library.  This will not link a library against itself.
 */
void cmUnixMakefileGenerator::OutputLinkLibraries(std::ostream& fout,
                                                  const char* targetLibrary,
                                                  const cmTarget &tgt)
{
  // collect all the flags needed for linking libraries
  std::string linkLibs;        
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(std::vector<std::string>::iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    { 
    std::string::size_type pos = libDir->find("-L");
    if((pos == std::string::npos || pos > 0)
       && libDir->find("${") == std::string::npos)
      {
      linkLibs += "-L";
      }
    linkLibs += cmSystemTools::EscapeSpaces(libDir->c_str());
    linkLibs += " ";
    }
  std::string librariesLinked;
  const cmTarget::LinkLibraries& libs = tgt.GetLinkLibraries();
  cmRegularExpression reg("lib(.*)(\\.so$|\\.a|\\.sl$)");
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
    cmRegularExpression reg("(^[ \t]*\\-l)|(\\${)");
    if(lib->first.find('/') != std::string::npos
       && !reg.find(lib->first))
      {
      std::string dir, file;
      cmSystemTools::SplitProgramPath(lib->first.c_str(),
                                      dir, file);
      linkLibs += "-L";
      linkLibs += cmSystemTools::EscapeSpaces(dir.c_str());
      linkLibs += " ";
      librariesLinked += "-l";
      if(reg.find(file))
        {
        file = reg.match(1);
        }
      librariesLinked += file;
      librariesLinked += " ";
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

  if(!targetLibrary)
    {
    // For executables, add these a second time so order does not matter
    linkLibs += librariesLinked;
    }
  fout << linkLibs;
}


void cmUnixMakefileGenerator::OutputTargets(std::ostream& fout)
{
  // for each target
  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::LIBRARY)
      {
      fout << "#---------------------------------------------------------\n";
      fout << "# rules for a library\n";
      fout << "#\n";
      fout << "lib" << l->first << ".a: ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${CMAKE_AR} cr lib" << l->first << ".a ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${CMAKE_RANLIB} lib" << l->first << ".a\n";
      fout << std::endl;

      fout << "lib" << l->first << "$(SHLIB_SUFFIX):  ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\trm -f lib" << l->first << "$(SHLIB_SUFFIX)\n";
      fout << "\t$(CMAKE_CXX_COMPILER) ${CMAKE_SHLIB_LINK_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_SHLIB_BUILD_FLAGS} -o \\\n";
      fout << "\t  lib" << l->first << "$(SHLIB_SUFFIX) \\\n";
      fout << "\t  ${" << l->first << 
        "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, l->first.c_str(), l->second);
      fout << "\n\n";
      }
    else if (l->second.GetType() == cmTarget::EXECUTABLE)
      {
      fout << l->first << ": ${" << 
        l->first << "_SRC_OBJS} ${CMAKE_DEPEND_LIBS}\n";
      fout << "\t${CMAKE_CXX_COMPILER}  ${CMAKE_CXXFLAGS} ${" << 
        l->first << "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, NULL,l->second);
      fout << " -o " << l->first << "\n\n";
      }
    }
}


// output the list of libraries that the executables 
// in this makefile will depend on.
void cmUnixMakefileGenerator::OutputDependencies(std::ostream& fout)
{
  fout << "CMAKE_DEPEND_LIBS = ";
  cmTarget::LinkLibraries& libs = m_Makefile->GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator lib2;
  // Search the list of libraries that will be linked into
  // the executable
  for(lib2 = libs.begin(); lib2 != libs.end(); ++lib2)
    {
    // loop over the list of directories that the libraries might
    // be in, looking for an ADD_LIBRARY(lib...) line. This would
    // be stored in the cache
    const char* cacheValue
      = cmCacheManager::GetInstance()->GetCacheValue(lib2->first.c_str());
    if(cacheValue)
      {
      std::string libpath = cacheValue;
      libpath += "/lib";
      libpath += lib2->first; 
      bool dll = cmCacheManager::GetInstance()->IsOn("BUILD_SHARED_LIBS");
      if(dll)
        {
        libpath += m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        }
      else
        {
        libpath += ".a";
        }
      fout << libpath << " ";
      }
    }
  fout << "\n\n";
  for(lib2 = libs.begin(); lib2 != libs.end(); ++lib2)
    {
    // loop over the list of directories that the libraries might
    // be in, looking for an ADD_LIBRARY(lib...) line. This would
    // be stored in the cache
    const char* cacheValue
      = cmCacheManager::GetInstance()->GetCacheValue(lib2->first.c_str());
    if(cacheValue)
      {
      std::string library = "lib";
      library += lib2->first; 
      bool dll = cmCacheManager::GetInstance()->IsOn("BUILD_SHARED_LIBS");
      if(dll)
        {
        library += m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        }
      else
        {
        library += ".a";
        }
      std::string libpath = cacheValue;
      libpath += "/";
      libpath += library;
      // put out a rule to build the library if it does not exist
      fout << libpath.c_str()
           << ":\n\tcd " << cacheValue 
           << "; make " << library.c_str() << "\n\n";
      }
    }

  std::vector<std::string>& utils = m_Makefile->GetUtilities();
  std::vector<std::string>& utildirs = m_Makefile->GetUtilityDirectories();
  std::vector<std::string>::iterator dir, util;
  // Search the list of utilities that may be used to generate code for
  // this project.
  for(util = utils.begin(); util != utils.end(); ++util)
    {
    bool found = false;
    // loop over the list of directories that the utilities might
    // be in, looking for an ADD_EXECUTABLE(util ...) line.
    for(dir = utildirs.begin(); dir != utildirs.end() && !found; ++dir)
      {
      std::string expression = "TARGETS =.*";
      expression += util->c_str();
      if(cmSystemTools::Grep(dir->c_str(), "Makefile",
                             expression.c_str()))
        {
        fout << *util << " ";
        found = true;
        }
      }
    }
  fout << "\n";
}


// output make include flags
void cmUnixMakefileGenerator::OutputMakeFlags(std::ostream& fout)
{
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  fout << "-I" << m_Makefile->GetStartDirectory() << " ";
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    fout << "-I" << cmSystemTools::EscapeSpaces(i->c_str()).c_str() << " ";
    }
  fout << m_Makefile->GetDefineFlags();
  fout << "\n\n";
  fout << "default_target: all\n\n";
  // see if there are files to compile in this makefile
  // These are used for both libraries and executables
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

void 
cmUnixMakefileGenerator::
OutputSubDirectoryVars(std::ostream& fout,
                       const char* var,
                       const char* target,
                       const char* target1,
                       const char* target2,
                       const std::vector<std::string>& SubDirectories)
{
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
  for(unsigned int i =0; i < SubDirectories.size(); i++)
    {
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << target << "_" << subdir.c_str() << ":\n";
    if(target1)
      {
	fout << "\tif test ! -d " << SubDirectories[i].c_str() << "; then ${MAKE} rebuild_cache; fi\n"
	  "\tcd " << SubDirectories[i].c_str()
           << "; ${MAKE} -${MAKEFLAGS} " << target1 << "\n";
      }
    if(target2)
      {
      fout << "\tcd " << SubDirectories[i].c_str()
           << "; ${MAKE} -${MAKEFLAGS} " << target2 << "\n";
      }
    }
  fout << "\n\n";
}


// output rules for decending into sub directories
void cmUnixMakefileGenerator::OutputSubDirectoryRules(std::ostream& fout)
{
    // Output Sub directory build rules
  const std::vector<std::string>& SubDirectories
    = m_Makefile->GetSubDirectories();
    
  if( SubDirectories.size() == 0)
    {
    return;
    }
  this->OutputSubDirectoryVars(fout, "SUBDIR_BUILD", "build",
                               "cmake.depends",
                               "all",
                               SubDirectories);
  this->OutputSubDirectoryVars(fout, "SUBDIR_CLEAN", "clean",
                               "clean",
                               0,
                               SubDirectories);
  this->OutputSubDirectoryVars(fout, "SUBDIR_DEPEND", "depend",
                               "depend",
                               0,
                               SubDirectories);
}




// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend GenerateMakefile
void cmUnixMakefileGenerator::OutputObjectDepends(std::ostream& fout)
{
  // Iterate over every target.
  std::map<std::string, cmTarget>& targets = m_Makefile->GetTargets();
  for(std::map<std::string, cmTarget>::const_iterator target = targets.begin(); 
      target != targets.end(); ++target)
    {
    // Iterate over every source for this target.
    const std::vector<cmSourceFile>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!source->IsAHeaderFileOnly())
        {
        if(!source->GetDepends().empty())
          {
          fout << source->GetSourceName() << ".o :";
          // Iterate through all the dependencies for this source.
          for(std::vector<std::string>::const_iterator dep =
                source->GetDepends().begin();
              dep != source->GetDepends().end(); ++dep)
            {
            fout << " \\\n" << dep->c_str();
            }
          fout << "\n\n";
          }
        }
      }
    }
}


// Output each custom rule in the following format:
// output: source depends...
//   (tab)   command...
void cmUnixMakefileGenerator::OutputCustomRules(std::ostream& fout)
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
      cmSourceGroup& sourceGroup = 
        m_Makefile->FindSourceGroup(cr->GetSourceName().c_str(),
                                    sourceGroups);
      cmCustomCommand cc(*cr);
      cc.ExpandVariables(*m_Makefile);
      sourceGroup.AddCustomCommand(cc);
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
      const cmSourceGroup::Commands& commands = cc->second;
      // Loop through every command generating code from the current source.
      for(cmSourceGroup::Commands::const_iterator c = commands.begin();
          c != commands.end(); ++c)
        {
        std::string command = c->first;
        const cmSourceGroup::CommandFiles& commandFiles = c->second;
        // if the command has no outputs, then it is a utility command
        // with no outputs
        if(commandFiles.m_Outputs.size() == 0)
          {
	    fout << source.c_str() << ": ";
	    // Write out all the dependencies for this rule.
	    for(std::set<std::string>::const_iterator d =
		  commandFiles.m_Depends.begin();
		d != commandFiles.m_Depends.end(); ++d)
	      {
		std::string dep = cmSystemTools::EscapeSpaces(d->c_str());
		fout << " " << dep.c_str();
	      }
	    fout << "\n\t" << command.c_str() << "\n\n";
          }
        // Write a rule for every output generated by this command.
        for(std::set<std::string>::const_iterator output =
              commandFiles.m_Outputs.begin();
            output != commandFiles.m_Outputs.end(); ++output)
          {
          std::string src = cmSystemTools::EscapeSpaces(source.c_str());
          fout << output->c_str() << ": " << src.c_str();
          // Write out all the dependencies for this rule.
          for(std::set<std::string>::const_iterator d =
                commandFiles.m_Depends.begin();
              d != commandFiles.m_Depends.end(); ++d)
            {
            std::string dep = cmSystemTools::EscapeSpaces(d->c_str());
            fout << " " << dep.c_str();
            }
          fout << "\n\t" << command.c_str() << "\n\n";
          }
        }
      }
    if(name != "")
      {
      fout << "# End of source group \"" << name.c_str() << "\"\n\n";
      }
    }  
}


void cmUnixMakefileGenerator::GenerateCacheOnly()
{
  cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory());
  std::string dest = m_Makefile->GetStartOutputDirectory();
  dest += "/Makefile";
  std::cout << "cmake: creating : " << dest.c_str() << std::endl;
  this->OutputMakefile(dest.c_str());
  return;
}

void cmUnixMakefileGenerator::RecursiveGenerateCacheOnly()
{ 
  std::vector<cmMakefile*> makefiles;
  m_Makefile->FindSubDirectoryCMakeListsFiles(makefiles);
  for(std::vector<cmMakefile*>::iterator i = makefiles.begin();
      i != makefiles.end(); ++i)
    {
    cmMakefile* mf = *i;
    if(m_Makefile->GetDefinition("RUN_CONFIGURE"))
      {
      mf->AddDefinition("RUN_CONFIGURE", true);
      }
    cmUnixMakefileGenerator* gen = new cmUnixMakefileGenerator;
    gen->SetCacheOnlyOn();
    gen->SetRecurseOff();
    mf->SetMakefileGenerator(gen);
    mf->GenerateMakefile();
    }
  // CLEAN up the makefiles created
  for(unsigned int i =0; i < makefiles.size(); ++i)
    {
      delete makefiles[i];
    }
}

void cmUnixMakefileGenerator::OutputMakeVariables(std::ostream& fout)
{
  if(strcmp(m_Makefile->GetHomeDirectory(), 
            m_Makefile->GetHomeOutputDirectory()) == 0)
    {
    fout << "srcdir        = .\n\n";
    }
  else
    {
    fout << "srcdir        = " <<  m_Makefile->GetStartDirectory() << "\n";
    fout << "VPATH         = " <<  m_Makefile->GetStartDirectory() << "\n";
    }
  const char* variables = 
    "# the standard shell for make\n"
    "SHELL = /bin/sh\n"
    "\n"
    "CMAKE_LIB_EXT       = @CMAKE_LIB_EXT@\n"
    "CMAKE_RANLIB        = @CMAKE_RANLIB@\n"
    "CMAKE_AR            = @CMAKE_AR@\n"
    "CMAKE_C_COMPILER    = @CMAKE_C_COMPILER@\n"
    "CMAKE_CFLAGS        = @CMAKE_C_FLAGS@ @CMAKE_SHLIB_CFLAGS@ \n"
    "\n"
    "CMAKE_CXX_COMPILER  = @CMAKE_CXX_COMPILER@\n"
    "CMAKE_CXXFLAGS      = @CMAKE_CXX_FLAGS@ @CMAKE_SHLIB_CFLAGS@ @CMAKE_TEMPLATE_FLAGS@ \n"
    "\n"
    "CMAKE_SHLIB_BUILD_FLAGS = @CMAKE_SHLIB_BUILD_FLAGS@\n"
    "CMAKE_SHLIB_LINK_FLAGS = @CMAKE_SHLIB_LINK_FLAGS@\n"
    "DL_LIBS              = @CMAKE_DL_LIBS@\n"
    "SHLIB_LD_LIBS        = @CMAKE_SHLIB_LD_LIBS@\n"
    "SHLIB_SUFFIX         = @CMAKE_SHLIB_SUFFIX@\n"
    "THREAD_LIBS          = @CMAKE_THREAD_LIBS@\n"
    "\n"
    "# set up the path to the rulesgen program\n"
    "CMAKE_COMMAND = ${CMAKE_COMMAND}\n"
    "\n"
    "\n"
    "\n";
  std::string replaceVars = variables;
  bool dll = cmCacheManager::GetInstance()->IsOn("BUILD_SHARED_LIBS");
  if(!dll)
    {
    // if not a dll then remove the shlib -fpic flag
    m_Makefile->AddDefinition("CMAKE_SHLIB_CFLAGS", "");
    }
  
  m_Makefile->ExpandVariablesInString(replaceVars);
  fout << replaceVars.c_str();
  fout << "CMAKE_CURRENT_SOURCE = " << m_Makefile->GetStartDirectory() << "\n";
  fout << "CMAKE_CURRENT_BINARY = " << m_Makefile->GetStartOutputDirectory() << "\n";
}


void cmUnixMakefileGenerator::OutputMakeRules(std::ostream& fout)
{
  this->OutputMakeRule(fout, 
                       "# tell make about .cxx and .java",
                       ".SUFFIXES", ".cxx .java .class", 0);
  this->OutputMakeRule(fout, 
                       "# build c file",
                       ".c.o", 
                       0,
                       "${CMAKE_C_COMPILER} ${CMAKE_CFLAGS} ${INCLUDE_FLAGS} -c $< -o $@");
  this->OutputMakeRule(fout, 
                       "# build cplusplus file",
                       ".cxx.o", 
                       0,
                       "${CMAKE_CXX_COMPILER} ${CMAKE_CXXFLAGS} ${INCLUDE_FLAGS} -c $< -o $@");  this->OutputMakeRule(fout, 
                       "Default build rule",
                       "all",
                       "Makefile cmake.depends ${TARGETS} ${SUBDIR_BUILD} ${CMAKE_COMMAND}",
                       0);
  this->OutputMakeRule(fout, 
                       "remove generated files",
                       "clean",
                       "${SUBDIR_CLEAN}",
                       "rm -f ${CLEAN_OBJECT_FILES} ${EXECUTABLES} ${TARGETS}");
  this->OutputMakeRule(fout, 
                       "Rule to build the Makefile",
                       "Makefile",
                       "${CMAKE_COMMAND} ${CMAKE_MAKEFILE_SOURCES} ",
                       "${CMAKE_COMMAND} "
                       "-S${CMAKE_CURRENT_SOURCE} -O${CMAKE_CURRENT_BINARY} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");  
  this->OutputMakeRule(fout, 
                       "Rule to build the cmake.depends",
                       "cmake.depends",
                       "${CMAKE_COMMAND} ${CMAKE_MAKEFILE_SOURCES} ",
                       "${CMAKE_COMMAND} "
                       "-S${CMAKE_CURRENT_SOURCE} -O${CMAKE_CURRENT_BINARY} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");
  this->OutputMakeRule(fout, 
                       "Rule to force the build of cmake.depends",
                       "depend",
                       "${SUBDIR_DEPEND}",
                       "${CMAKE_COMMAND} "
                       "-S${CMAKE_CURRENT_SOURCE} -O${CMAKE_CURRENT_BINARY} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");  
  this->OutputMakeRule(fout, 
                       "Rebuild the cache",
                       "rebuild_cache",
                       "${CMAKE_BINARY_DIR}/CMakeCache.txt",
                       "${CMAKE_COMMAND} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");
  this->OutputMakeRule(fout, 
                       "Rebuild cmake dummy rule",
                       "${CMAKE_COMMAND}",
                       0,
                       "echo \"cmake might be out of date\"");
  
}

void cmUnixMakefileGenerator::OutputMakeRule(std::ostream& fout, 
                                             const char* comment,
                                             const char* target,
                                             const char* depends, 
                                             const char* command)
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
    fout << "# " << comment;
    }
  fout << "\n";
  replace = target;
  m_Makefile->ExpandVariablesInString(replace);
  fout << replace.c_str() << ": ";
  if(depends)
    {
    replace = depends;
    m_Makefile->ExpandVariablesInString(replace);
    fout << replace.c_str();
    }
  fout << "\n";
  if(command)
    {
    replace = command;
    m_Makefile->ExpandVariablesInString(replace);
    fout << "\t" << replace.c_str() << "\n\n";
    }
  fout << "\n\n\n";

}


void cmUnixMakefileGenerator::SetLocal (bool local)
{
  if (local)
    {
    m_CacheOnly = false;
    m_Recurse = false;
    }
  else
    {
    m_CacheOnly = true;
    m_Recurse = true;
    }
}

void cmUnixMakefileGenerator::ComputeSystemInfo()
{
  if (m_CacheOnly)
    {
      // currently we run configure shell script here to determine the info
      std::string output;
      std::string cmd;
      const char* root
	= cmCacheManager::GetInstance()->GetCacheValue("CMAKE_ROOT");
      cmd = root;
      cmd += "/Templates/configure";
      cmSystemTools::RunCommand(cmd.c_str(), output);
    }

  // now load the settings
  std::string fpath = m_Makefile->GetHomeOutputDirectory();
  fpath += "/CMakeSystemConfig.cmake";
  m_Makefile->ReadListFile(NULL,fpath.c_str());
}
