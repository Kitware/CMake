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
#ifndef cmLocalUnixMakefileGenerator3_h
#define cmLocalUnixMakefileGenerator3_h

#include "cmLocalGenerator.h"

class cmCustomCommand;
class cmDependInformation;
class cmDepends;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator3
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator3 produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator3 : public cmLocalGenerator
{
public:
  cmLocalUnixMakefileGenerator3();
  virtual ~cmLocalUnixMakefileGenerator3();

  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  /**
   * Process the CMakeLists files for this directory to fill in the
   * m_Makefile ivar 
   */
  virtual void Configure();

  /** creates the common disclainer text at the top of each makefile */
  void WriteDisclaimer(std::ostream& os);

  // this returns the relative path between the HomeOutputDirectory and this
  // local generators StartOutputDirectory
  const std::string &GetHomeRelativeOutputPath();

  // Write out a make rule 
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const char* target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands);
  
  // write the main variables used by the makefiles
  void WriteMakeVariables(std::ostream& makefileStream);

  // write a  comment line #====... in the stream
  void WriteDivider(std::ostream& os);

  /**
   * If true, then explicitly pass MAKEFLAGS on the make all target for makes
   * that do not use environment variables.
   *
   */
  void SetPassMakeflags(bool s){m_PassMakeflags = s;}
  bool GetPassMakeflags() { return m_PassMakeflags; }
  
  /**
   * Set the flag used to keep the make program silent.
   */
  void SetMakeSilentFlag(const char* s) { m_MakeSilentFlag = s; }
  std::string &GetMakeSilentFlag() { return m_MakeSilentFlag; }

  /** used to create a recursive make call */
  std::string GetRecursiveMakeCall(const char *makefile, const char* tgt);

  
  
  
  
  
  
  /** Set whether the echo command needs its argument quoted.  */
  void SetEchoNeedsQuote(bool b) { m_EchoNeedsQuote = b; }

  /**
   * Set to true if the shell being used is the windows shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetWindowsShell(bool v)  {m_WindowsShell = v;}

  /**
   * If set to true, then NULL is set to nil for non Windows_NT.
   * This uses make syntax used by nmake and borland.
   * The default is false.
   */
  void SetDefineWindowsNULL(bool v)  {m_DefineWindowsNULL = v;}

  /**
   * If set to true, cd dir && command is used to 
   * run commands in a different directory.
   */
  void SetUnixCD(bool v)  {m_UnixCD = v;}

  /**
   * Set Support Verbose Variable.  If true, then .SILENT will
   * be not end with :  i.e. .SILENT: or .SILENT
   */
  void SetSilentNoColon(bool v)  {m_SilentNoColon = v;}

  /**
   * Set the string used to include one makefile into another default
   * is include.
   */
  void SetIncludeDirective(const char* s) { m_IncludeDirective = s; }
  const char *GetIncludeDirective() { return m_IncludeDirective.c_str(); }

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { m_MakefileVariableSize = s; }

  /**
   * If ignore lib prefix is true, then do not strip lib from the name
   * of a library.
   */
  void SetIgnoreLibPrefix(bool s) { m_IgnoreLibPrefix = s; }

    
  
  
  
  /** Called from command-line hook to scan dependencies.  */
  virtual bool ScanDependencies(std::vector<std::string> const& args);

  /** Called from command-line hook to check dependencies.  */
  virtual void CheckDependencies(cmMakefile* mf, bool verbose,
                                 bool clear);
  
  /** write some extra rules suahc as make test etc */
  void WriteSpecialTargetsTop(std::ostream& makefileStream);

  void WriteSpecialTargetsBottom(std::ostream& makefileStream);
  std::string GetRelativeTargetDirectory(cmTarget& target);

  // List the files for which to check dependency integrity.  Each
  // language has its own list because integrity may be checked
  // differently.
  struct IntegrityCheckSet: public std::set<cmSourceFile *> {};
  struct IntegrityCheckSetMap: public std::map<cmStdString, IntegrityCheckSet> {};
  std::map<cmStdString, IntegrityCheckSetMap> &GetIntegrityCheckSet() 
  { return m_CheckDependFiles;}
  
  void AppendTargetDepends(std::vector<std::string>& depends,
                           cmTarget& target);

  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);

  void AppendEcho(std::vector<std::string>& commands,
                  const char* text);

  // write the target rules for the local Makefile into the stream
  void WriteLocalAllRules(std::ostream& ruleFileStream);
  
  std::map<cmStdString,std::vector<cmTarget *> > GetLocalObjectFiles()
    { return m_LocalObjectFiles;}
protected:
  // Return the a string with -F flags on apple
  std::string GetFrameworkFlags(cmTarget&);
  
  // write the depend info 
  void WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &tgt);
  
  // write the target rules for the local Makefile into the stream
  void WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                                 std::set<cmStdString> &emitted);

  // write the local help rule
  void WriteHelpRule(std::ostream& ruleFileStream);
  
  // create a command that cds to the start dir then runs the commands
  void CreateCDCommand(std::vector<std::string>& commands, 
                       const char *targetDir, const char *returnDir);

  // these two methods just compute reasonable values for m_LibraryOutputPath
  // and m_ExecutableOutputPath
  void ConfigureOutputPaths();
  void FormatOutputPath(std::string& path, const char* name);

  // this converts a file name that is relative to the StartOuputDirectory
  // into a full path
  std::string ConvertToFullPath(const std::string& localPath);

  // this is responsible for writing all of the rules for all this
  // directories custom commands (but not utility targets)
  void WriteCustomCommands(std::ostream& os,
                           std::vector<std::string>& cleanFiles);
  
  // this method Writes the Directory informaiton files
  void WriteDirectoryInformationFile();

  // cleanup the name of a potential target
  std::string ConvertToMakeTarget(const char* tgt);

  // used in writing out Cmake files such as WriteDirectoryInformation
  void WriteCMakeArgument(std::ostream& os, const char* s);

  // write out all the rules for this target
  void WriteTargetRuleFiles(cmTarget& target);
  void WriteUtilityRuleFiles(cmTarget& target);
  
  // create the rule files for an object
  void WriteObjectRuleFiles(cmTarget& target,
                            cmSourceFile& source,
                            std::vector<std::string>& objects,
                            std::ostream &filestr,
                            std::ostream &flagstr);

  // write the build rule for an object
  void WriteObjectBuildFile(std::string &obj,
                            const char *lang, 
                            cmTarget& target, 
                            cmSourceFile& source,
                            std::vector<std::string>& depends,
                            std::ostream &filestr,
                            std::ostream &flagstr);
  
  // write the depend.make file for an object
  void WriteObjectDependRules(cmSourceFile& source,
                              std::vector<std::string>& depends);
  
  // this is used only by WriteObjectDependFile
  bool GenerateDependsMakeFile(const std::string& lang,
                               const char* objFile);

  // return the appropriate depends checker
  cmDepends* GetDependsChecker(const std::string& lang,
                               bool verbose);
  
  
  void GenerateCustomRuleFile(const cmCustomCommand& cc, 
                              std::ostream &ruleStream);
  
  // these three make some simple changes and then call WriteLibraryRule
  void WriteStaticLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              std::vector<std::string>& cleanFiles);
  
  void WriteSharedLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              std::vector<std::string>& cleanFiles);
  
  void WriteModuleLibraryRule(std::ostream& ruleFileStream,
                              const char* ruleFileName,
                              cmTarget& target,
                              const std::vector<std::string>& objects,
                              const std::vector<std::string>& external_objects,
                              std::vector<std::string>& cleanFiles);

  // the main code for writing the Executable target rules
  void WriteExecutableRule(std::ostream& ruleFileStream,
                           const char* ruleFileName,
                           cmTarget& target,
                           const std::vector<std::string>& objects,
                           const std::vector<std::string>& external_objects,
                           std::vector<std::string>& cleanFiles);

  // the main method for writing library rules
  void WriteLibraryRule(std::ostream& ruleFileStream,
                        const char* ruleFileName,
                        cmTarget& target,
                        const std::vector<std::string>& objects,
                        const std::vector<std::string>& external_objects,
                        const char* linkRuleVar,
                        const char* extraLinkFlags,
                        std::vector<std::string>& cleanFiles);
  
  void WriteLocalMakefile();
  
  
  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const char* realTarget,
                            const char* helpTarget);
  void WriteObjectsVariable(std::ostream& ruleFileStream,
                            cmTarget& target,
                            const std::vector<std::string>& objects,
                            const std::vector<std::string>& external_objects,
                            std::string& variableName,
                            std::string& variableNameExternal);
  void WriteTargetDependRule(std::ostream& ruleFileStream,
                             cmTarget& target);
  void WriteTargetCleanRule(std::ostream& ruleFileStream,
                            cmTarget& target,
                            const std::vector<std::string>& files);
  void WriteTargetRequiresRule(std::ostream& ruleFileStream,
                               cmTarget& target,
                               const std::vector<std::string>& objects);
  
  std::string GetTargetDirectory(cmTarget& target);
  std::string GetObjectFileName(cmTarget& target,
                                const cmSourceFile& source,
                                std::string* nameWithoutTargetDir = 0);
  const char* GetSourceFileLanguage(const cmSourceFile& source);
  std::string ConvertToQuotedOutputPath(const char* p);

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

  //==========================================================================
  bool SamePath(const char* path1, const char* path2);
  std::string& CreateSafeUniqueObjectFileName(const char* sin);
  std::string CreateMakeVariable(const char* sin, const char* s2in);
  //==========================================================================

  void ComputeHomeRelativeOutputPath();

private:
  std::map<cmStdString, IntegrityCheckSetMap> m_CheckDependFiles;

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
  std::string m_ConfigurationName;
  bool m_DefineWindowsNULL;
  bool m_UnixCD;
  bool m_PassMakeflags;
  bool m_SilentNoColon;
  //==========================================================================

  // Flag for whether echo command needs quotes.
  bool m_EchoNeedsQuote;

  std::string m_HomeRelativeOutputPath;
  
  // Set of object file names that will be built in this directory.
  std::set<cmStdString> m_ObjectFiles;

  std::map<cmStdString,std::vector<cmTarget *> > m_LocalObjectFiles;
};

#endif
