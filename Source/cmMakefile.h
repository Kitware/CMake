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
#ifndef cmMakefile_h
#define cmMakefile_h

#include "cmStandardIncludes.h"
#include "cmData.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmSourceGroup.h"
#include "cmTarget.h"
#include "cmCacheManager.h"

class cmFunctionBlocker;
class cmCommand;
class cmMakefileGenerator;

/** \class cmMakefile
 * \brief Process the input CMakeLists.txt file.
 *
 * Process and store into memory the input CMakeLists.txt file.
 * Each CMakeLists.txt file is parsed and the commands found there
 * are added into the build process.
 */
class cmMakefile
{
public:
  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion() { return 1;}
  static unsigned int GetMinorVersion() { return 1;}
  /**
   * Return the major and minor version of the cmake that
   * was used to write the currently loaded cache, note
   * this method will not work before the cache is loaded.
   */
  static unsigned int GetCacheMajorVersion();
  static unsigned int GetCacheMinorVersion();
  
  /**
   * Construct an empty makefile.
   */
  cmMakefile();

  /**
   * Destructor.
   */
  ~cmMakefile();

  /**
   * Read and parse a CMakeLists.txt file.
   */
  bool ReadListFile(const char* listfile, const char* external= 0); 

  /**
   * Add a wrapper generator.
   */
  void AddCommand(cmCommand* );

  /**
   * Add a function blocker to this makefile
   */
  void AddFunctionBlocker(cmFunctionBlocker *fb)
    { m_FunctionBlockers.insert(fb);}
  void RemoveFunctionBlocker(cmFunctionBlocker *fb)
    { m_FunctionBlockers.erase(fb);}
  void RemoveFunctionBlocker(const char *name, const std::vector<std::string> &args);
  
  /**
   * Specify the makefile generator. This is platform/compiler
   * dependent, although the interface is through a generic
   * superclass.
   */
  void SetMakefileGenerator(cmMakefileGenerator*);
  
  ///! Get the current makefile generator.
  cmMakefileGenerator* GetMakefileGenerator() 
    { return m_MakefileGenerator;}

  /**
   * Produce the output makefile.
   */
  void GenerateMakefile();
  
  /**
   * run the final pass on all commands.
   */
  void FinalPass();
  
  /**
   * Print the object state to std::cout.
   */
  void Print() const;
  
  /**
   * Add a custom command to the build.
   */
  void AddCustomCommand(const char* source,
                        const char* command,
                        const std::vector<std::string>& commandArgs,
                        const std::vector<std::string>& depends,
                        const std::vector<std::string>& outputs,
                        const char *target);

  void AddCustomCommand(const char* source,
                        const char* command,
                        const std::vector<std::string>& commandArgs,
                        const std::vector<std::string>& depends,
                        const char* output,
                        const char* target);
  
  /**
   * Add a define flag to the build.
   */
  void AddDefineFlag(const char* definition);

  /**
   * Add an executable to the build.
   */
  void AddExecutable(const char *exename, 
                     const std::vector<std::string> &srcs);
  void AddExecutable(const char *exename, 
                     const std::vector<std::string> &srcs, bool win32);

  /**
   * Add a utility to the build.  A utiltity target is
   * a command that is run every time a target is built.
   */
  void AddUtilityCommand(const char* utilityName,
                         const char* command,
                         const char* arguments,
                         bool all);
  void AddUtilityCommand(const char* utilityName,
                         const char* command,
                         const char* arguments,
                         bool all,
                         const std::vector<std::string> &depends,
                         const std::vector<std::string> &outputs);

  /**
   * Get a list of link libraries in the build.
   */
  cmTarget::LinkLibraries& GetLinkLibraries()
    { 
    return m_LinkLibraries;
    }

  /**
   * Get a list of link libraries in the build.
   */
  const cmTarget::LinkLibraries& GetLinkLibraries() const
    { 
    return m_LinkLibraries;
    }

  /**
   * Add a link library to the build.
   */
  void AddLinkLibrary(const char*);
  void AddLinkLibrary(const char*, cmTarget::LinkLibraryType type);
  void AddLinkLibraryForTarget(const char *tgt, const char*, 
                               cmTarget::LinkLibraryType type);

  /**
   * Add a link directory to the build.
   */
  void AddLinkDirectory(const char*);

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const char*);

  /**
   * Add a subdirectory dependency.
   */
  void AddSubdirDependency(const char* subdir, const char* dependency);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectory(const char*, bool before = false);

  /**
   * Add a variable definition to the build. This variable
   * can be used in CMake to refer to lists, directories, etc.
   */
  void AddDefinition(const char* name, const char* value);
  ///! Add a definition to this makefile and the global cmake cache.
  void AddCacheDefinition(const char* name, const char* value, 
                          const char* doc,
                          cmCacheManager::CacheEntryType type);

  /**
   * Add bool variable definition to the build. 
   */
  void AddDefinition(const char* name, bool);
  ///! Add a definition to this makefile and the global cmake cache.
  void AddCacheDefinition(const char* name, bool, const char* doc);

  /**
   * Specify the name of the project for this build.
   */
  void SetProjectName(const char*);

  /**
   * Get the name of the project for this build.
   */
  const char* GetProjectName()
    {
    return m_ProjectName.c_str();
    }
  
  /**
   * Set the name of the library.
   */
  void AddLibrary(const char *libname, int shared,
                  const std::vector<std::string> &srcs);

  /**
   * Add a class/source file to the build.
   */
  void AddSource(cmSourceFile& ,const char *srcListName);

  /**
   * Remove a class/source file from the build.
   */
  void RemoveSource(cmSourceFile& ,const char *srcListName);

  /**
   * Add a source group for consideration when adding a new source.
   */
  void AddSourceGroup(const char* name, const char* regex);
  
  /**
   * Add an auxiliary directory to the build.
   */
  void AddExtraDirectory(const char* dir);
  

  /**
   * Add an auxiliary directory to the build.
   */
  void MakeStartDirectoriesCurrent()
    {
      m_cmCurrentDirectory = m_cmStartDirectory;
      m_CurrentOutputDirectory = m_StartOutputDirectory;
    }
  
  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is where
   * CMakeSetup or configure was run. Remember that CMake processes
   * CMakeLists files by recursing up the tree starting at the StartDirectory
   * and going up until it reaches the HomeDirectory.  
   */
  void SetHomeDirectory(const char* dir);
  const char* GetHomeDirectory() const
    {
    return m_cmHomeDirectory.c_str();
    }
  void SetHomeOutputDirectory(const char* lib);
  const char* GetHomeOutputDirectory() const
    {
    return m_HomeOutputDirectory.c_str();
    }
  //@}
  
  //@{
  /**
   * Set/Get the start directory (or output directory). The start directory
   * is the directory of the CMakeLists.txt file that started the current
   * round of processing. Remember that CMake processes CMakeLists files by
   * recursing up the tree starting at the StartDirectory and going up until
   * it reaches the HomeDirectory.  
   */
  void SetStartDirectory(const char* dir) 
    {
      m_cmStartDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(m_cmStartDirectory);
    }
  const char* GetStartDirectory() const
    {
      return m_cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      m_StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_StartOutputDirectory);
      cmSystemTools::MakeDirectory(m_StartOutputDirectory.c_str());
    }
  const char* GetStartOutputDirectory() const
    {
      return m_StartOutputDirectory.c_str();
    }
  //@}

  //@{
  /**
   * Set/Get the current directory (or output directory) in the project. The
   * current directory is the directory of the CMakeLists.txt file that is
   * currently being processed. Remember that CMake processes CMakeLists
   * files by recursing up the tree starting at the StartDirectory and going
   * up until it reaches the HomeDirectory.  
   */
  void SetCurrentDirectory(const char* dir) 
    {
      m_cmCurrentDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(m_cmCurrentDirectory);
    }
  const char* GetCurrentDirectory() const 
    {
      return m_cmCurrentDirectory.c_str();
    }
  void SetCurrentOutputDirectory(const char* lib)
    {
      m_CurrentOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_CurrentOutputDirectory);
    }
  const char* GetCurrentOutputDirectory() const
    {
      return m_CurrentOutputDirectory.c_str();
    }

  /* Get the current CMakeLists.txt file that is being processed.  This
   * is just used in order to be able to 'branch' from one file to a second
   * transparently */
  const char* GetCurrentListFile() const
    {
      return m_cmCurrentListFile.c_str();
    }

  //@}

  /** 
   * Set a regular expression that include files must match
   * in order to be considered as part of the depend information.
   */
  void SetIncludeRegularExpression(const char* regex)
    {
      m_IncludeFileRegularExpression = regex;
    }

  /** 
   * Set a regular expression that include files that are not found
   * must match in order to be considered a problem.
   */
  void SetComplainRegularExpression(const char* regex)
    {
      m_ComplainFileRegularExpression = regex;
    }

  /**
   * Get the list of targets
   */
  cmTargets &GetTargets() { return m_Targets; }
  const cmTargets &GetTargets() const { return m_Targets; }

  /**
   * Get a list of the build subdirectories.
   */
  const std::vector<std::string>& GetSubDirectories()
    { 
    return m_SubDirectories;
    }

  /**
   * Get the subdirectory dependencies for the given subdirectory.
   */
  const std::set<cmStdString>& GetSubdirDepends(const char* subdir)
    { 
    return m_SubdirDepends[subdir];
    }

  /**
   * Get a list of include directories in the build.
   */
  std::vector<std::string>& GetIncludeDirectories()
    { 
    return m_IncludeDirectories;
    }
  const std::vector<std::string>& GetIncludeDirectories() const
    { 
    return m_IncludeDirectories;
    }

  /**
   * Get a list of link directories in the build.
   */
  std::vector<std::string>& GetLinkDirectories()
    { 
    return m_LinkDirectories;
    }
  
  /**
   * Return a list of source files in this makefile.
   */
  typedef std::map<cmStdString,std::vector<cmSourceFile> > SourceMap;
  const SourceMap &GetSources() const {return  m_Sources;}
  SourceMap &GetSources() {return  m_Sources;}
  cmSourceFile *GetSource(const char *srclist, const char *sourceName);
  
  /**
   * Obtain a list of auxiliary source directories.
   */
  std::vector<std::string>& GetAuxSourceDirectories()
    {return m_AuxSourceDirectories;}

  //@{
  /**
   * Return a list of extensions associated with source and header
   * files
   */
  const std::vector<std::string>& GetSourceExtensions() const
    {return m_SourceFileExtensions;}
  const std::vector<std::string>& GetHeaderExtensions() const
    {return m_HeaderFileExtensions;}
  //@}

  /**
   * Given a variable name, return its value (as a string).
   * If the variable is not found in this makefile instance, the
   * cache is then queried.
   */
  const char* GetDefinition(const char*) const;
  
  /** Test a boolean cache entry to see if it is true or false, 
   *  returns false if no entry defined.
   */
  bool IsOn(const char* name);

  /**
   * Get a list of preprocessor define flags.
   */
  const char* GetDefineFlags()
    {return m_DefineFlags.c_str();}
  
  /**
   * Get the vector of used command instances.
   */
  const std::vector<cmCommand*>& GetUsedCommands() const
    {return m_UsedCommands;}
  
  /**
   * Get the vector source groups.
   */
  const std::vector<cmSourceGroup>& GetSourceGroups() const
    { return m_SourceGroups; }

  /**
   * Get the vector of list files on which this makefile depends
   */
  const std::vector<std::string>& GetListFiles() const
    { return m_ListFiles; }
  
  ///! When the file changes cmake will be re-run from the build system.
  void AddCMakeDependFile(const char* file)
    { m_ListFiles.push_back(file);}
  
  /**
   * Dump documentation to a file. If 0 is returned, the
   * operation failed.
   */
  int DumpDocumentationToFile(std::ostream&);

  /**
   * Expand all defined varibles in the string.  
   * Defined varibles come from the m_Definitions map.
   * They are expanded with ${var} where var is the
   * entry in the m_Definitions map.  Also @var@ is
   * expanded to match autoconf style expansions.
   */
  const char *ExpandVariablesInString(std::string& source) const;
  const char *ExpandVariablesInString(std::string& source, bool escapeQuotes,
                                      bool atOnly = false) const;

  /**
   * Remove any remaining variables in the string. Anything with ${var} or
   * @var@ will be removed.  
   */
  void RemoveVariablesInString(std::string& source, bool atOnly = false) const;

  /**
   * Expand variables in the makefiles ivars such as link directories etc
   */
  void ExpandVariables();  

  /** Recursivly read and create a cmMakefile object for
   *  all CMakeLists.txt files in the GetSubDirectories list.
   *  Once the file is found, it ReadListFile is called on
   *  the cmMakefile created for it.
   */
  void FindSubDirectoryCMakeListsFiles(std::vector<cmMakefile*>& makefiles);
  
  /**
   * find what source group this source is in
   */
  cmSourceGroup& FindSourceGroup(const char* source,
                                 std::vector<cmSourceGroup> &groups);

  void RegisterData(cmData*);
  void RegisterData(const char*, cmData*);
  cmData* LookupData(const char*) const;
  
  /**
   * execute a single CMake command
   */
  void ExecuteCommand(std::string &name, std::vector<std::string> const& args);
  
    
protected:
  std::string m_Prefix;
  std::vector<std::string> m_AuxSourceDirectories; // 

  std::string m_cmCurrentDirectory; 
  std::string m_CurrentOutputDirectory; 
  std::string m_cmStartDirectory; 
  std::string m_StartOutputDirectory; 
  std::string m_cmHomeDirectory; 
  std::string m_HomeOutputDirectory;
  std::string m_cmCurrentListFile;

  std::string m_ProjectName;	// project name

  // libraries, classes, and executables
  cmTargets m_Targets;
  SourceMap m_Sources; 

  std::vector<std::string> m_SubDirectories; // list of sub directories
  struct StringSet : public std::set<cmStdString>
  {
  };
  
  std::map<cmStdString, StringSet >  m_SubdirDepends;
  
  // The include and link-library paths.  These may have order
  // dependency, so they must be vectors (not set).
  std::vector<std::string> m_IncludeDirectories;
  std::vector<std::string> m_LinkDirectories;
  
  std::vector<std::string> m_ListFiles; // list of command files loaded
  
  
  cmTarget::LinkLibraries m_LinkLibraries;

  std::string m_IncludeFileRegularExpression;
  std::string m_ComplainFileRegularExpression;
  std::vector<std::string> m_SourceFileExtensions;
  std::vector<std::string> m_HeaderFileExtensions;
  std::string m_DefineFlags;
  std::vector<cmSourceGroup> m_SourceGroups;
  typedef std::map<cmStdString, cmCommand*> RegisteredCommandsMap;
  typedef std::map<cmStdString, cmStdString> DefinitionMap;
  DefinitionMap m_Definitions;
  RegisteredCommandsMap m_Commands;
  std::vector<cmCommand*> m_UsedCommands;
  cmMakefileGenerator* m_MakefileGenerator;
  bool IsFunctionBlocked(const char *name, std::vector<std::string> const& args);
  
private:
  /**
   * Get the name of the parent directories CMakeLists file
   * given a current CMakeLists file name
   */
  std::string GetParentListFileName(const char *listFileName);

  void ReadSources(std::ifstream& fin, bool t);
  friend class cmMakeDepend;	// make depend needs direct access 
				// to the m_Sources array 
  void PrintStringVector(const char* s, const std::vector<std::string>& v) const;
  void AddDefaultCommands();
  void AddDefaultDefinitions();
  std::set<cmFunctionBlocker *> m_FunctionBlockers;
  
  typedef std::map<cmStdString, cmData*> DataMap;
  DataMap m_DataMap;
  bool m_Inheriting;
};


#endif
