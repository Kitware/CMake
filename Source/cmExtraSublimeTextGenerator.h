/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include "cmExternalMakefileProjectGenerator.h"

class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmExtraSublimeTextGenerator
 * \brief Write Sublime Text 2 project files for Makefile based projects
 */
class cmExtraSublimeTextGenerator : public cmExternalMakefileProjectGenerator
{
public:
  static cmExternalMakefileProjectGeneratorFactory* GetFactory();
  using MapSourceFileFlags = std::map<std::string, std::vector<std::string>>;
  cmExtraSublimeTextGenerator();

  void Generate() override;

private:
  void CreateProjectFile(std::vector<cmLocalGenerator*> const& lgs);

  void CreateNewProjectFile(std::vector<cmLocalGenerator*> const& lgs,
                            std::string const& filename);

  /** Appends all targets as build systems to the project file and get all
   * include directories and compiler definitions used.
   */
  void AppendAllTargets(std::vector<cmLocalGenerator*> const& lgs,
                        cmMakefile const* mf, cmGeneratedFileStream& fout,
                        MapSourceFileFlags& sourceFileFlags);
  /** Returns the build command that needs to be executed to build the
   *  specified target.
   */
  std::string BuildMakeCommand(std::string const& make,
                               std::string const& makefile,
                               std::string const& target);
  /** Appends the specified target to the generated project file as a Sublime
   *  Text build system.
   */
  void AppendTarget(cmGeneratedFileStream& fout, std::string const& targetName,
                    cmLocalGenerator* lg, cmGeneratorTarget* target,
                    char const* make, cmMakefile const* makefile,
                    char const* compiler, MapSourceFileFlags& sourceFileFlags,
                    bool firstTarget);
  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile* source, cmLocalGenerator* lg,
                                    cmGeneratorTarget* gtgt);

  std::string ComputeDefines(cmSourceFile* source, cmLocalGenerator* lg,
                             cmGeneratorTarget* gtgt);

  std::string ComputeIncludes(cmSourceFile* source, cmLocalGenerator* lg,
                              cmGeneratorTarget* gtgt);

  bool Open(std::string const& bindir, std::string const& projectName,
            bool dryRun) override;

  bool ExcludeBuildFolder;
  std::string EnvSettings;
};
