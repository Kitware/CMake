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
#ifndef cmMakefile_h
#define cmMakefile_h

#include "cmData.h"
#include "cmSystemTools.h"
#include "cmSourceGroup.h"
#include "cmTarget.h"
#include "cmListFileCache.h"
#include "cmCacheManager.h"

#include <cmsys/RegularExpression.hxx>

class cmFunctionBlocker;
class cmCommand;
class cmLocalGenerator;
class cmMakeDepend;
class cmSourceFile;
class cmVariableWatch;
class cmake;

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
  static unsigned int GetMajorVersion() { return CMake_VERSION_MAJOR; }
  static unsigned int GetMinorVersion() { return CMake_VERSION_MINOR; }
  static const char* GetReleaseVersion();
  
  /**
   * Return the major and minor version of the cmake that
   * was used to write the currently loaded cache, note
   * this method will not work before the cache is loaded.
   */
  unsigned int GetCacheMajorVersion();
  unsigned int GetCacheMinorVersion();
  
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
   * Add a function blocker to this makefile
   */
  void AddFunctionBlocker(cmFunctionBlocker *fb)
    { m_FunctionBlockers.push_back(fb);}
  void RemoveFunctionBlocker(cmFunctionBlocker *fb)
    { m_FunctionBlockers.remove(fb);}
  void RemoveFunctionBlocker(const cmListFileFunction& lff);

  /**
   * Add file to the written file list. These file should not be in the list
   * of dependencies because they cause infinite loops.
   */
  void AddWrittenFile(const char* file);
  bool HasWrittenFile(const char* file);

  /**
   * Check if there are any infinite loops
   */
  bool CheckInfiniteLoops();
  
  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  int TryCompile(const char *srcdir, const char *bindir, 
                 const char *projectName, const char *targetName,
                 const std::vector<std::string> *cmakeArgs,
                 std::string *output);
    
  /**
   * Specify the makefile generator. This is platform/compiler
   * dependent, although the interface is through a generic
   * superclass.
   */
  void SetLocalGenerator(cmLocalGenerator*);
  
  ///! Get the current makefile generator.
  cmLocalGenerator* GetLocalGenerator() 
    { return m_LocalGenerator;}

  /**
   * Perform FinalPass, Library dependency analysis etc before output of the
   * makefile.  
   */
  void ConfigureFinalPass();
  
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
  void AddCustomCommandToOutput(const char* output,
                                const char* command,
                                const std::vector<std::string>& commandArgs,
                                const char *main_dependency,
                                const std::vector<std::string>& depends,
                                const char *comment = 0,
                                bool replace = false);
  void AddCustomCommandToTarget(const char* target,
                                const char* command,
                                const std::vector<std::string>& commandArgs,
                                cmTarget::CustomCommandType type,
                                const char *comment = 0);
  void AddCustomCommandToTarget(const char* target,
                                const char* command,
                                const std::vector<std::string>& commandArgs,
                                cmTarget::CustomCommandType type,
                                const char *comment,
                                const std::vector<std::string>& depends);
  
  /**
   * Add a custom command to the build.
   */
  void AddCustomCommand(const char* source,
                        const char* command,
                        const std::vector<std::string>& commandArgs,
                        const std::vector<std::string>& depends,
                        const std::vector<std::string>& outputs,
                        const char *target,
                        const char *comment = 0);

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
  void RemoveDefineFlag(const char* definition);

  /**
   * Add an executable to the build.
   */
  cmTarget* AddExecutable(const char *exename, 
                     const std::vector<std::string> &srcs);

  /**
   * Add a utility to the build.  A utiltity target is
   * a command that is run every time a target is built.
   */
  void AddUtilityCommand(const char* utilityName,
                         const char* command,
                         const char* arguments,
                         bool all,
                         const std::vector<std::string> &depends);
  void AddUtilityCommand(const char* utilityName,
                         const char* command,
                         const char* arguments,
                         bool all,
                         const std::vector<std::string> &depends,
                         const std::vector<std::string> &outputs);

  /**
   * Add a link library to the build.
   */
  void AddLinkLibrary(const char*);
  void AddLinkLibrary(const char*, cmTarget::LinkLibraryType type);
  void AddLinkLibraryForTarget(const char *tgt, const char*, 
                               cmTarget::LinkLibraryType type);
  void AddLinkDirectoryForTarget(const char *tgt, const char* d);

  /**
   * Add a link directory to the build.
   */
  void AddLinkDirectory(const char*);

  /**
   * Get the list of link directories
   */
  std::vector<std::string>& GetLinkDirectories()
    {
    return m_LinkDirectories;
    }
  const std::vector<std::string>& GetLinkDirectories() const
    {
    return m_LinkDirectories;
    }
  void SetLinkDirectories(const std::vector<std::string>& vec)
    {
    m_LinkDirectories = vec;
    }

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const char*, bool includeTopLevel=true, bool preorder = false);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectory(const char*, bool before = false);

  /**
   * Find a library (as in cmSystemTools) but add in compiler specific paths
   */
  std::string FindLibrary(const char* name,
                          const std::vector<std::string>& path);

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
   * Remove a variable definition from the build.  This is not valid
   * for cache entries, and will only affect the current makefile.
   */
  void RemoveDefinition(const char* name);
  
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
  
  std::vector<std::string> const& GetParentProjects()
    {
      return m_ParentProjects;
    }
  /**
   * Set the name of the library.
   */
  void AddLibrary(const char *libname, int shared,
                  const std::vector<std::string> &srcs);

  /**
   * Add a source group for consideration when adding a new source.
   */
  void AddSourceGroup(const char* name, const char* regex=0);
  
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
      this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", m_cmCurrentDirectory.c_str());
      this->AddDefinition("CMAKE_CURRENT_BINARY_DIR", m_CurrentOutputDirectory.c_str());
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
      this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", m_cmCurrentDirectory.c_str());
    }
  const char* GetCurrentDirectory() const 
    {
      return m_cmCurrentDirectory.c_str();
    }
  void SetCurrentOutputDirectory(const char* lib)
    {
      m_CurrentOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_CurrentOutputDirectory);
      this->AddDefinition("CMAKE_CURRENT_BINARY_DIR", m_CurrentOutputDirectory.c_str());
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
  const char* GetIncludeRegularExpression()
    { 
      return m_IncludeFileRegularExpression.c_str();
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
  const std::vector<std::pair<cmStdString, bool> >& GetSubDirectories()
    { 
    return m_SubDirectories;
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
  void SetIncludeDirectories(const std::vector<std::string>& vec)
    {
    m_IncludeDirectories = vec;
    }

  /** Expand out any arguements in the vector that have ; separated
   *  strings into multiple arguements.  A new vector is created 
   *  containing the expanded versions of all arguments in argsIn.
   * This method differes from the one in cmSystemTools in that if
   * the CmakeLists file is version 1.2 or earlier it will check for
   * source lists being used without ${} around them
   */
  void ExpandSourceListArguments(std::vector<std::string> const& argsIn,
                                 std::vector<std::string>& argsOut,
                                 unsigned int startArgumentIndex);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmSourceFile* GetSource(const char* sourceName) const;
  ///! Add a new cmSourceFile to the list of sources for this makefile.
  cmSourceFile* AddSource(cmSourceFile const&);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then create the source file and return it. generated 
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* GetOrCreateSource(const char* sourceName, 
                                  bool generated = false);
  
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
  const char* GetSafeDefinition(const char*) const;

  /**
   * Get the list of all variables in the current space. If argument
   * cacheonly is specified and is greater than 0, then only cache
   * variables will be listed.
   */
  std::vector<std::string> GetDefinitions(int cacheonly=0) const;
  
  /** Test a boolean cache entry to see if it is true or false, 
   *  returns false if no entry defined.
   */
  bool IsOn(const char* name) const;

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
   * Get the source group
   */
  cmSourceGroup* GetSourceGroup(const char* name); 

  /**
   * Get the vector of list files on which this makefile depends
   */
  const std::vector<std::string>& GetListFiles() const
    { return m_ListFiles; }
  
  ///! When the file changes cmake will be re-run from the build system.
  void AddCMakeDependFile(const char* file)
    { m_ListFiles.push_back(file);}
  
  /**
   * Expand all defined variables in the string.  
   * Defined variables come from the m_Definitions map.
   * They are expanded with ${var} where var is the
   * entry in the m_Definitions map.  Also @var@ is
   * expanded to match autoconf style expansions.
   */
  const char *ExpandVariablesInString(std::string& source) const;
  const char *ExpandVariablesInString(std::string& source, bool escapeQuotes,
                                      bool atOnly = false,
                                      const char* filename = 0,
                                      long line = -1) const;

  /**
   * Remove any remaining variables in the string. Anything with ${var} or
   * @var@ will be removed.  
   */
  void RemoveVariablesInString(std::string& source, bool atOnly = false) const;

  /**
   * Expand variables in the makefiles ivars such as link directories etc
   */
  void ExpandVariables();  
  void ExpandVariablesInCustomCommands();

  /**
   * Replace variables and #cmakedefine lines in the given string.
   * See cmConfigureFileCommand for details.
   */
  void ConfigureString(const std::string& input, std::string& output,
                       bool atOnly, bool escapeQuotes);

  /**
   * Copy file but change lines acording to ConfigureString
   */
  int ConfigureFile(const char* infile, const char* outfile, 
    bool copyonly, bool atOnly, bool escapeQuotes);

  /**
   * find what source group this source is in
   */
  cmSourceGroup& FindSourceGroup(const char* source,
                                 std::vector<cmSourceGroup> &groups);
  void RegisterData(cmData*);
  void RegisterData(const char*, cmData*);
  cmData* LookupData(const char*) const;
  
  /**
   * Execute a single CMake command.  Returns true if the command
   * succeeded or false if it failed.
   */
  bool ExecuteCommand(const cmListFileFunction& lff);
  
  /** Check if a command exists. */
  bool CommandExists(const char* name) const;
    
  /**
   * Add a command to this cmake instance
   */
  void AddCommand(cmCommand* );

  ///! Enable support for the named language, if null then all languages are enabled.
  void EnableLanguage(const char* );

  /**
   * Set/Get the name of the parent directories CMakeLists file
   * given a current CMakeLists file name
   */
  cmCacheManager *GetCacheManager() const;
  cmVariableWatch* GetVariableWatch() const;

  //! Determine wether this is a local or global build.
  bool GetLocal() const;

  ///! Display progress or status message.
  void DisplayStatus(const char*, float);
  
  /**
   * Expand the given list file arguments into the full set after
   * variable replacement and list expansion.
   */
  void ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                       std::vector<std::string>& outArgs);
  /**
   * Get the instance
   */ 
  cmake *GetCMakeInstance() const;

  /**
   * Get all the source files this makefile knows about
   */
  const std::vector<cmSourceFile*> &GetSourceFiles() const 
    {return m_SourceFiles;}
  std::vector<cmSourceFile*> &GetSourceFiles() {return m_SourceFiles;}

  /**
   * Is there a source file that has the provided source file as an output?
   * if so then return it
   */
  cmSourceFile *GetSourceFileWithOutput(const char *outName);

  /**
   * Add a macro to the list of macros. The arguments should be name of the
   * macro and a documentation signature of it 
   */
  void AddMacro(const char* name, const char* signature);

  /**
   * Get a list of macros as a ; separated string
   */
  void GetListOfMacros(std::string& macros);

  /**
   * Return a location of a file in cmake or custom modules directory
   */
  std::string GetModulesFile(const char* name);

  ///! Return true if the directory is preorder.
  bool IsDirectoryPreOrder(const char* dir);
  
  ///! Set/Get a property of this directory 
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;

protected:
  // add link libraries and directories to the target
  void AddGlobalLinkInformation(const char* name, cmTarget& target);
  
  std::string m_Prefix;
  std::vector<std::string> m_AuxSourceDirectories; // 

  std::string m_cmCurrentDirectory; 
  std::string m_CurrentOutputDirectory; 
  std::string m_cmStartDirectory; 
  std::string m_StartOutputDirectory; 
  std::string m_cmHomeDirectory; 
  std::string m_HomeOutputDirectory;
  std::string m_cmCurrentListFile;

  std::string m_ProjectName;    // project name
  std::vector<std::string> m_ParentProjects;

  // libraries, classes, and executables
  cmTargets m_Targets;
  std::vector<cmSourceFile*> m_SourceFiles;

  std::vector<std::pair<cmStdString, bool> > m_SubDirectories; // list of sub directories
  struct StringSet : public std::set<cmStdString>
  {
  };
  
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
  typedef std::map<cmStdString, cmStdString> DefinitionMap;
  DefinitionMap m_Definitions;
  std::vector<cmCommand*> m_UsedCommands;
  cmLocalGenerator* m_LocalGenerator;
  bool IsFunctionBlocked(const cmListFileFunction& lff);
  
private:
  /**
   * Get the name of the parent directories CMakeLists file
   * given a current CMakeLists file name
   */
  std::string GetParentListFileName(const char *listFileName);

  void ReadSources(std::ifstream& fin, bool t);
  friend class cmMakeDepend;    // make depend needs direct access 
                                // to the m_Sources array 
  void PrintStringVector(const char* s, const std::vector<std::pair<cmStdString, bool> >& v) const;
  void PrintStringVector(const char* s, const std::vector<std::string>& v) const;
  void AddDefaultDefinitions();
  std::list<cmFunctionBlocker *> m_FunctionBlockers;

  typedef std::map<cmStdString, cmData*> DataMap;
  DataMap m_DataMap;
  bool m_Inheriting;

  typedef std::map<cmStdString, cmStdString> StringStringMap;
  StringStringMap m_MacrosMap;

  std::map<cmStdString, bool> m_SubDirectoryOrder;
  // used in AddDefinition for performance improvement
  DefinitionMap::key_type  m_TemporaryDefinitionKey;

  cmsys::RegularExpression m_cmDefineRegex;

  std::map<cmStdString,cmStdString> m_Properties;
};


#endif
