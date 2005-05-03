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

#include "cmLocalGenerator.h"

#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 1375 /* base class destructor not virtual */
#endif

class cmCustomCommand;
class cmDependInformation;
class cmDepends;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator2
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator2 produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator2 : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalUnixMakefileGenerator2();

  virtual ~cmLocalUnixMakefileGenerator2();

  /** Set the command used when there are no dependencies or rules for
      a target.  This is used to avoid errors on some make
      implementations.  */
  void SetEmptyCommand(const char* cmd);

  /** Set whether the echo command needs its argument quoted.  */
  void SetEchoNeedsQuote(bool b) { m_EchoNeedsQuote = b; }

  /**
   * Set to true if the shell being used is the windows shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetWindowsShell(bool v)  {m_WindowsShell = v;}

  /**
   * Set the string used to include one makefile into another default
   * is include.
   */
  void SetIncludeDirective(const char* s) { m_IncludeDirective = s; }

  /**
   * Set the flag used to keep the make program silent.
   */
  void SetMakeSilentFlag(const char* s) { m_MakeSilentFlag = s; }

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { m_MakefileVariableSize = s; }

  /**
   * If ignore lib prefix is true, then do not strip lib from the name
   * of a library.
   */
  void SetIgnoreLibPrefix(bool s) { m_IgnoreLibPrefix = s; }

  /**
   * If true, then explicitly pass MAKEFLAGS on the make all target for makes
   * that do not use environment variables.
   *
   */
  void SetPassMakeflags(bool s){m_PassMakeflags = s;}

  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  /** Called from command-line hook to scan dependencies.  */
  static bool ScanDependencies(std::vector<std::string> const& args);

  /** Called from command-line hook to check dependencies.  */
  static void CheckDependencies(cmMakefile* mf);

protected:

  void GenerateMakefile();
  void GenerateCMakefile();
  void GenerateDirectoryInformationFile();
  void GenerateTargetRuleFile(const cmTarget& target);
  void GenerateObjectRuleFile(const cmTarget& target,
                              const cmSourceFile& source,
                              std::vector<std::string>& objects,
                              std::vector<std::string>& provides_requires);
  void GenerateCustomRuleFile(const cmCustomCommand& cc);
  void GenerateUtilityRuleFile(const cmTarget& target);
  bool GenerateDependsMakeFile(const std::string& lang,
                               const char* objFile,
                               std::string& depMakeFile,
                               std::string& depMarkFile);
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const char* target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands);
  void WriteDivider(std::ostream& os);
  void WriteDisclaimer(std::ostream& os);
  void WriteMakeVariables(std::ostream& makefileStream);
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);
  void WriteRuleFileIncludes(std::ostream& makefileStream);
  void WriteAllRules(std::ostream& makefileStream);
  void WritePassRules(std::ostream& makefileStream, const char* pass);
  void WriteDriverRules(std::ostream& makefileStream, const char* pass,
                        const char* local1, const char* local2=0);
  void WriteSubdirRules(std::ostream& makefileStream, const char* pass);
  void WriteSubdirRule(std::ostream& makefileStream, const char* pass,
                       const char* subdir, std::string& last);
  void WriteSubdirDriverRule(std::ostream& makefileStream, const char* pass,
                             const char* order, const std::string& last);
  void WriteLocalRule(std::ostream& ruleFileStream, const char* pass,
                      const char* dependency);
  void WriteConvenienceRules(std::ostream& ruleFileStream,
                             const cmTarget& target,
                             const char* targetOutPath);
  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const char* realTarget,
                            const char* helpTarget);
  void WriteExecutableRule(std::ostream& ruleFileStream,
                           const char* ruleFileName,
                           const cmTarget& target,
                           const std::vector<std::string>& objects,
                           const std::vector<std::string>& external_objects,
                           const std::vector<std::string>& provides_requires);
  void WriteStaticLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              const std::vector<std::string>& provides_requires);
  void WriteSharedLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              const std::vector<std::string>& provides_requires);
  void WriteModuleLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              const std::vector<std::string>& provides_requires);
  void WriteLibraryRule(std::ostream& ruleFileStream,
                        const char* ruleFileName,
                        const cmTarget& target,
                        const std::vector<std::string>& objects,
                        const std::vector<std::string>& external_objects,
                        const char* linkRuleVar,
                        const char* extraLinkFlags,
                        const std::vector<std::string>& provides_requires);
  void WriteObjectsVariable(std::ostream& ruleFileStream,
                            const cmTarget& target,
                            const std::vector<std::string>& objects,
                            const std::vector<std::string>& external_objects,
                            std::string& variableName,
                            std::string& variableNameExternal);
  void WriteTargetDependsRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              const cmTarget& target,
                              const std::vector<std::string>& objects);
  void WriteTargetCleanRule(std::ostream& ruleFileStream,
                            const cmTarget& target,
                            const std::vector<std::string>& files);
  void WriteTargetRequiresRule(std::ostream& ruleFileStream,
                               const cmTarget& target,
                               const std::vector<std::string>& provides_requires);
  void WriteLocalCleanRule(std::ostream& makefileStream);
  void WriteCMakeArgument(std::ostream& os, const char* s);
  std::string GetTargetDirectory(const cmTarget& target);
  std::string GetSubdirTargetName(const char* pass, const char* subdir);
  std::string GetObjectFileName(const cmTarget& target,
                                const cmSourceFile& source);
  const char* GetSourceFileLanguage(const cmSourceFile& source);
  std::string ConvertToFullPath(const std::string& localPath);
  std::string ConvertToRelativeOutputPath(const char* p);
  std::string ConvertToQuotedOutputPath(const char* p);
  void ConfigureOutputPaths();
  void FormatOutputPath(std::string& path, const char* name);

  void AppendTargetDepends(std::vector<std::string>& depends,
                           const cmTarget& target);
  void AppendAnyDepend(std::vector<std::string>& depends, const char* name,
                       bool assume_unknown_is_file=false);
  void AppendRuleDepend(std::vector<std::string>& depends,
                        const char* ruleFileName);
  void AppendCustomDepends(std::vector<std::string>& depends,
                           const std::vector<cmCustomCommand>& ccs);
  void AppendCustomDepend(std::vector<std::string>& depends,
                          const cmCustomCommand& cc);
  void AppendCustomCommands(std::vector<std::string>& commands,
                            const std::vector<cmCustomCommand>& ccs);
  void AppendCustomCommand(std::vector<std::string>& commands,
                           const cmCustomCommand& cc);
  void AppendCleanCommand(std::vector<std::string>& commands,
                          const std::vector<std::string>& files);
  void AppendEcho(std::vector<std::string>& commands,
                  const char* text);

  //==========================================================================
  bool SamePath(const char* path1, const char* path2);
  std::string ConvertToMakeTarget(const char* tgt);
  std::string& CreateSafeUniqueObjectFileName(const char* sin);
  std::string CreateMakeVariable(const char* sin, const char* s2in);
  //==========================================================================

  std::string GetRecursiveMakeCall(const char* tgt);
  void WriteJumpAndBuildRules(std::ostream& makefileStream);

  static cmDepends* GetDependsChecker(const std::string& lang,
                                      const char* dir,
                                      const char* objFile);

private:
  // Map from target name to build directory containing it for
  // jump-and-build targets.
  struct RemoteTarget
  {
    std::string m_BuildDirectory;
    std::string m_FilePath;
  };
  std::map<cmStdString, RemoteTarget> m_JumpAndBuild;

  // List the files for which to check dependency integrity.  Each
  // language has its own list because integrity may be checked
  // differently.
  struct IntegrityCheckSet: public std::set<cmStdString> {};
  std::map<cmStdString, IntegrityCheckSet> m_CheckDependFiles;

  // Command used when a rule has no dependencies or commands.
  std::vector<std::string> m_EmptyCommands;

  //==========================================================================
  // Configuration settings.
  int m_MakefileVariableSize;
  std::map<cmStdString, cmStdString> m_MakeVariableMap;
  std::map<cmStdString, cmStdString> m_ShortMakeVariableMap;
  std::map<cmStdString, cmStdString> m_UniqueObjectNamesMap;
  std::string m_IncludeDirective;
  std::string m_MakeSilentFlag;
  std::string m_ExecutableOutputPath;
  std::string m_LibraryOutputPath;
  bool m_PassMakeflags;
  //==========================================================================

  // Flag for whether echo command needs quotes.
  bool m_EchoNeedsQuote;

  // List of make rule files that need to be included by the makefile.
  std::vector<std::string> m_IncludeRuleFiles;

  // Set of custom rule files that have been generated.
  std::set<cmStdString> m_CustomRuleFiles;

  // Set of object file names that will be built in this directory.
  std::set<cmStdString> m_ObjectFiles;
};

#if defined(__sgi) && !defined(__GNUC__)
# pragma reset woff 1375 /* base class destructor not virtual */
#endif

#endif
