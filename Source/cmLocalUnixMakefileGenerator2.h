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
#ifndef cmLocalUnixMakefileGenerator2_h
#define cmLocalUnixMakefileGenerator2_h

#include "cmLocalUnixMakefileGenerator.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator2
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator2 produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator2 : public cmLocalUnixMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalUnixMakefileGenerator2();

  virtual ~cmLocalUnixMakefileGenerator2();

  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make.
   */
  virtual void Generate(bool fromTheTop);

  /** Called from command-line hook to scan dependencies.  */
  static bool ScanDependencies(std::vector<std::string> const& args);

  /** Called from command-line hook to check dependencies.  */
  static void CheckDependencies(const char* depCheck);

protected:

  void GenerateMakefile();
  void GenerateCMakefile();
  void GenerateTargetRuleFile(const cmTarget& target);
  void GenerateObjectRuleFile(const cmTarget& target,
                              const cmSourceFile& source);
  std::string GenerateDependsMakeFile(const char* file);
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const char* preEcho,
                     const char* target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands,
                     const char* postEcho=0);
  void WriteDivider(std::ostream& os);
  void WriteDisclaimer(std::ostream& os);
  void WriteMakeVariables(std::ostream& makefileStream);
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);
  void WriteTargetIncludes(std::ostream& makefileStream);
  void WriteAllRule(std::ostream& makefileStream);
  void WriteSubdirRules(std::ostream& makefileStream, const char* pass);
  void WriteSubdirRule(std::ostream& makefileStream, const char* pass,
                       const char* subdir, std::string& last);
  void WriteRequiresRule(std::ostream& ruleFileStream, const cmTarget& target,
                         const char* targetFullPath);
  void WriteExecutableRule(std::ostream& ruleFileStream,
                           const char* ruleFileName,
                           const cmTarget& target,
                           std::vector<std::string>& objects);
  void WriteStaticLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              std::vector<std::string>& objects);
  void WriteSharedLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              std::vector<std::string>& objects);
  void WriteModuleLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              std::vector<std::string>& objects);
  void WriteLibraryRule(std::ostream& ruleFileStream,
                        const char* ruleFileName,
                        const cmTarget& target,
                        std::vector<std::string>& objects,
                        const char* linkRuleVar,
                        const char* extraLinkFlags);

  std::string GetTargetDirectory(const cmTarget& target);
  std::string GetSubdirTargetName(const char* pass, const char* subdir);
  std::string GetObjectFileName(const cmTarget& target,
                                const cmSourceFile& source);
  const char* GetSourceFileLanguage(const cmSourceFile& source);
  std::string ConvertToFullPath(const std::string& localPath);

  void AddLanguageFlags(std::string& flags, const char* lang);
  void AddSharedFlags(std::string& flags, const char* lang, bool shared);
  void AddConfigVariableFlags(std::string& flags, const char* var);
  void AppendFlags(std::string& flags, const char* newFlags);
  void AppendLibDepend(std::vector<std::string>& depends, const char* name);
  std::string GetRecursiveMakeCall(const char* tgt);
  void WriteJumpAndBuildRules(std::ostream& makefileStream);

  static bool ScanDependenciesC(const char* objFile, const char* srcFile,
                                std::vector<std::string> const& includes);
  static void CheckDependencies(const char* dir, const char* file);
  static void WriteEmptyDependMakeFile(const char* file,
                                       const char* depMarkFileFull,
                                       const char* depMakeFileFull);
private:
  // Map from target name to build directory containing it for
  // jump-and-build targets.
  struct RemoteTarget
  {
    std::string m_BuildDirectory;
    std::string m_FilePath;
  };
  std::map<cmStdString, RemoteTarget> m_JumpAndBuild;

  // List the files for which to check dependency integrity.
  std::set<cmStdString> m_CheckDependFiles;
};

#endif
