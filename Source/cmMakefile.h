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
/**
 * cmMakefile - used to parse and store the contents of a
 * CMakeLists.txt makefile in memory.
 */
#ifndef cmMakefile_h
#define cmMakefile_h
#include "cmStandardIncludes.h"
#include "cmClassFile.h"
#include "cmSystemTools.h"
class cmRuleMaker;
class cmMakefileGenerator;


class cmMakefile
{
public:
  cmMakefile();
  ~cmMakefile();
  // Parse a CMakeLists.txt file
  bool ReadMakefile(const char* makefile, bool inheriting = false); 
  // Add a wrap generator 
  void AddRuleMaker(cmRuleMaker* );
  // Set the make file 
  void SetMakefileGenerator(cmMakefileGenerator*);
  // Generate the output file
  void GenerateMakefile();
  
  // Print useful stuff to stdout
  void Print();
  
  // cmRuleMaker interfaces
  void AddCustomRule(const char* source,
                     const char* result,
                     const char* command,
                     std::vector<std::string>& depends);
  void AddDefineFlag(const char* definition);
  void AddExecutable(cmClassFile&);
  void AddLinkLibrary(const char*);
  void AddLinkDirectory(const char*);
  void AddSubDirectory(const char*);
  void AddIncludeDirectory(const char*);
  void AddDefinition(const char* name, const char* value);
  void SetProjectName(const char*);
  void SetLibraryName(const char*);
  void AddClass(cmClassFile& );
  void AddExtraDirectory(const char* dir);
  
  // Set the home directory for the project
  void SetHomeDirectory(const char* dir) 
    {
      m_cmHomeDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(m_cmHomeDirectory);
    }
  const char* GetHomeDirectory() 
    {
      return m_cmHomeDirectory.c_str();
    }
  // Set the current directory in the project
  void SetCurrentDirectory(const char* dir) 
    {
      m_cmCurrentDirectory = dir;
    }
  const char* GetCurrentDirectory() 
    {
      return m_cmCurrentDirectory.c_str();
    }
  // Set the name of the library that is built by this makefile
  const char* GetLibraryName()
    {
      return m_LibraryName.c_str();
    }

  const char* GetProjectName()
    {
      return m_ProjectName.c_str();
    }
  
  // Set the name of the library that is built by this makefile
  void SetOutputDirectory(const char* lib)
    {
      m_OutputDirectory = lib;
    }
  const char* GetOutputDirectory()
    {
      return m_OutputDirectory.c_str();
    }
  
  // Set the name of the library that is built by this makefile
  void SetOutputHomeDirectory(const char* lib)
    {
      m_OutputHomeDirectory = lib;
    }
  const char* GetOutputHomeDirectory()
    {
      return m_OutputHomeDirectory.c_str();
    }
  const std::vector<std::string>& GetSubDirectories()
    { 
      return m_SubDirectories;
    }
  
  bool HasExecutables() 
    {
      return m_Executables;
    }
  
  std::vector<std::string>& GetIncludeDirectories()
    { 
      return m_IncludeDirectories;
    }
  
  std::vector<std::string>& GetLinkDirectories()
    { 
      return m_LinkDirectories;
    }
  
  std::vector<std::string>& GetLinkLibraries()
    { 
      return m_LinkLibraries;
    }
  
  std::vector<std::string>& GetLinkLibrariesWin32()
    { 
      return m_LinkLibrariesWin32;
    }
  
  std::vector<std::string>& GetLinkLibrariesUnix()
    { 
      return m_LinkLibrariesUnix;
    }
  std::vector<cmClassFile>& GetClasses(){ return  m_Classes;}
  std::vector<std::string>& GetAuxSourceDirectories()
    { return m_AuxSourceDirectories; }
  std::vector<std::string>& GetMakeVerbatim() 
    { return m_MakeVerbatim;}
  const char* GetDefinition(const char*);

  const char* GetDefineFlags()
    { return m_DefineFlags.c_str();}

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
  void AddDefaultRules();
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
  struct customRule
  {
    std::string m_Source;
    std::string m_Result;
    std::string m_Command;
    std::vector<std::string> m_Depends;
  };
  std::vector<customRule> m_CustomRules;
  typedef std::map<std::string, cmRuleMaker*> StringRuleMakerMap;
  typedef std::map<std::string, std::string> DefinitionMap;
  DefinitionMap m_Definitions;
  StringRuleMakerMap m_RuleMakers;
  std::vector<cmRuleMaker*> m_UsedRuleMakers;
  cmMakefileGenerator* m_MakefileGenerator;
};


#endif
