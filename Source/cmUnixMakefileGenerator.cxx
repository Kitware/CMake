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

void cmUnixMakefileGenerator::GenerateMakefile()
{
  // Generate depends 
  cmMakeDepend md;
  md.SetMakefile(m_Makefile);
  md.DoDepends();
  // output the makefile fragment
  this->OutputMakefile("CMakeTargets.make"); 
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
  this->OutputMakeFlags(fout);
  this->OutputVerbatim(fout);
  this->OutputTargetRules(fout);
  this->OutputDependencies(fout);
  this->OutputTargets(fout);
  this->OutputSubDirectoryRules(fout);
  this->OutputObjectDepends(fout);
  this->OutputCustomRules(fout);
}

// Output the rules for any targets
void cmUnixMakefileGenerator::OutputTargetRules(std::ostream& fout)
{
  // for each target add to the list of targets
  fout << "TARGETS = ";
  const cmTargets &tgts = m_Makefile->GetTargets();
  // libraries
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::LIBRARY)
      {
      fout << " \\\nlib" << l->first.c_str() << "${CMAKE_LIB_EXT}";
      }
    }
  // executables
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (!l->second.GetType() == cmTarget::LIBRARY)
      {
      fout << "\\\n" << l->first.c_str();
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
  std::vector<std::string>::iterator j;
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(j = libdirs.begin(); j != libdirs.end(); ++j)
    { 
    std::string::size_type pos = (*j).find("-L");
    if((pos == std::string::npos || pos > 0)
       && (*j).find("${") == std::string::npos)
      {
      linkLibs += "-L";
      }
    linkLibs += *j;
    linkLibs += " ";
    }
  std::string librariesLinked;
  const cmTarget::LinkLibraries& libs = tgt.GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator j2;
  for(j2 = libs.begin(); j2 != libs.end(); ++j2)
    {
    // Don't link the library against itself!
    if(targetLibrary && (j2->first == targetLibrary)) continue;
    // don't look at debug libraries
    if (j2->second == cmTarget::DEBUG) continue;
    std::string::size_type pos = j2->first.find("-l");
    if((pos == std::string::npos || pos > 0)
       && j2->first.find("${") == std::string::npos)
      {
      librariesLinked += "-l";
      }
    librariesLinked += j2->first;
    librariesLinked += " ";
    }
  linkLibs += librariesLinked;

  if(!targetLibrary)
    {
    // For executables, add these a second time so order does not matter
    linkLibs += librariesLinked;
    }
  linkLibs += " ${LOCAL_LINK_FLAGS} ";
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
      fout << "lib" << l->first << ".a: ${KIT_OBJ} ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${AR} cr lib" << l->first << ".a ${KIT_OBJ} ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${RANLIB} lib" << l->first << ".a\n";
      fout << std::endl;

      fout << "lib" << l->first << "$(SHLIB_SUFFIX): ${KIT_OBJ} ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\trm -f lib" << l->first << "$(SHLIB_SUFFIX)\n";
      fout << "\t$(CXX) ${CXX_FLAGS} ${CMAKE_SHLIB_BUILD_FLAGS} -o \\\n";
      fout << "\t  lib" << l->first << "$(SHLIB_SUFFIX) \\\n";
      fout << "\t  ${KIT_OBJ} ${" << l->first << 
        "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, l->first.c_str(), l->second);
      fout << "\n\n";
      }
    else if (l->second.GetType() == cmTarget::EXECUTABLE)
      {
      fout << l->first << ": ${" << 
        l->first << "_SRC_OBJS} ${CMAKE_DEPEND_LIBS}\n";
      fout << "\t${CXX}  ${CXX_FLAGS} ${" << 
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
      libpath += "${CMAKE_LIB_EXT}";
      fout << libpath << " ";
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
      if(cmSystemTools::Grep(dir->c_str(), "CMakeTargets.make",
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
    fout << "-I" << i->c_str() << " ";
    }
  fout << m_Makefile->GetDefineFlags();
  fout << " ${LOCAL_INCLUDE_FLAGS} ";
  fout << "\n\n";
  fout << "default_target: all\n\n";
  // see if there are files to compile in this makefile
  // These are used for both libraries and executables
}

// output verbatim section
void cmUnixMakefileGenerator::OutputVerbatim(std::ostream& fout)
{
    std::vector<std::string>& MakeVerbatim = m_Makefile->GetMakeVerbatim();
  // Ouput user make text embeded in the input file
  for(unsigned int i =0; i < MakeVerbatim.size(); i++)
    {
    fout << MakeVerbatim[i] << "\n";
    }
  fout << "\n\n";
  
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
  fout << "SUBDIR_BUILD = \\\n";
  unsigned int i;
  for(i =0; i < SubDirectories.size(); i++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "build_" << subdir.c_str();
    if(i == SubDirectories.size()-1)
      {
      fout << " \n\n";
      }
    else
      {
      fout << " \\\n";
      }
    }
  fout << std::endl;
  fout << "SUBDIR_CLEAN = \\\n";
  for(i =0; i < SubDirectories.size(); i++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "clean_" << subdir.c_str();
    if(i == SubDirectories.size()-1)
      {
      fout << " \n\n";
      }
    else
      {
      fout << " \\\n";
      }
    }
  fout << std::endl;
  fout << "alldirs : ${SUBDIR_BUILD}\n\n";

  for(i =0; i < SubDirectories.size(); i++)
    {
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "build_" << subdir.c_str() << ":\n";
    fout << "\tcd " << SubDirectories[i].c_str()
         << "; ${MAKE} -${MAKEFLAGS} CMakeTargets.make\n";
    fout << "\tcd " << SubDirectories[i].c_str()
         << "; ${MAKE} -${MAKEFLAGS} all\n\n";

    fout << "clean_" << subdir.c_str() << ": \n";
    fout << "\tcd " << SubDirectories[i].c_str() 
         << "; ${MAKE} -${MAKEFLAGS} clean\n\n";
    }
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
        // with no outputs or depends
        if(commandFiles.m_Outputs.size() == 0)
          {
          fout << source.c_str() << ":\n\t" << command.c_str() << "\n\n";
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
