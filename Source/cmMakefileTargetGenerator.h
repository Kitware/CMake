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
#ifndef cmMakefileTargetGenerator_h
#define cmMakefileTargetGenerator_h

#include "cmLocalUnixMakefileGenerator3.h"

class cmCustomCommand;
class cmDependInformation;
class cmDepends;
class cmGeneratedFileStream;
class cmGlobalGenerator;
class cmLocalUnixMakefileGenerator3;
class cmMakeDepend;
class cmMakefile;
class cmTarget;
class cmSourceFile;

/** \class cmMakefileTargetGenerator
 * \brief Support Routines for writing makefiles
 *
 */
class cmMakefileTargetGenerator 
{
public:
  // constructor to set the ivars
  cmMakefileTargetGenerator();
  virtual ~cmMakefileTargetGenerator() {};

  // construct using this factory call
  static cmMakefileTargetGenerator *New(cmLocalUnixMakefileGenerator3 *lg,
                                        cmStdString tgtName,
                                        cmTarget *tgt);

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  virtual void WriteRuleFiles() = 0;
  
protected:

  // create the file and directory etc
  void CreateRuleFile();
  
  // outputs the rules for any custom commands used by this target
  void WriteCustomCommandsForTarget();

  // write some common code at the top of build.make
  void WriteCommonCodeRules();

  // write the provide require rules for this target
  void WriteTargetRequiresRules();

  // write the clean rules for this target
  void WriteTargetCleanRules();

  // write the depend rules for this target
  void WriteTargetDependRules();

  // write the rules for an object
  void WriteObjectRuleFiles(cmSourceFile& source);

  // write the build rule for an object
  void WriteObjectBuildFile(std::string &obj,
                            const char *lang, 
                            cmSourceFile& source,
                            std::vector<std::string>& depends);
  
  // write the depend.make file for an object
  void WriteObjectDependRules(cmSourceFile& source,
                              std::vector<std::string>& depends);
  
  // this is responsible for writing all of the rules for all this
  // directories custom commands (but not utility targets)
  void WriteCustomCommands();
  void GenerateCustomRuleFile(const cmCustomCommand& cc);
  
  // write out the variable that lists the objects for this target
  void WriteObjectsVariable(std::string& variableName,
                            std::string& variableNameExternal);
  
  // Return the a string with -F flags on apple
  std::string GetFrameworkFlags();
  
  // append intertarget dependencies
  void AppendTargetDepends(std::vector<std::string>& depends);

  virtual void CloseFileStreams();
  
  cmStdString TargetName;
  cmTarget *Target;
  cmLocalUnixMakefileGenerator3 *LocalGenerator;
  cmGlobalGenerator *GlobalGenerator;
  cmMakefile *Makefile;
  
  // the full path to the build file
  std::string BuildFileName;
  std::string BuildFileNameFull;

  // the path to the directory the build file is in
  std::string TargetBuildDirectory;
  std::string TargetBuildDirectoryFull;

  // the stream for the build file
  cmGeneratedFileStream *BuildFileStream;

  // the stream for the flag file
  std::string FlagFileNameFull;
  cmGeneratedFileStream *FlagFileStream;

  // the stream for the info file
  std::string InfoFileNameFull;
  cmGeneratedFileStream *InfoFileStream;

  // files to clean
  std::vector<std::string> CleanFiles;

  // objects used by this target
  std::vector<std::string> Objects;
  std::vector<std::string> ExternalObjects;

  // Set of object file names that will be built in this directory.
  std::set<cmStdString> m_ObjectFiles;


  //==================================================================
  // Convenience routines that do nothing more than forward to 
  // implementaitons
  std::string Convert(const char* source, 
                      cmLocalGenerator::RelativeRoot relative, 
                      cmLocalGenerator::OutputFormat output = 
                      cmLocalGenerator::UNCHANGED,
                      bool optional = false) 
  {
    return this->LocalGenerator->Convert(source, relative, output, optional);
  }

};

#endif
