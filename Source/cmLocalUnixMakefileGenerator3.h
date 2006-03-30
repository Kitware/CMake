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
class cmMakefileTargetGenerator;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator3
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator3 produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalUnixMakefileGenerator3 : public cmLocalGenerator
{
public:
  cmLocalUnixMakefileGenerator3();
  virtual ~cmLocalUnixMakefileGenerator3();

  /**
   * Process the CMakeLists files for this directory to fill in the
   * Makefile ivar 
   */
  virtual void Configure();

  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  
  
  
  
  
  
  
  
  
  
  
  
  
  // this returns the relative path between the HomeOutputDirectory and this
  // local generators StartOutputDirectory
  const std::string &GetHomeRelativeOutputPath();

  // Write out a make rule 
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const char* target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands,
                     bool symbolic);
  
  // write the main variables used by the makefiles
  void WriteMakeVariables(std::ostream& makefileStream);

  /**
   * If true, then explicitly pass MAKEFLAGS on the make all target for makes
   * that do not use environment variables.
   *
   */
  void SetPassMakeflags(bool s){this->PassMakeflags = s;}
  bool GetPassMakeflags() { return this->PassMakeflags; }
  
  /**
   * Set the flag used to keep the make program silent.
   */
  void SetMakeSilentFlag(const char* s) { this->MakeSilentFlag = s; }
  std::string &GetMakeSilentFlag() { return this->MakeSilentFlag; }

  /** Set whether the echo command needs its argument quoted.  */
  void SetEchoNeedsQuote(bool b) { this->EchoNeedsQuote = b; }

  /**
   * Set to true if the shell being used is the windows shell.
   * This controls if statements in the makefile and the SHELL variable.
   * The default is false.
   */
  void SetWindowsShell(bool v)  {this->WindowsShell = v;}

  /**
   * If set to true, then NULL is set to nil for non Windows_NT.
   * This uses make syntax used by nmake and borland.
   * The default is false.
   */
  void SetDefineWindowsNULL(bool v)  {this->DefineWindowsNULL = v;}

  /**
   * If set to true, cd dir && command is used to 
   * run commands in a different directory.
   */
  void SetUnixCD(bool v)  {this->UnixCD = v;}

  /**
   * Set Support Verbose Variable.  If true, then .SILENT will
   * be not end with :  i.e. .SILENT: or .SILENT
   */
  void SetSilentNoColon(bool v)  {this->SilentNoColon = v;}

  /**
   * Set the string used to include one makefile into another default
   * is include.
   */
  void SetIncludeDirective(const char* s) { this->IncludeDirective = s; }
  const char *GetIncludeDirective() { return this->IncludeDirective.c_str(); }

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { this->MakefileVariableSize = s; }

  /**
   * If ignore lib prefix is true, then do not strip lib from the name
   * of a library.
   */
  void SetIgnoreLibPrefix(bool s) { this->IgnoreLibPrefix = s; }

  // used in writing out Cmake files such as WriteDirectoryInformation
  static void WriteCMakeArgument(std::ostream& os, const char* s);

  /** creates the common disclainer text at the top of each makefile */
  void WriteDisclaimer(std::ostream& os);

  // write a  comment line #====... in the stream
  void WriteDivider(std::ostream& os);

  /** used to create a recursive make call */
  std::string GetRecursiveMakeCall(const char *makefile, const char* tgt);    
  
  // append an echo command
  enum EchoColor { EchoNormal, EchoDepend, EchoBuild, EchoLink,
                   EchoGenerate, EchoGlobal };
  void AppendEcho(std::vector<std::string>& commands, const char* text,
                  EchoColor color = EchoNormal);

  static std::string GetTargetDirectory(cmTarget& target);

    // create a command that cds to the start dir then runs the commands
  void CreateCDCommand(std::vector<std::string>& commands, 
                       const char *targetDir, const char *returnDir);

  static std::string ConvertToQuotedOutputPath(const char* p);

  std::string& CreateSafeUniqueObjectFileName(const char* sin);
  std::string CreateMakeVariable(const char* sin, const char* s2in);

  // cleanup the name of a potential target
  std::string ConvertToMakeTarget(const char* tgt);


  const char* GetSourceFileLanguage(const cmSourceFile& source);


  
  
  /** Called from command-line hook to scan dependencies.  */
  virtual bool ScanDependencies(std::vector<std::string> const& args);

  /** Called from command-line hook to check dependencies.  */
  virtual void CheckDependencies(cmMakefile* mf, bool verbose,
                                 bool clear);
  
  /** write some extra rules such as make test etc */
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);

  std::string GetRelativeTargetDirectory(cmTarget& target);

  // List the files for which to check dependency integrity.  Each
  // language has its own list because integrity may be checked
  // differently.
  struct IntegrityCheckSet: public std::set<cmSourceFile *> {};
  struct IntegrityCheckSetMap: public std::map<cmStdString, IntegrityCheckSet>
  {};
  std::map<cmStdString, IntegrityCheckSetMap> &GetIntegrityCheckSet() 
  { return this->CheckDependFiles;}
  
  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);

  // write the target rules for the local Makefile into the stream
  void WriteLocalAllRules(std::ostream& ruleFileStream);
  
  std::map<cmStdString,std::vector<cmTarget *> > GetLocalObjectFiles()
    { return this->LocalObjectFiles;}

protected:
  // these two methods just compute reasonable values for LibraryOutputPath
  // and ExecutableOutputPath
  void ConfigureOutputPaths();
  void FormatOutputPath(std::string& path, const char* name);

  void WriteLocalMakefile();
  
  // write the target rules for the local Makefile into the stream
  void WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                                 std::set<cmStdString> &emitted);

  // this method Writes the Directory informaiton files
  void WriteDirectoryInformationFile();


  












  // write the depend info 
  void WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &tgt);
  
  // write the local help rule
  void WriteHelpRule(std::ostream& ruleFileStream);
  
  // this converts a file name that is relative to the StartOuputDirectory
  // into a full path
  std::string ConvertToFullPath(const std::string& localPath);

  
  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const char* realTarget,
                            const char* helpTarget);

  void WriteTargetDependRule(std::ostream& ruleFileStream,
                             cmTarget& target);
  void WriteTargetCleanRule(std::ostream& ruleFileStream,
                            cmTarget& target,
                            const std::vector<std::string>& files);
  void WriteTargetRequiresRule(std::ostream& ruleFileStream,
                               cmTarget& target,
                               const std::vector<std::string>& objects);
  
  std::string GetObjectFileName(cmTarget& target,
                                const cmSourceFile& source,
                                std::string* nameWithoutTargetDir = 0);

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
                          const std::vector<std::string>& files,
                          cmTarget& target, const char* filename =0);

private:
  friend class cmMakefileTargetGenerator;
  friend class cmMakefileExecutableTargetGenerator;
  friend class cmMakefileLibraryTargetGenerator;
  friend class cmMakefileUtilityTargetGenerator;
  
  std::map<cmStdString, IntegrityCheckSetMap> CheckDependFiles;

  //==========================================================================
  // Configuration settings.
  int MakefileVariableSize;
  std::string IncludeDirective;
  std::string MakeSilentFlag;
  std::string ExecutableOutputPath;
  std::string LibraryOutputPath;
  std::string ConfigurationName;
  bool DefineWindowsNULL;
  bool UnixCD;
  bool PassMakeflags;
  bool SilentNoColon;
  // Flag for whether echo command needs quotes.
  bool EchoNeedsQuote;
  //==========================================================================

  std::string HomeRelativeOutputPath;
  
  std::map<cmStdString,std::vector<cmTarget *> > LocalObjectFiles;

  /* does the work for each target */
  std::vector<cmMakefileTargetGenerator *> TargetGenerators;
  std::map<cmStdString, cmStdString> MakeVariableMap;
  std::map<cmStdString, cmStdString> ShortMakeVariableMap;
  std::map<cmStdString, cmStdString> UniqueObjectNamesMap;
};

#endif
