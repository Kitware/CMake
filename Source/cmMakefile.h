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
  bool ReadMakefile(const char* makefile, bool inheriting = false); 

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
   * Specify the home directory for the build.
   */
  void SetHomeDirectory(const char* dir) 
    {
    m_cmHomeDirectory = dir;
    cmSystemTools::ConvertToUnixSlashes(m_cmHomeDirectory);
    }

  /**
   * Get the home directory for the build.
   */
  const char* GetHomeDirectory() 
    {
    return m_cmHomeDirectory.c_str();
    }

  /**
   * Set the current directory in the project.
   */
  void SetCurrentDirectory(const char* dir) 
    {
    m_cmCurrentDirectory = dir;
    }

  /**
   * Get the current directory in the project.
   */
  const char* GetCurrentDirectory() 
    {
    return m_cmCurrentDirectory.c_str();
    }

  /**
   * Specify the name of the library that is built by this makefile.
   */
  const char* GetLibraryName()
    {
    return m_LibraryName.c_str();
    }

  /**
   * Set the name of the library that is built by this makefile.
   */
  void SetOutputDirectory(const char* lib)
    {
    m_OutputDirectory = lib;
    }

  /**
   * Get the name of the library that is built by this makefile.
   */
  const char* GetOutputDirectory()
    {
    return m_OutputDirectory.c_str();
    }

  /**
   * Set the name of the current output directory.
   */
  void SetOutputHomeDirectory(const char* lib)
    {
    m_OutputHomeDirectory = lib;
    }

  /**
   * Get the name of the current output directory.
   */
  const char* GetOutputHomeDirectory()
    {
    return m_OutputHomeDirectory.c_str();
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
  bool HasExecutables() 
    {
    return m_Executables;
    }

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

protected:
  bool m_Executables;
  std::string m_Prefix;
  std::vector<std::string> m_AuxSourceDirectories; // 
  std::string m_OutputDirectory; // Current output directory for makefile
  std::string m_OutputHomeDirectory; // Top level output directory
  std::string m_cmHomeDirectory; // Home directory for source
  std::string m_cmCurrentDirectory; // current directory in source
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
  std::string m_SourceHomeDirectory;
  struct customCommand
  {
    std::string m_Source;
    std::string m_Result;
    std::string m_Command;
    std::vector<std::string> m_Depends;
  };
  std::vector<customCommand> m_CustomCommands;
  typedef std::map<std::string, cmCommand*> RegisteredCommandsMap;
  typedef std::map<std::string, std::string> DefinitionMap;
  DefinitionMap m_Definitions;
  RegisteredCommandsMap m_Commands;
  std::vector<cmCommand*> m_UsedCommands;
  cmMakefileGenerator* m_MakefileGenerator;
  
private:
   /**
   * Look for CMakeLists.txt files to parse in dir,
   * then in dir's parents, until the SourceHome directory
   * is found.
   */
  void ParseDirectory(const char* dir);

  /**
   * Parse a file for includes links and libs
   */
  void ExpandVaribles();

  void ReadClasses(std::ifstream& fin, bool t);
  friend class cmMakeDepend;	// make depend needs direct access 
				// to the m_Classes array 
  void PrintStringVector(const char* s, std::vector<std::string>& v);
  void AddDefaultCommands();
  
};


#endif
