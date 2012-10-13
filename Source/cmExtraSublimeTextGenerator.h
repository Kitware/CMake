/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExtraSublimeTextGenerator_h
#define cmExtraSublimeTextGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;
class cmMakefile;
class cmTarget;
class cmGeneratedFileStream;

/** \class cmExtraSublimeTextGenerator
 * \brief Write Sublime Text 2 project files for Makefile based projects
 */
class cmExtraSublimeTextGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraSublimeTextGenerator();

  virtual const char* GetName() const
                        { return cmExtraSublimeTextGenerator::GetActualName();}
  static const char* GetActualName()
                        { return "SublimeText2";}
  static cmExternalMakefileProjectGenerator* New()
                                    { return new cmExtraSublimeTextGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const char* fullName) const;

  virtual void Generate();
private:

  void CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs);

  void CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                                const std::string& filename);
  /** Populates allFiles with the full paths to all of the source files
   *  from the local generators in lgs.
   */
  void GetFileList(const std::vector<cmLocalGenerator*>& lgs,
                   std::vector<std::string>& allFiles);
  /** Sends comma seperated source files paths to fileIncludePatternsStream
   *  and builds a set of all directories and subdirectories containing
   *  source files.
   */
  void GetFileStringAndFolderSet(const std::vector<cmLocalGenerator*>& lgs,
                                 const cmMakefile* mf,
                                 const std::vector<std::string>& allFiles,
                                 std::stringstream& fileIncludePatternsStream,
                                 std::set<std::string>&
                                   folderIncludePatternsSet);
  /** Appends all targets as build systems to the project file and get all
   * include directories and compiler definitions used.
   */
  void AppendAllTargets(const std::vector<cmLocalGenerator*>& lgs,
                        const cmMakefile* mf,
                        cmGeneratedFileStream& fout,
                        std::set<std::string>& includeDirs,
                        std::set<std::string>& defines);
  /** Returns the build command that needs to be executed to build the
   *  specified target.
   */
  std::string BuildMakeCommand(const std::string& make, const char* makefile,
                               const char* target);
  /** Appends the specified target to the generated project file as a Sublime
   *  Text build system.
   */
  void AppendTarget(cmGeneratedFileStream& fout,
                    const char* targetName,
                    cmTarget* target,
                    const char* make,
                    const cmMakefile* makefile,
                    const char* compiler,
                    std::set<std::string>& includeDirs,
                    std::set<std::string>& defines, bool firstTarget);

};

#endif
