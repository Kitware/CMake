/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmMakefile_h
#define cmMakefile_h

#include "cmStandardIncludes.h"
#include "cmClassFile.h"
#include "cmSystemTools.h"

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
  bool ReadListFile(const char* listfile); 

  /**
   * Add a wrapper generator.
   */
  void AddCommand(cmCommand* );

  /**
   * Specify the makefile generator. This is platform/compiler
   * dependent, although the interface is through a generic
   * superclass.
   */
  void SetMakefileGenerator(cmMakefileGenerator*);

  /**
   * Produce the output makefile.
   */
  void GenerateMakefile();
  
  /**
   * Print the object state to std::cout.
   */
  void Print();
  
  /**
   * Add a custom command to the build.
   */
  void AddCustomCommand(const char* source,
                     const char* result,
                     const char* command,
                     std::vector<std::string>& depends);
  /**
   * Add a define flag to the build.
   */
  void AddDefineFlag(const char* definition);

  /**
   * Add an executable to the build.
   */
  void AddExecutable(cmClassFile&);

  /**
   * Add a link library to the build.
   */
  void AddLinkLibrary(const char*);

  /**
   * Add a link directory to the build.
   */
  void AddLinkDirectory(const char*);

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const char*);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectory(const char*);

  /**
   * Add a variable definition to the build. This variable
   * can be used in CMake to refer to lists, directories, etc.
   */
  void AddDefinition(const char* name, const char* value);

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
  void SetLibraryName(const char*);

  /**
   * Add a class/source file to the build.
   */
  void AddClass(cmClassFile& );

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
  void SetHomeDirectory(const char* dir) 
    {
    m_cmHomeDirectory = dir;
    cmSystemTools::ConvertToUnixSlashes(m_cmHomeDirectory);
    }
  const char* GetHomeDirectory() 
    {
    return m_cmHomeDirectory.c_str();
    }
  void SetHomeOutputDirectory(const char* lib)
    {
    m_HomeOutputDirectory = lib;
    cmSystemTools::ConvertToUnixSlashes(m_HomeOutputDirectory);
    }
  const char* GetHomeOutputDirectory()
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
  const char* GetStartDirectory() 
    {
      return m_cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      m_StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_StartOutputDirectory);
    }
  const char* GetStartOutputDirectory()
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
  const char* GetCurrentDirectory() 
    {
      return m_cmCurrentDirectory.c_str();
    }
  void SetCurrentOutputDirectory(const char* lib)
    {
      m_CurrentOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_CurrentOutputDirectory);
    }
  const char* GetCurrentOutputDirectory()
    {
      return m_CurrentOutputDirectory.c_str();
    }
  //@}
  
  /**
   * Specify the name of the library that is built by this makefile.
   */
  const char* GetLibraryName()
    {
    return m_LibraryName.c_str();
    }

  /**
   * Get a list of the build subdirectories.
   */
  const std::vector<std::string>& GetSubDirectories()
    { 
    return m_SubDirectories;
    }

  /**
   * Return a boolean flag indicating whether the build generates
   * any executables.
   */
  bool HasExecutables();

  /**
   * Get a list of include directories in the build.
   */
  std::vector<std::string>& GetIncludeDirectories()
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
   * Get a list of link libraries in the build.
   */
  std::vector<std::string>& GetLinkLibraries()
    { 
    return m_LinkLibraries;
    }

  /**
   * Get a list of Win32 link libraries in the build.
   */
  std::vector<std::string>& GetLinkLibrariesWin32()
    { 
    return m_LinkLibrariesWin32;
    }
  
  /**
   * Get a list of Unix link libraries in the build.
   */
  std::vector<std::string>& GetLinkLibrariesUnix()
    { 
    return m_LinkLibrariesUnix;
    }

  /**
   * Return a list of source files in this makefile.
   */
  std::vector<cmClassFile>& GetClasses()
    {return  m_Classes;}

  /**
   * Obtain a list of auxiliary source directories.
   */
  std::vector<std::string>& GetAuxSourceDirectories()
    {return m_AuxSourceDirectories;}

  /**
   * Do not use this.
   */
  std::vector<std::string>& GetMakeVerbatim() 
    {return m_MakeVerbatim;}

  /**
   * Given a variable name, return its value (as a string).
   */
  const char* GetDefinition(const char*);

  /**
   * Get a list of preprocessor define flags.
   */
  const char* GetDefineFlags()
    {return m_DefineFlags.c_str();}

  /**
   * Dump documentation to a file. If 0 is returned, the
   * operation failed.
   */
  int DumpDocumentationToFile(const char *fileName);

  /**
   * Expand all defined varibles in the string.  
   * Defined varibles come from the m_Definitions map.
   * They are expanded with ${var} where var is the
   * entry in the m_Definitions map.  Also @var@ is
   * expanded to match autoconf style expansions.
   */
  void ExpandVariablesInString(std::string& source);

  /**
   * Expand variables in the makefiles ivars such as link directories etc
   */
  void ExpandVariables();

  struct customCommand
  {
    std::string m_Source;
    std::string m_Result;
    std::string m_Command;
    std::vector<std::string> m_Depends;
  };

  std::vector<customCommand>& GetCustomCommands() {
    return m_CustomCommands; };
  
      
  /** Recursivly read and create a cmMakefile object for
   *  all CMakeLists.txt files in the GetSubDirectories list.
   *  Once the file is found, it ReadListFile is called on
   *  the cmMakefile created for it.
   */
  void FindSubDirectoryCMakeListsFiles(std::vector<cmMakefile*>& makefiles);
  
  /** Generate the cache file only.  This is done
   *  by calling FindSubDirectoryCMakeListsFiles which
   *  will cause all the rules to fire, and the cache to
   *  be filled.
   */
  void GenerateCacheOnly();

protected:
  std::string m_Prefix;
  std::vector<std::string> m_AuxSourceDirectories; // 

  std::string m_cmCurrentDirectory; 
  std::string m_CurrentOutputDirectory; 
  std::string m_cmStartDirectory; 
  std::string m_StartOutputDirectory; 
  std::string m_cmHomeDirectory; 
  std::string m_HomeOutputDirectory;

  std::string m_LibraryName;	// library name
  std::string m_ProjectName;	// project name
  std::vector<cmClassFile> m_Classes; // list of classes in makefile
  std::vector<std::string> m_SubDirectories; // list of sub directories
  std::vector<std::string> m_MakeVerbatim; // lines copied from input file
  std::vector<std::string> m_IncludeDirectories;
  std::vector<std::string> m_LinkDirectories;
  std::vector<std::string> m_LinkLibraries;
  std::vector<std::string> m_LinkLibrariesWin32;
  std::vector<std::string> m_LinkLibrariesUnix;
  std::string m_DefineFlags;
  std::vector<customCommand> m_CustomCommands;
  typedef std::map<std::string, cmCommand*> RegisteredCommandsMap;
  typedef std::map<std::string, std::string> DefinitionMap;
  DefinitionMap m_Definitions;
  RegisteredCommandsMap m_Commands;
  std::vector<cmCommand*> m_UsedCommands;
  cmMakefileGenerator* m_MakefileGenerator;
  
private:
  /**
   * Get the name of the parent directories CMakeLists file
   * given a current CMakeLists file name
   */
  std::string GetParentListFileName(const char *listFileName);

  void ReadClasses(std::ifstream& fin, bool t);
  friend class cmMakeDepend;	// make depend needs direct access 
				// to the m_Classes array 
  void PrintStringVector(const char* s, std::vector<std::string>& v);
  void AddDefaultCommands();
  
};


#endif
