/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmCommonTargetGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmOSXBundleGenerator.h"

class cmCustomCommandGenerator;
class cmGeneratedFileStream;
class cmGlobalUnixMakefileGenerator3;
class cmLinkLineComputer;
class cmOutputConverter;
class cmSourceFile;
class cmStateDirectory;

/** \class cmMakefileTargetGenerator
 * \brief Support Routines for writing makefiles
 *
 */
class cmMakefileTargetGenerator : public cmCommonTargetGenerator
{
public:
  // constructor to set the ivars
  cmMakefileTargetGenerator(cmGeneratorTarget* target);
  cmMakefileTargetGenerator(const cmMakefileTargetGenerator&) = delete;
  ~cmMakefileTargetGenerator() override;

  cmMakefileTargetGenerator& operator=(const cmMakefileTargetGenerator&) =
    delete;

  // construct using this factory call
  static std::unique_ptr<cmMakefileTargetGenerator> New(
    cmGeneratorTarget* tgt);

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  virtual void WriteRuleFiles() = 0;

  /* return the number of actions that have progress reporting on them */
  virtual unsigned long GetNumberOfProgressActions()
  {
    return this->NumberOfProgressActions;
  }
  std::string GetProgressFileNameFull() { return this->ProgressFileNameFull; }

  cmGeneratorTarget* GetGeneratorTarget() { return this->GeneratorTarget; }

  std::string GetConfigName();

protected:
  void GetDeviceLinkFlags(std::string& linkFlags,
                          const std::string& linkLanguage);
  void GetTargetLinkFlags(std::string& flags, const std::string& linkLanguage);

  // create the file and directory etc
  void CreateRuleFile();

  // outputs the rules for object files and custom commands used by
  // this target
  void WriteTargetBuildRules();

  // write some common code at the top of build.make
  void WriteCommonCodeRules();
  void WriteTargetLanguageFlags();

  // write the clean rules for this target
  void WriteTargetCleanRules();

  // write the depend rules for this target
  void WriteTargetDependRules();

  // write rules for macOS Application Bundle content.
  struct MacOSXContentGeneratorType
    : cmOSXBundleGenerator::MacOSXContentGeneratorType
  {
    MacOSXContentGeneratorType(cmMakefileTargetGenerator* gen)
      : Generator(gen)
    {
    }

    void operator()(cmSourceFile const& source, const char* pkgloc,
                    const std::string& config) override;

  private:
    cmMakefileTargetGenerator* Generator;
  };
  friend struct MacOSXContentGeneratorType;

  // write the rules for an object
  void WriteObjectRuleFiles(cmSourceFile const& source);

  // write the depend.make file for an object
  void WriteObjectDependRules(cmSourceFile const& source,
                              std::vector<std::string>& depends);

  // CUDA device linking.
  void WriteDeviceLinkRule(std::vector<std::string>& commands,
                           const std::string& output);

  // write the build rule for a custom command
  void GenerateCustomRuleFile(cmCustomCommandGenerator const& ccg);

  // write a rule to drive building of more than one output from
  // another rule
  void GenerateExtraOutput(const char* out, const char* in,
                           bool symbolic = false);

  void MakeEchoProgress(cmLocalUnixMakefileGenerator3::EchoProgress&) const;

  // write out the variable that lists the objects for this target
  void WriteObjectsVariable(std::string& variableName,
                            std::string& variableNameExternal,
                            bool useWatcomQuote);
  void WriteObjectsStrings(std::vector<std::string>& objStrings,
                           std::string::size_type limit = std::string::npos);

  // write the driver rule to build target outputs
  void WriteTargetDriverRule(const std::string& main_output, bool relink);

  void DriveCustomCommands(std::vector<std::string>& depends);

  // append intertarget dependencies
  void AppendTargetDepends(std::vector<std::string>& depends,
                           bool ignoreType = false);

  // Append object file dependencies.
  void AppendObjectDepends(std::vector<std::string>& depends);

  // Append link rule dependencies (objects, etc.).
  void AppendLinkDepends(std::vector<std::string>& depends,
                         const std::string& linkLanguage);

  // Lookup the link rule for this target.
  std::string GetLinkRule(const std::string& linkRuleVar);

  /** Create a script to hold link rules and a command to invoke the
      script at build time.  */
  void CreateLinkScript(const char* name,
                        std::vector<std::string> const& link_commands,
                        std::vector<std::string>& makefile_commands,
                        std::vector<std::string>& makefile_depends);

  std::unique_ptr<cmLinkLineComputer> CreateLinkLineComputer(
    cmOutputConverter* outputConverter, cmStateDirectory const& stateDir);

  /** Create a response file with the given set of options.  Returns
      the relative path from the target build working directory to the
      response file name.  */
  std::string CreateResponseFile(const char* name, std::string const& options,
                                 std::vector<std::string>& makefile_depends);

  bool CheckUseResponseFileForObjects(std::string const& l) const;
  bool CheckUseResponseFileForLibraries(std::string const& l) const;

  /** Create list of flags for link libraries. */
  void CreateLinkLibs(cmLinkLineComputer* linkLineComputer,
                      std::string& linkLibs, bool useResponseFile,
                      std::vector<std::string>& makefile_depends);

  /** Create lists of object files for linking and cleaning.  */
  void CreateObjectLists(bool useLinkScript, bool useArchiveRules,
                         bool useResponseFile, std::string& buildObjs,
                         std::vector<std::string>& makefile_depends,
                         bool useWatcomQuote);

  /** Add commands for generate def files */
  void GenDefFile(std::vector<std::string>& real_link_commands);

  void AddIncludeFlags(std::string& flags, const std::string& lang,
                       const std::string& config) override;

  virtual void CloseFileStreams();
  cmLocalUnixMakefileGenerator3* LocalGenerator;
  cmGlobalUnixMakefileGenerator3* GlobalGenerator;

  enum CustomCommandDriveType
  {
    OnBuild,
    OnDepends,
    OnUtility
  };
  CustomCommandDriveType CustomCommandDriver;

  // the full path to the build file
  std::string BuildFileName;
  std::string BuildFileNameFull;

  // the full path to the progress file
  std::string ProgressFileNameFull;
  unsigned long NumberOfProgressActions;
  bool NoRuleMessages;

  bool CMP0113New = false;

  // the path to the directory the build file is in
  std::string TargetBuildDirectory;
  std::string TargetBuildDirectoryFull;

  // the stream for the build file
  std::unique_ptr<cmGeneratedFileStream> BuildFileStream;

  // the stream for the flag file
  std::string FlagFileNameFull;
  std::unique_ptr<cmGeneratedFileStream> FlagFileStream;
  class StringList : public std::vector<std::string>
  {
  };
  std::map<std::string, StringList> FlagFileDepends;

  // the stream for the info file
  std::string InfoFileNameFull;
  std::unique_ptr<cmGeneratedFileStream> InfoFileStream;

  // files to clean
  std::set<std::string> CleanFiles;

  // objects used by this target
  std::vector<std::string> Objects;
  std::vector<std::string> ExternalObjects;

  // Set of object file names that will be built in this directory.
  std::set<std::string> ObjectFiles;

  // Set of extra output files to be driven by the build.
  std::set<std::string> ExtraFiles;

  // Set of custom command output files to be driven by the build.
  std::set<std::string> CustomCommandOutputs;

  using MultipleOutputPairsType = std::map<std::string, std::string>;
  MultipleOutputPairsType MultipleOutputPairs;
  bool WriteMakeRule(std::ostream& os, const char* comment,
                     const std::vector<std::string>& outputs,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands,
                     bool in_help = false);

  // Target name info.
  cmGeneratorTarget::Names TargetNames;

  // macOS content info.
  std::set<std::string> MacContentFolders;
  std::unique_ptr<cmOSXBundleGenerator> OSXBundleGenerator;
  std::unique_ptr<MacOSXContentGeneratorType> MacOSXContentGenerator;
};
