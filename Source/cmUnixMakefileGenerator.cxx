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
#include "cmGeneratedFileStream.h"

cmUnixMakefileGenerator::cmUnixMakefileGenerator()
{
  m_CacheOnly = false;
  m_Recurse = false;
}

void cmUnixMakefileGenerator::GenerateMakefile()
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
    md.GenerateMakefileDependencies();
    this->ProcessDepends(md);
    // output the makefile fragment
    this->OutputMakefile("Makefile"); 
    }
}

void cmUnixMakefileGenerator::ProcessDepends(const cmMakeDepend &md)
{
  // Now create cmDependInformation objects for files in the directory
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    std::vector<cmSourceFile> &classes = l->second.GetSourceFiles();
    for(std::vector<cmSourceFile>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      if(!i->GetIsAHeaderFileOnly())
        {
        // get the depends
        const cmDependInformation *info = 
          md.GetDependInformationForSourceFile(*i);
        
        // Delete any hints from the source file's dependencies.
        i->GetDepends().erase(i->GetDepends().begin(), i->GetDepends().end());
        
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
              i->GetDepends().push_back((*d)->m_FullPath);
              }
            }
          }
        }
      }
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
  this->OutputDependLibs(fout);
  this->OutputTargets(fout);
  this->OutputSubDirectoryRules(fout);
  std::string dependName = m_Makefile->GetStartOutputDirectory();
  dependName += "/cmake.depends";
  if(!this->m_CacheOnly)
    {
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
  this->OutputInstallRules(fout);
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
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.IsInAll())
      {
      if(l->second.GetType() == cmTarget::STATIC_LIBRARY)
        {
        fout << " \\\n" << m_LibraryOutputPath << "lib" << l->first.c_str()
             << ".a";
        }
      else if(l->second.GetType() == cmTarget::SHARED_LIBRARY)
        {
        fout << " \\\n" << m_LibraryOutputPath << "lib" << l->first.c_str()
             << m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        }
      else if(l->second.GetType() == cmTarget::MODULE_LIBRARY)
        {
        fout << " \\\n" << m_LibraryOutputPath << "lib" << l->first.c_str()
             << m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
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
      fout << " \\\n" << m_ExecutableOutputPath << l->first.c_str();
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
    if (classes.begin() != classes.end())
      {
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
  fout << "CLEAN_OBJECT_FILES = ";
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    std::vector<cmSourceFile> classes = l->second.GetSourceFiles();
    if (classes.begin() != classes.end())
      {
      fout << "${" << l->first << "_SRC_OBJS} ";
      }
    }
  fout << "\n\n";
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
  // Try to emit each search path once
  std::set<std::string> emitted;

  // Embed runtime search paths if possible and if required.
  bool outputRuntime = true;
  std::string runtimeFlag;
  std::string runtimeSep;
  std::vector<std::string> runtimeDirs;

  if(m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_FLAG"))
    {
    runtimeFlag = m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_FLAG");
    }
  if(m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_SEP"))
    {
    runtimeSep = m_Makefile->GetDefinition("CMAKE_SHLIB_RUNTIME_SEP");
    }

  // concatenate all paths or no?
  bool runtimeConcatenate = ( runtimeSep!="" );
  if(runtimeFlag == "")
    {
    outputRuntime = false;
    }

  // Some search paths should never be emitted
  emitted.insert("");
  emitted.insert("/usr/lib");

  // collect all the flags needed for linking libraries
  std::string linkLibs;
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(std::vector<std::string>::iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    { 
    std::string libpath = cmSystemTools::EscapeSpaces(libDir->c_str());
    if (m_LibraryOutputPath.size())
      {
      if(m_LibraryOutputPath != libpath 
         && (libpath.find(m_Makefile->GetHomeOutputDirectory()) 
             != std::string::npos))
        {
        emitted.insert(libpath);
        }
      }
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
    cmRegularExpression reg("(^[ \t]*\\-l)|(\\${)");
    if(lib->first.find('/') != std::string::npos
       && !reg.find(lib->first))
      {
      std::string dir, file;
      cmSystemTools::SplitProgramPath(lib->first.c_str(),
                                      dir, file);
      std::string libpath = cmSystemTools::EscapeSpaces(dir.c_str());
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
      cmRegularExpression libname("lib(.*)\\.(.*)");
      cmRegularExpression libname_noprefix("(.*)\\.(.*)");
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

  if(!targetLibrary)
    {
    // For executables, add these a second time so order does not matter
    linkLibs += librariesLinked;
    }
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


void cmUnixMakefileGenerator::OutputTargets(std::ostream& fout)
{
  // for each target
  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    if (l->second.GetType() == cmTarget::STATIC_LIBRARY)
      {
      fout << "#---------------------------------------------------------\n";
      fout << "# rules for a static library\n";
      fout << "#\n";
      fout << m_LibraryOutputPath << "lib" << l->first << ".a: ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${CMAKE_AR} ${CMAKE_AR_ARGS} "
           << m_LibraryOutputPath << "lib" << l->first << ".a ${" << 
        l->first << "_SRC_OBJS} \n";
      fout << "\t${CMAKE_RANLIB} "
           << m_LibraryOutputPath << "lib" << l->first << ".a\n";
      fout << "\n\n";
      }
    else if (l->second.GetType() == cmTarget::SHARED_LIBRARY)
      {
      fout << "#---------------------------------------------------------\n";
      fout << "# rules for a shared library\n";
      fout << "#\n";
      fout << m_LibraryOutputPath << "lib" << l->first << "$(SHLIB_SUFFIX):  ${" << 
        l->first << "_SRC_OBJS} ${" << l->first << "_DEPEND_LIBS} \n";
      fout << "\trm -f lib" << l->first << "$(SHLIB_SUFFIX)\n";
      fout << "\t$(CMAKE_CXX_COMPILER)  ${CMAKE_SHLIB_LINK_FLAGS} "
        "${CMAKE_SHLIB_BUILD_FLAGS} ${CMAKE_CXXFLAGS} -o \\\n";
      fout << "\t  " << m_LibraryOutputPath << "lib" << l->first << "$(SHLIB_SUFFIX) \\\n";
      fout << "\t  ${" << l->first << 
        "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, l->first.c_str(), l->second);
      fout << "\n\n";
      }
    else if (l->second.GetType() == cmTarget::MODULE_LIBRARY)
      {
      fout << "#---------------------------------------------------------\n";
      fout << "# rules for a shared module library\n";
      fout << "#\n";
      fout << m_LibraryOutputPath << "lib" << l->first << "$(MODULE_SUFFIX):  ${" << 
        l->first << "_SRC_OBJS} ${" << l->first << "_DEPEND_LIBS} \n";
      fout << "\trm -f lib" << l->first << "$(MODULE_SUFFIX)\n";
      fout << "\t$(CMAKE_CXX_COMPILER)  ${CMAKE_MODULE_LINK_FLAGS} "
        "${CMAKE_MODULE_BUILD_FLAGS} ${CMAKE_CXXFLAGS} -o \\\n";
      fout << "\t  " << m_LibraryOutputPath << "lib" << l->first << "$(MODULE_SUFFIX) \\\n";
      fout << "\t  ${" << l->first << 
        "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, l->first.c_str(), l->second);
      fout << "\n\n";
      }
    else if ((l->second.GetType() == cmTarget::EXECUTABLE)
             || (l->second.GetType() == cmTarget::WIN32_EXECUTABLE))
      {
      fout << m_ExecutableOutputPath << l->first << ": ${" << 
        l->first << "_SRC_OBJS} ${" << l->first << "_DEPEND_LIBS}\n";
      fout << "\t${CMAKE_CXX_COMPILER} ${CMAKE_SHLIB_LINK_FLAGS} ${CMAKE_CXXFLAGS} "
           << "${" << l->first << "_SRC_OBJS} ";
      this->OutputLinkLibraries(fout, NULL,l->second);
      fout << " -o " << m_ExecutableOutputPath << l->first << "\n\n";
      }
    }
}


// For each target that is an executable or shared library, generate
// the "<name>_DEPEND_LIBS" variable listing its library dependencies.
void cmUnixMakefileGenerator::OutputDependLibs(std::ostream& fout)
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
      fout << l->first << "_DEPEND_LIBS = ";
      
      // A library should not depend on itself!
      emitted.insert(l->first);
      
      // First look at all makefile level link libraries.
      const cmTarget::LinkLibraries& libs = m_Makefile->GetLinkLibraries();
      for(cmTarget::LinkLibraries::const_iterator lib = libs.begin();
          lib != libs.end(); ++lib)
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
    const char* cacheValue = m_Makefile->GetDefinition(lib->c_str());
    // if cache and not the current directory add a rule, to
    // jump into the directory and build for the first time
    if(cacheValue 
       && (strcmp(m_Makefile->GetCurrentOutputDirectory(), cacheValue) != 0))
      {
      std::string library = "lib";
      library += *lib;
      std::string libpath = cacheValue;
      // add the correct extension
      std::string ltname = *lib+"_LIBRARY_TYPE";
      const char* libType
        = m_Makefile->GetDefinition(ltname.c_str());
      if(libType && std::string(libType) == "SHARED")
        {
        library += m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
        }
      else if(libType && std::string(libType) == "MODULE")
	{
	library += m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
	}
      else
        {
        library += ".a";
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
      fout << libpath.c_str()
           << ":\n\tcd " << cacheValue 
           << "; ${MAKE} " << m_LibraryOutputPath << library.c_str() << "\n\n";
      }
    }
}

void cmUnixMakefileGenerator::OutputLibDepend(std::ostream& fout,
                                              const char* name)
{
  const char* cacheValue = m_Makefile->GetDefinition(name);
  if(cacheValue )
    {
    // if there is a cache value, then this is a library that cmake
    // knows how to build, so we can depend on it
    std::string libpath;
    if (strcmp(m_Makefile->GetCurrentOutputDirectory(), cacheValue) != 0)
      {
      // if the library is not in the current directory, then get the full
      // path to it
      libpath = cacheValue;
      if(m_LibraryOutputPath.size())
        {
        libpath = m_LibraryOutputPath;
        libpath += "lib";
        }
      else
        {
        libpath += "/lib";
        }
      }
    else
      {
      // library is in current Makefile so use lib as a prefix
      libpath = m_LibraryOutputPath;
      libpath += "lib";
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
    else
      {
      libpath += ".a";
      }
    fout << libpath << " ";
    }
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
    // Don't output a -I for the standard include path "/usr/include".
    // This can cause problems with certain standard library
    // implementations because the wrong headers may be found first.
    if(include != "/usr/include")
      {
      fout << "-I" << cmSystemTools::EscapeSpaces(i->c_str()).c_str() << " ";
      }
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
    fout << target << "_" << subdir.c_str() << ":";
    const std::set<cmStdString>& subdirDepends = m_Makefile->GetSubdirDepends(SubDirectories[i].c_str());
    for(std::set<cmStdString>::const_iterator d = subdirDepends.begin();
        d != subdirDepends.end(); ++d)
      {
      std::string fixed_d = FixDirectoryName(d->c_str());
      fout << " " << target << "_" << fixed_d.c_str();
      }
    fout << "\n";
    if(target1)
      {
	fout << "\t@if test ! -d " << SubDirectories[i].c_str() << "; then ${MAKE} rebuild_cache; fi\n"
	  "\tcd " << SubDirectories[i].c_str()
           << "; ${MAKE} -${MAKEFLAGS} " << target1 << "\n";
      }
    if(target2)
      {
      fout << "\t@cd " << SubDirectories[i].c_str()
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
  this->OutputSubDirectoryVars(fout, "SUBDIR_INSTALL", "install",
                               "install",
                               0,
                               SubDirectories);
}




// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend GenerateMakefile
void cmUnixMakefileGenerator::OutputObjectDepends(std::ostream& fout)
{
  // Iterate over every target.
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  for(std::map<cmStdString, cmTarget>::const_iterator target = targets.begin(); 
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
    "CMAKE_AR_ARGS       = @CMAKE_AR_ARGS@\n"
    "CMAKE_C_COMPILER    = @CMAKE_C_COMPILER@\n"
    "CMAKE_CFLAGS        = @CMAKE_C_FLAGS@\n"
    "CMAKE_SHLIB_CFLAGS  = @CMAKE_SHLIB_CFLAGS@\n"
    "\n"
    "CMAKE_CXX_COMPILER  = @CMAKE_CXX_COMPILER@\n"
    "CMAKE_CXXFLAGS      = @CMAKE_CXX_FLAGS@ @CMAKE_TEMPLATE_FLAGS@\n"
    "\n"
    "CMAKE_SHLIB_BUILD_FLAGS  = @CMAKE_SHLIB_BUILD_FLAGS@\n"
    "CMAKE_SHLIB_LINK_FLAGS   = @CMAKE_SHLIB_LINK_FLAGS@\n"
    "CMAKE_MODULE_BUILD_FLAGS = @CMAKE_MODULE_BUILD_FLAGS@\n"
    "CMAKE_MODULE_LINK_FLAGS  = @CMAKE_MODULE_LINK_FLAGS@\n"
    "DL_LIBS                  = @CMAKE_DL_LIBS@\n"
    "SHLIB_LD_LIBS            = @CMAKE_SHLIB_LD_LIBS@\n"
    "SHLIB_SUFFIX             = @CMAKE_SHLIB_SUFFIX@\n"
    "MODULE_SUFFIX            = @CMAKE_MODULE_SUFFIX@\n"
    "THREAD_LIBS              = @CMAKE_THREAD_LIBS@\n"
    "\n"
    "# set up the path to the rulesgen program\n"
    "CMAKE_COMMAND = ${CMAKE_COMMAND}\n"
    "\n"
    "\n"
    "\n";
  std::string replaceVars = variables;
  
  m_Makefile->ExpandVariablesInString(replaceVars);
  fout << replaceVars.c_str();
  fout << "CMAKE_CURRENT_SOURCE = " << m_Makefile->GetStartDirectory() << "\n";
  fout << "CMAKE_CURRENT_BINARY = " << m_Makefile->GetStartOutputDirectory() << "\n";
}


void cmUnixMakefileGenerator::OutputInstallRules(std::ostream& fout)
{
  const char* root
    = m_Makefile->GetDefinition("CMAKE_ROOT");
  fout << "INSTALL = " << root << "/Templates/install-sh -c\n";
  fout << "INSTALL_PROGRAM = ${INSTALL}\n";
  fout << "INSTALL_DATA =    ${INSTALL} -m 644\n";
  
  const cmTargets &tgts = m_Makefile->GetTargets();
  fout << "install: ${SUBDIR_INSTALL}\n";
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
      fout << "\t@if [ ! -d " << prefix << l->second.GetInstallPath() << 
	" ] ; then \\\n";
      fout << "\t   echo \"Making directory " << prefix 
	   << l->second.GetInstallPath() << " \"; \\\n";
      fout << "\t   mkdir -p " << prefix << l->second.GetInstallPath() 
	   << "; \\\n";
      fout << "\t   chmod 755 " <<  prefix << l->second.GetInstallPath() 
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
	  fout << " " << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::SHARED_LIBRARY:
	  fout << "\t$(INSTALL_DATA) " << m_LibraryOutputPath << "lib" 
               << l->first;
          fout << m_Makefile->GetDefinition("CMAKE_SHLIB_SUFFIX");
	  fout << " " << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::MODULE_LIBRARY:
	  fout << "\t$(INSTALL_DATA) " << m_LibraryOutputPath << "lib" 
               << l->first;
          fout << m_Makefile->GetDefinition("CMAKE_MODULE_SUFFIX");
	  fout << " " << prefix << l->second.GetInstallPath() << "\n";
	  break;
	case cmTarget::WIN32_EXECUTABLE:
	case cmTarget::EXECUTABLE:
          fout << "\t$(INSTALL_PROGRAM) " << m_ExecutableOutputPath 
               << l->first
               << cmSystemTools::GetExecutableExtension()
	       << " " << prefix << l->second.GetInstallPath() << "\n";
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
		 << " " << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\t elif [ -f ${srcdir}/" << *i << " ] ; then \\\n";
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
	    fout << "${srcdir}/" << *i 
		 << " " << prefix << l->second.GetInstallPath() << "; \\\n";
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
		 << " " << prefix << l->second.GetInstallPath() << "; \\\n";
	    fout << "\t elif [ -f ${srcdir}/" << *i << " ] ; then \\\n";
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
	    fout << "${srcdir}/" << *i 
		 << " " << prefix << l->second.GetInstallPath() << "; \\\n";
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

void cmUnixMakefileGenerator::OutputMakeRules(std::ostream& fout)
{
  // only include the cmake.depends and not the Makefile, as
  // building one will cause the other to be built
  this->OutputMakeRule(fout, 
                       "Default build rule",
                       "all",
                       "cmake.depends ${TARGETS} ${SUBDIR_BUILD} ${CMAKE_COMMAND}",
                       0);
  this->OutputMakeRule(fout, 
                       "remove generated files",
                       "clean",
                       "${SUBDIR_CLEAN}",
                       "rm -f ${CLEAN_OBJECT_FILES} ${EXECUTABLES} ${TARGETS}");
  this->OutputMakeRule(fout, 
                       "Rule to build the cmake.depends and Makefile as side effect",
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
                       "Rebuild the cache",
                       "${CMAKE_BINARY_DIR}/CMakeCache.txt",
		       0,
                       "${CMAKE_COMMAND} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");

  this->OutputMakeRule(fout, 
                       "Rule to keep make from removing Makefiles "
                       "if control-C is hit during a run of cmake.",
                       ".PRECIOUS",
                       "Makefile cmake.depends",
                       0);
  
  this->OutputSourceObjectBuildRules(fout);
  // see if there is already a target for a cmake executable in this
  // makefile
  bool buildingCMake = false;
  std::map<cmStdString, cmTarget>& targets = m_Makefile->GetTargets();
  for(cmTargets::const_iterator l = targets.begin(); 
      l != targets.end(); l++)
    {
    if ((l->second.GetType() == cmTarget::EXECUTABLE ||
         l->second.GetType() == cmTarget::WIN32_EXECUTABLE) &&
        l->second.IsInAll())
      {
      if(l->first == "cmake")
        {
        buildingCMake = true;
        }
      }
    }
  // do not put this command in for the cmake project
  if(!buildingCMake)
    {
    this->OutputMakeRule(fout, 
                         "Rebuild cmake dummy rule",
                         "${CMAKE_COMMAND}",
                         0,
                         "echo \"cmake might be out of date\"");
    }

  // find ctest
  std::string ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  if (cmSystemTools::FileExists(ctest.c_str()))
    {
    this->OutputMakeRule(fout, 
                         "run any tests",
                         "test",
                         "",
                         ctest.c_str());
    }
}


void cmUnixMakefileGenerator::OutputSourceObjectBuildRules(std::ostream& fout)
{
  fout << "# Rules to build .o files from their sources:\n";

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
    const std::vector<cmSourceFile>& sources = target->second.GetSourceFiles();
    for(std::vector<cmSourceFile>::const_iterator source = sources.begin(); 
        source != sources.end(); ++source)
      {
      if(!source->IsAHeaderFileOnly())
        {
        std::string shortName;
        std::string sourceName;
        // If the full path to the source file includes this
        // directory, we want to use the relative path for the
        // filename of the object file.  Otherwise, we will use just
        // the filename portion.
        if((cmSystemTools::GetFilenamePath(source->GetFullPath()).find(m_Makefile->GetCurrentDirectory()) == 0)
           || (cmSystemTools::GetFilenamePath(source->GetFullPath()).find(m_Makefile->GetCurrentOutputDirectory()) == 0))
          {
          sourceName = source->GetSourceName()+"."+source->GetSourceExtension();
          shortName = source->GetSourceName();
          
          // The path may be relative.  See if a directory needs to be
          // created for the output file.  This is a ugly, and perhaps
          // should be moved elsewhere.
          std::string relPath =
            cmSystemTools::GetFilenamePath(source->GetSourceName());
          if(relPath != "")
            {
            std::string outPath = m_Makefile->GetCurrentOutputDirectory();
            outPath += "/"+relPath;
            cmSystemTools::MakeDirectory(outPath.c_str());
            }
          }
        else
          {
          sourceName = source->GetFullPath();
          shortName = cmSystemTools::GetFilenameName(source->GetSourceName());
          }
        // Only output a rule for each .o once.
        if(rules.find(shortName) == rules.end())
          {
          rules.insert(shortName);
          fout << shortName.c_str() << ".o : " << sourceName.c_str() << "\n";
          std::string ext = source->GetSourceExtension();
          if ( ext == "cxx" || ext == "cc" || ext == "cpp" || ext == "C" )
            {
            fout << "\t${CMAKE_CXX_COMPILER} ${CMAKE_CXXFLAGS} " << exportsDef.c_str()
                 << (shared? "${CMAKE_SHLIB_CFLAGS} ":"") << "${INCLUDE_FLAGS} -c $< -o $@\n\n";
            }
          else if ( ext == "c" )
            {
            fout << "\t${CMAKE_C_COMPILER} ${CMAKE_CFLAGS} " << exportsDef.c_str()
                 << (shared? "${CMAKE_SHLIB_CFLAGS} ":"") << "${INCLUDE_FLAGS} -c $< -o $@\n\n";
            }
          }
        }
      }
    }
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
  fout << "\n";

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
    if(m_Makefile->GetDefinition("CMAKE_CXX_COMPILER"))
      {
      std::string env = "CXX=${CMAKE_CXX_COMPILER}";
      m_Makefile->ExpandVariablesInString(env);
      std::cout << "Setting: " << env.c_str() << "\n";
      putenv(const_cast<char*>(env.c_str()));
      }
    if(m_Makefile->GetDefinition("CMAKE_C_COMPILER"))
      {
      std::string env = "CC=${CMAKE_C_COMPILER}";
      m_Makefile->ExpandVariablesInString(env);
      std::cout << "Setting: " << env.c_str() << "\n";
      putenv(const_cast<char*>(env.c_str()));
      }
    // currently we run configure shell script here to determine the info
    std::string output;
    std::string cmd = "cd ";
    cmd += m_Makefile->GetHomeOutputDirectory();
    cmd += "; ";
    const char* root
      = m_Makefile->GetDefinition("CMAKE_ROOT");
    cmd += root;
    cmd += "/Templates/configure";
    cmSystemTools::RunCommand(cmd.c_str(), output);
    m_Makefile->AddDefinition("RUN_CONFIGURE", true);
    }

  // now load the settings
  std::string fpath = m_Makefile->GetHomeOutputDirectory();
  fpath += "/CMakeSystemConfig.cmake";
  m_Makefile->ReadListFile(NULL,fpath.c_str());
}
