/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmDepends.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalGenerator.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator3
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator3 produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalUnixMakefileGenerator3 : public cmLocalCommonGenerator
{
public:
  cmLocalUnixMakefileGenerator3(cmGlobalGenerator* gg, cmMakefile* mf);
  ~cmLocalUnixMakefileGenerator3() override;

  std::string GetConfigName() const;

  void ComputeHomeRelativeOutputPath() override;

  /**
   * Generate the makefile for this directory.
   */
  void Generate() override;

  // this returns the relative path between the HomeOutputDirectory and this
  // local generators StartOutputDirectory
  const std::string& GetHomeRelativeOutputPath();

  /**
   * Convert a file path to a Makefile target or dependency with
   * escaping and quoting suitable for the generator's make tool.
   */
  std::string ConvertToMakefilePath(std::string const& path) const;

  // Write out a make rule
  void WriteMakeRule(std::ostream& os, const char* comment,
                     const std::string& target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands, bool symbolic,
                     bool in_help = false);

  // write the main variables used by the makefiles
  void WriteMakeVariables(std::ostream& makefileStream);

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { this->MakefileVariableSize = s; }

  /**
   * Set whether passing a make target on a command line requires an
   * extra level of escapes.
   */
  void SetMakeCommandEscapeTargetTwice(bool b)
  {
    this->MakeCommandEscapeTargetTwice = b;
  }

  /**
   * Set whether the Borland curly brace command line hack should be
   * applied.
   */
  void SetBorlandMakeCurlyHack(bool b) { this->BorlandMakeCurlyHack = b; }

  // used in writing out Cmake files such as WriteDirectoryInformation
  static void WriteCMakeArgument(std::ostream& os, const std::string& s);

  /** creates the common disclaimer text at the top of each makefile */
  void WriteDisclaimer(std::ostream& os);

  // write a  comment line #====... in the stream
  void WriteDivider(std::ostream& os);

  /** used to create a recursive make call */
  std::string GetRecursiveMakeCall(const std::string& makefile,
                                   const std::string& tgt);

  // append flags to a string
  void AppendFlags(std::string& flags,
                   const std::string& newFlags) const override;
  using cmLocalCommonGenerator::AppendFlags;

  // append an echo command
  enum EchoColor
  {
    EchoNormal,
    EchoDepend,
    EchoBuild,
    EchoLink,
    EchoGenerate,
    EchoGlobal
  };
  struct EchoProgress
  {
    std::string Dir;
    std::string Arg;
  };
  void AppendEcho(std::vector<std::string>& commands, std::string const& text,
                  EchoColor color = EchoNormal, EchoProgress const* = nullptr);

  /** Get whether the makefile is to have color.  */
  bool GetColorMakefile() const { return this->ColorMakefile; }

  std::string GetTargetDirectory(
    cmGeneratorTarget const* target) const override;

  // create a command that cds to the start dir then runs the commands
  void CreateCDCommand(std::vector<std::string>& commands,
                       std::string const& targetDir,
                       std::string const& relDir);

  static std::string ConvertToQuotedOutputPath(const std::string& p,
                                               bool useWatcomQuote);

  std::string CreateMakeVariable(const std::string& sin,
                                 const std::string& s2in);

  /** Called from command-line hook to bring dependencies up to date
      for a target.  */
  bool UpdateDependencies(const std::string& tgtInfo, bool verbose,
                          bool color) override;

  /** Called from command-line hook to clear dependencies.  */
  void ClearDependencies(cmMakefile* mf, bool verbose) override;

  /** write some extra rules such as make test etc */
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);

  std::string GetRelativeTargetDirectory(
    cmGeneratorTarget const* target) const;

  // File pairs for implicit dependency scanning.  The key of the map
  // is the depender and the value is the explicit dependee.
  using ImplicitDependFileMap = cmDepends::DependencyMap;
  using ImplicitDependLanguageMap =
    std::map<std::string, ImplicitDependFileMap>;
  using ImplicitDependScannerMap =
    std::map<cmDependencyScannerKind, ImplicitDependLanguageMap>;
  using ImplicitDependTargetMap =
    std::map<std::string, ImplicitDependScannerMap>;
  ImplicitDependLanguageMap const& GetImplicitDepends(
    cmGeneratorTarget const* tgt,
    cmDependencyScannerKind scanner = cmDependencyScannerKind::CMake);

  void AddImplicitDepends(
    cmGeneratorTarget const* tgt, const std::string& lang,
    const std::string& obj, const std::string& src,
    cmDependencyScannerKind scanner = cmDependencyScannerKind::CMake);

  // write the target rules for the local Makefile into the stream
  void WriteLocalAllRules(std::ostream& ruleFileStream);

  std::vector<std::string> const& GetLocalHelp() { return this->LocalHelp; }

  /** Get whether to create rules to generate preprocessed and
      assembly sources.  This could be converted to a variable lookup
      later.  */
  bool GetCreatePreprocessedSourceRules() const
  {
    return !this->SkipPreprocessedSourceRules;
  }
  bool GetCreateAssemblySourceRules() const
  {
    return !this->SkipAssemblySourceRules;
  }

  // Fill the vector with the target names for the object files,
  // preprocessed files and assembly files. Currently only used by the
  // Eclipse generator.
  void GetIndividualFileTargets(std::vector<std::string>& targets);

  std::string GetLinkDependencyFile(cmGeneratorTarget* target,
                                    std::string const& config) const override;

protected:
  void WriteLocalMakefile();

  // write the target rules for the local Makefile into the stream
  void WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                                 std::set<std::string>& emitted);

  // this method Writes the Directory information files
  void WriteDirectoryInformationFile();

  // write the depend info
  void WriteDependLanguageInfo(std::ostream& cmakefileStream,
                               cmGeneratorTarget* tgt);

  // this converts a file name that is relative to the StartOuputDirectory
  // into a full path
  std::string ConvertToFullPath(const std::string& localPath);

  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const std::string& realTarget,
                            const std::string& helpTarget);

  void AppendRuleDepend(std::vector<std::string>& depends,
                        const char* ruleFileName);
  void AppendRuleDepends(std::vector<std::string>& depends,
                         std::vector<std::string> const& ruleFiles);
  void AppendCustomDepends(std::vector<std::string>& depends,
                           const std::vector<cmCustomCommand>& ccs);
  void AppendCustomDepend(std::vector<std::string>& depends,
                          cmCustomCommandGenerator const& cc);
  void AppendCustomCommands(std::vector<std::string>& commands,
                            const std::vector<cmCustomCommand>& ccs,
                            cmGeneratorTarget* target,
                            std::string const& relative);
  void AppendCustomCommand(std::vector<std::string>& commands,
                           cmCustomCommandGenerator const& ccg,
                           cmGeneratorTarget* target,
                           std::string const& relative,
                           bool echo_comment = false,
                           std::ostream* content = nullptr);
  void AppendCleanCommand(std::vector<std::string>& commands,
                          const std::set<std::string>& files,
                          cmGeneratorTarget* target,
                          const char* filename = nullptr);
  void AppendDirectoryCleanCommand(std::vector<std::string>& commands);

  // Helper methods for dependency updates.
  bool ScanDependencies(std::string const& targetDir,
                        std::string const& dependFile,
                        std::string const& internalDependFile,
                        cmDepends::DependencyMap& validDeps);
  void CheckMultipleOutputs(bool verbose);

private:
  std::string MaybeConvertWatcomShellCommand(std::string const& cmd);

  friend class cmMakefileTargetGenerator;
  friend class cmMakefileExecutableTargetGenerator;
  friend class cmMakefileLibraryTargetGenerator;
  friend class cmMakefileUtilityTargetGenerator;
  friend class cmGlobalUnixMakefileGenerator3;

  ImplicitDependTargetMap ImplicitDepends;

  std::string HomeRelativeOutputPath;

  struct LocalObjectEntry
  {
    cmGeneratorTarget* Target = nullptr;
    std::string Language;
    LocalObjectEntry() = default;
    LocalObjectEntry(cmGeneratorTarget* t, std::string lang)
      : Target(t)
      , Language(std::move(lang))
    {
    }
  };
  struct LocalObjectInfo : public std::vector<LocalObjectEntry>
  {
    bool HasSourceExtension = false;
    bool HasPreprocessRule = false;
    bool HasAssembleRule = false;
  };
  void GetLocalObjectFiles(
    std::map<std::string, LocalObjectInfo>& localObjectFiles);

  void WriteObjectConvenienceRule(std::ostream& ruleFileStream,
                                  const char* comment,
                                  const std::string& output,
                                  LocalObjectInfo const& info);

  std::vector<std::string> LocalHelp;

  /* does the work for each target */
  std::map<std::string, std::string> MakeVariableMap;
  std::map<std::string, std::string> ShortMakeVariableMap;

  int MakefileVariableSize;
  bool MakeCommandEscapeTargetTwice;
  bool BorlandMakeCurlyHack;
  bool ColorMakefile;
  bool SkipPreprocessedSourceRules;
  bool SkipAssemblySourceRules;

  std::set<cmSourceFile const*>& GetCommandsVisited(
    cmGeneratorTarget const* target)
  {
    return this->CommandsVisited[target];
  }

  std::map<cmGeneratorTarget const*, std::set<cmSourceFile const*>>
    CommandsVisited;
};
