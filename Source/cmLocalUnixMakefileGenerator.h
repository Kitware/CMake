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
#ifndef cmLocalUnixMakefileGenerator_h
#define cmLocalUnixMakefileGenerator_h

#include "cmLocalGenerator.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalUnixMakefileGenerator();

  virtual ~cmLocalUnixMakefileGenerator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool fromTheTop);

  /**
   * Output the depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  virtual bool OutputObjectDepends(std::ostream&);

  /**
   * Output the check depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  virtual void OutputCheckDepends(std::ostream&);

  /**
   * Set to true if the shell being used is the windows shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetWindowsShell(bool v)  {m_WindowsShell = v;}

  ///! Set the string used to include one makefile into another default is include.
  void SetIncludeDirective(const char* s) { m_IncludeDirective = s; }

  ///! Set the flag used to keep the make program silent.
  void SetMakeSilentFlag(const char* s) { m_MakeSilentFlag = s; }

  ///! Set max makefile variable size, default is 0 which means unlimited.
  void SetMakefileVariableSize(int s) { m_MakefileVariableSize = s; }

  ///! If ignore lib prefix is true, then do not strip lib from the name of a library.
  void SetIgnoreLibPrefix(bool s) { m_IgnoreLibPrefix = s; }

  /**
   * If true, then explicitly pass MAKEFLAGS on the make all target for makes
   * that do not use environment variables.
   *
   */
  void SetPassMakeflags(bool s){m_PassMakeflags = s;}
  
protected:
  void AddDependenciesToSourceFile(cmDependInformation const*info,
                                   cmSourceFile *i,
                                   std::set<cmDependInformation const*> *visited);
  virtual const char* GetSafeDefinition(const char*);
  virtual void ProcessDepends(const cmMakeDepend &md);
  virtual void OutputMakefile(const char* file, bool withDepends);
  virtual void OutputTargetRules(std::ostream& fout);
  virtual void OutputLinkLibraries(std::ostream&, const char* name, const cmTarget &);
  void OutputLibraryRule(std::ostream& fout,  
                         const char* name, 
                         const cmTarget &t,
                         const char* createRule,
                         const char* comment,
                         const char* linkFlags
    );
  void ExpandRuleVariables(std::string& string,
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
  virtual void OutputSharedLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputModuleLibraryRule(std::ostream&, const char* name, 
                                       const cmTarget &);
  virtual void OutputStaticLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputExecutableRule(std::ostream&, const char* name,
                                    const cmTarget &);
  virtual void OutputUtilityRule(std::ostream&, const char* name,
                                 const cmTarget &);
  
  virtual void OutputTargets(std::ostream&);
  virtual void OutputSubDirectoryRules(std::ostream&);
  virtual void OutputDependLibs(std::ostream&);
  virtual void OutputLibDepend(std::ostream&, const char*);
  virtual void OutputExeDepend(std::ostream&, const char*);
  virtual void OutputCustomRules(std::ostream&);
  virtual void OutputMakeVariables(std::ostream&);
  virtual void OutputMakeRules(std::ostream&);
  virtual void OutputInstallRules(std::ostream&);
  virtual void OutputSourceObjectBuildRules(std::ostream& fout);
  virtual void OutputBuildObjectFromSource(std::ostream& fout,
                                           const char* shortName,
                                           const cmSourceFile& source,
                                           const char* extraCompileFlags,
                                           bool sharedTarget);
  
  virtual void BuildInSubDirectory(std::ostream& fout,
                                   const char* directory,
                                   const char* target1,
                                   const char* target2,
                                   bool silent = false);

  virtual void BuildInSubDirectoryWindows(std::ostream& fout,
                                          const char* directory,
                                          const char* target1,
                                          const char* target2,
                                          bool silent = false);
  
  virtual void OutputSubDirectoryVars(std::ostream& fout,
                                      const char* var,
                                      const char* target,
                                      const char* target1,
                                      const char* target2,
                                      const char* depend,
                                      const std::vector<std::string>&
                                      SubDirectories,
                                      bool silent = false);

  virtual void OutputMakeRule(std::ostream&, 
                              const char* comment,
                              const char* target,
                              const char* depends, 
                              const std::vector<std::string>& commands);

  virtual void OutputMakeRule(std::ostream&, 
                              const char* comment,
                              const char* target,
                              const std::vector<std::string>& depends,
                              const char* command);

  virtual void OutputMakeRule(std::ostream&, 
                              const char* comment,
                              const char* target,
                              const std::vector<std::string>& depends,
                              const std::vector<std::string>& commands);

  virtual void OutputMakeRule(std::ostream&, 
                              const char* comment,
                              const char* target,
                              const char* depends, 
                              const char* command,
                              const char* command2 = 0,
                              const char* command3 = 0,
                              const char* command4 = 0);
  virtual void OutputBuildTargetInDirWindows(std::ostream& fout,
                                      const char* path,
                                      const char* library,
                                      const char* fullpath);
  virtual void OutputBuildTargetInDir(std::ostream& fout,
                                      const char* path,
                                      const char* library,
                                      const char* fullpath);
  ///! return true if the two paths are the same
  virtual bool SamePath(const char* path1, const char* path2);
  virtual std::string GetOutputExtension(const char* sourceExtension);
  std::string CreatePreBuildRules(const cmTarget &target,
                                  const char* targetName);
  std::string CreatePreLinkRules(const cmTarget &target,
                                 const char* targetName);
  std::string CreatePostBuildRules(const cmTarget &target,
                                   const char* targetName);
  virtual std::string CreateMakeVariable(const char* s, const char* s2);
  
  ///! if the OS is case insensitive then return a lower case of the path.
  virtual std::string LowerCasePath(const char* path);

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const char*);
  
  /** Get the full name of the target's file, without path.  */
  std::string GetFullTargetName(const char* n, const cmTarget& t);

  /** Get the base name of the target's file, without path or extension.  */
  std::string GetBaseTargetName(const char* n, const cmTarget& t);

  /** Get the names associated with a library target.  */
  void GetLibraryNames(const char* n, const cmTarget& t,
                       std::string& name,
                       std::string& soName,
                       std::string& realName,
                       std::string& baseName);

  /** Output an echo command to the Makefile */
  void OutputEcho(std::ostream& fout, const char *msg);

  ///! final processing for a path to be put in a makefile
protected:
  int m_MakefileVariableSize;
  std::map<cmStdString, cmStdString> m_MakeVariableMap;
  std::map<cmStdString, cmStdString> m_ShortMakeVariableMap;
  bool m_IgnoreLibPrefix;
  std::string m_IncludeDirective;
  std::string m_MakeSilentFlag;
  std::string m_ExecutableOutputPath;
  std::string m_LibraryOutputPath;
  bool m_WindowsShell;
  bool m_PassMakeflags;
private:
};

#endif
