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
#ifndef cmLocalGenerator_h
#define cmLocalGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmGlobalGenerator;
class cmTarget;
class cmSourceFile;


/** \class cmLocalGenerator
 * \brief Create required build files for a directory.
 *
 * Subclasses of this abstract class generate makefiles, DSP, etc for various
 * platforms. This class should never be constructued directly. A
 * GlobalGenerator will create it and invoke the appropriate commands on it.
 */
class cmLocalGenerator
{
public:
  cmLocalGenerator();
  virtual ~cmLocalGenerator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool /* fromTheTop */) {};

  /**
   * Process the CMakeLists files for this directory to fill in the
   * m_Makefile ivar 
   */
  virtual void Configure();

  /**
   * Perform any final calculations prior to generation
   */
  virtual void ConfigureFinalPass();

  /**
   * Generate the install rules files in this directory.
   */
  virtual void GenerateInstallRules();

  ///! Get the makefile for this generator
  cmMakefile *GetMakefile() {
    return this->m_Makefile; };
  
  ///! Get the GlobalGenerator this is associated with
  cmGlobalGenerator *GetGlobalGenerator() {
    return m_GlobalGenerator; };

  ///! Set the Global Generator, done on creation by the GlobalGenerator
  void SetGlobalGenerator(cmGlobalGenerator *gg);
  
  /** Get the full name of the target's file, without path.  */
  std::string GetFullTargetName(const char* n, const cmTarget& t);

  std::string ConvertToRelativeOutputPath(const char* p);
  
  // flag to determine if this project should be included in a parent project
  bool GetExcludeAll()
    {
      return m_ExcludeFromAll;
    }
  void SetExcludeAll(bool b)
    {
      m_ExcludeFromAll = b;
    }
  
  ///! set/get the parent generator 
  cmLocalGenerator* GetParent(){return m_Parent;}
  void SetParent(cmLocalGenerator* g) { m_Parent = g;}

  void AddLanguageFlags(std::string& flags, const char* lang);
  void AddSharedFlags(std::string& flags, const char* lang, bool shared);
  void AddConfigVariableFlags(std::string& flags, const char* var);
  void AppendFlags(std::string& flags, const char* newFlags);
  ///! Get the include flags for the current makefile and language
  const char* GetIncludeFlags(const char* lang); 

protected:

  ///! Fill out these strings for the given target.  Libraries to link, flags, and linkflags.
  void GetTargetFlags(std::string& linkLibs, 
                      std::string& flags,
                      std::string& linkFlags,
                      cmTarget&target);
  
  ///! put all the libraries for a target on into the given stream
  virtual void OutputLinkLibraries(std::ostream&, const char* name, const cmTarget &);


  /** Get the include flags for the current makefile and language.  */
  void GetIncludeDirectories(std::vector<std::string>& dirs);

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const char* p);
  
  // Expand rule variables in CMake of the type found in language rules
  void ExpandRuleVariables(std::string& string,
                           const char* language,
                           const char* objects=0,
                           const char* target=0,
                           const char* linkLibs=0,
                           const char* source=0,
                           const char* object =0,
                           const char* flags = 0,
                           const char* objectsquoted = 0,
                           const char* targetBase = 0,
                           const char* targetSOName = 0,
                           const char* linkFlags = 0);
  ///! Convert a target to a utility target for unsupported languages of a generator
  void AddBuildTargetRule(const char* llang, cmTarget& target);
  ///! add a custom command to build a .o file that is part of a target 
  void AddCustomCommandToCreateObject(const char* ofname, 
                                      const char* lang, 
                                      cmSourceFile& source,
                                      cmTarget& target);
  // Create Custom Targets and commands for unsupported languages
  // The set passed in should contain the languages supported by the
  // generator directly.  Any targets containing files that are not
  // of the types listed will be compiled as custom commands and added
  // to a custom target.
  void CreateCustomTargetsAndCommands(std::set<cmStdString> const&);
  virtual void AddInstallRule(std::ostream& fout, const char* dest, int type, 
    const char* files, bool optional = false, const char* properties = 0);
  
  bool m_FromTheTop;
  cmMakefile *m_Makefile;
  cmGlobalGenerator *m_GlobalGenerator;
  // members used for relative path function ConvertToMakefilePath
  std::string m_RelativePathToSourceDir;
  std::string m_RelativePathToBinaryDir;
  std::string m_CurrentOutputDirectory;
  std::string m_HomeOutputDirectory;
  std::string m_HomeDirectory;
  std::string m_HomeOutputDirectoryNoSlash;
  bool m_ExcludeFromAll;
  cmLocalGenerator* m_Parent;
  std::map<cmStdString, cmStdString> m_LanguageToIncludeFlags;
  bool m_WindowsShell;
  bool m_UseRelativePaths;
  bool m_IgnoreLibPrefix;
};

#endif
