/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmDocumentationEntry.h"
#include "cmGlobalGenerator.h"
#include "cmTransformDepfile.h"
#include "cmValue.h"
#include "cmXCodeObject.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmGeneratorTarget;
class cmGlobalGeneratorFactory;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmSourceGroup;
class cmake;

/** \class cmGlobalXCodeGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalXCodeGenerator manages Xcode build process for a tree
 */
class cmGlobalXCodeGenerator : public cmGlobalGenerator
{
public:
  cmGlobalXCodeGenerator(cmake* cm, std::string const& version_string,
                         unsigned int version_number);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  cmGlobalXCodeGenerator(cmGlobalXCodeGenerator const&) = delete;
  cmGlobalXCodeGenerator const& operator=(cmGlobalXCodeGenerator const&) =
    delete;

  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalXCodeGenerator::GetActualName();
  }
  static std::string GetActualName() { return "Xcode"; }

  /** Get the documentation entry for this generator.  */
  static cmDocumentationEntry GetDocumentation();

  //! Create a local generator appropriate to this Global Generator
  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf) override;

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  /**
   * Open a generated IDE project given the following information.
   */
  bool Open(std::string const& bindir, std::string const& projectName,
            bool dryRun) override;

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions const& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  /** Append the subdirectory for the given configuration.  */
  void AppendDirectoryForConfig(std::string const& prefix,
                                std::string const& config,
                                std::string const& suffix,
                                std::string& dir) override;

  bool FindMakeProgram(cmMakefile*) override;

  //! What is the configurations directory variable called?
  char const* GetCMakeCFGIntDir() const override;
  //! expand CFGIntDir
  std::string ExpandCFGIntDir(std::string const& str,
                              std::string const& config) const override;

  void SetCurrentLocalGenerator(cmLocalGenerator*);

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  bool IsMultiConfig() const override;

  bool IsXcode() const override { return true; }

  bool HasKnownObjectFileLocation(cmTarget const&,
                                  std::string* reason) const override;

  bool IsIPOSupported() const override { return true; }

  bool UseEffectivePlatformName(cmMakefile* mf) const override;

  bool ShouldStripResourcePath(cmMakefile*) const override;

  /**
   * Used to determine if this generator supports DEPFILE option.
   */
  bool SupportsCustomCommandDepfile() const override { return true; }
  cm::optional<cmDepfileFormat> DepfileFormat() const override
  {
    return this->XcodeBuildSystem == BuildSystem::One
      ? cmDepfileFormat::MakeDepfile
      : cmDepfileFormat::GccDepfile;
  }

  bool SetSystemName(std::string const& s, cmMakefile* mf) override;
  bool SetGeneratorToolset(std::string const& ts, bool build,
                           cmMakefile* mf) override;
  void AppendFlag(std::string& flags, std::string const& flag) const;

  enum class BuildSystem
  {
    One = 1,
    Twelve = 12,
  };

protected:
  void AddExtraIDETargets() override;
  void Generate() override;

private:
  enum EmbedActionFlags
  {
    NoActionOnCopyByDefault = 0,
    CodeSignOnCopyByDefault = 1,
    RemoveHeadersOnCopyByDefault = 2,
  };

  bool ParseGeneratorToolset(std::string const& ts, cmMakefile* mf);
  bool ProcessGeneratorToolsetField(std::string const& key,
                                    std::string const& value, cmMakefile* mf);

  cmXCodeObject* CreateOrGetPBXGroup(cmGeneratorTarget* gtgt,
                                     cmSourceGroup* sg);
  cmXCodeObject* CreatePBXGroup(cmXCodeObject* parent,
                                std::string const& name);
  bool CreateGroups(std::vector<cmLocalGenerator*>& generators);
  std::string XCodeEscapePath(std::string const& p);
  std::string RelativeToSource(std::string const& p);
  std::string RelativeToRootBinary(std::string const& p);
  std::string RelativeToBinary(std::string const& p);
  void CreateCustomCommands(
    cmXCodeObject* buildPhases, cmXCodeObject* sourceBuildPhase,
    cmXCodeObject* headerBuildPhase, cmXCodeObject* resourceBuildPhase,
    std::vector<cmXCodeObject*> const& contentBuildPhases,
    cmXCodeObject* frameworkBuildPhase, cmGeneratorTarget* gtgt);

  std::string ComputeInfoPListLocation(cmGeneratorTarget* target);

  void AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                               cmGeneratorTarget* target,
                               std::vector<cmCustomCommand> const& commands,
                               char const* commandFileName);

  void CreateCustomRulesMakefile(char const* makefileBasename,
                                 cmGeneratorTarget* target,
                                 std::vector<cmCustomCommand> const& commands,
                                 std::string const& configName);

  cmXCodeObject* FindXCodeTarget(cmGeneratorTarget const*);
  std::string GetObjectId(cmXCodeObject::PBXType ptype, cm::string_view key);
  std::string GetOrCreateId(std::string const& name, std::string const& id);

  // create cmXCodeObject from these functions so that memory can be managed
  // correctly.  All objects created are stored in this->XCodeObjects.
  cmXCodeObject* CreateObject(cmXCodeObject::PBXType ptype,
                              cm::string_view key = {});
  cmXCodeObject* CreateObject(cmXCodeObject::Type type);
  cmXCodeObject* CreateString(std::string const& s);
  cmXCodeObject* CreateObjectReference(cmXCodeObject*);
  cmXCodeObject* CreateFlatClone(cmXCodeObject*);
  cmXCodeObject* CreateXCodeTarget(cmGeneratorTarget* gtgt,
                                   cmXCodeObject* buildPhases);
  void ForceLinkerLanguages() override;
  void ForceLinkerLanguage(cmGeneratorTarget* gtgt);
  char const* GetTargetLinkFlagsVar(cmGeneratorTarget const* target) const;
  char const* GetTargetFileType(cmGeneratorTarget* target);
  char const* GetTargetProductType(cmGeneratorTarget* target);
  std::string AddConfigurations(cmXCodeObject* target,
                                cmGeneratorTarget* gtgt);
  void AppendOrAddBuildSetting(cmXCodeObject* settings, char const* attr,
                               cmXCodeObject* value);
  void AppendBuildSettingAttribute(cmXCodeObject* settings,
                                   char const* attribute, cmXCodeObject* attr,
                                   cmXCodeObject* value);
  void AppendBuildSettingAttribute(cmXCodeObject* target, char const* attr,
                                   cmXCodeObject* value,
                                   std::string const& configName);
  void InheritBuildSettingAttribute(cmXCodeObject* target,
                                    char const* attribute);
  cmXCodeObject* CreateUtilityTarget(cmGeneratorTarget* gtgt);
  void AddDependAndLinkInformation(cmXCodeObject* target);
  void AddEmbeddedObjects(cmXCodeObject* target,
                          std::string const& copyFilesBuildPhaseName,
                          std::string const& embedPropertyName,
                          std::string const& dstSubfolderSpec,
                          int actionsOnByDefault,
                          std::string const& defaultDstPath = "");
  void AddEmbeddedFrameworks(cmXCodeObject* target);
  void AddEmbeddedPlugIns(cmXCodeObject* target);
  void AddEmbeddedAppExtensions(cmXCodeObject* target);
  void AddEmbeddedExtensionKitExtensions(cmXCodeObject* target);
  void AddEmbeddedResources(cmXCodeObject* target);
  void AddEmbeddedXPCServices(cmXCodeObject* target);
  void AddPositionIndependentLinkAttribute(cmGeneratorTarget* target,
                                           cmXCodeObject* buildSettings,
                                           std::string const& configName);
  void CreateGlobalXCConfigSettings(cmLocalGenerator* root,
                                    cmXCodeObject* config,
                                    std::string const& configName);
  void CreateTargetXCConfigSettings(cmGeneratorTarget* target,
                                    cmXCodeObject* config,
                                    std::string const& configName);
  void CreateBuildSettings(cmGeneratorTarget* gtgt,
                           cmXCodeObject* buildSettings,
                           std::string const& buildType);
  std::string ExtractFlag(char const* flag, std::string& flags);
  std::string ExtractFlagRegex(char const* exp, int matchIndex,
                               std::string& flags);
  void FilterConfigurationAttribute(std::string const& configName,
                                    std::string& attribute);
  void SortXCodeObjects();
  // delete all objects in the this->XCodeObjects vector.
  void ClearXCodeObjects();
  bool CreateXCodeObjects(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void OutputXCodeProject(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  // Write shared scheme files for all the native targets
  //  return true if any were written
  bool OutputXCodeSharedSchemes(std::string const& xcProjDir,
                                cmLocalGenerator* root);
  void OutputXCodeWorkspaceSettings(std::string const& xcProjDir,
                                    bool hasGeneratedSchemes);
  void WriteXCodePBXProj(std::ostream& fout, cmLocalGenerator* root,
                         std::vector<cmLocalGenerator*>& generators);
  cmXCodeObject* CreateXCodeFileReferenceFromPath(std::string const& fullpath,
                                                  cmGeneratorTarget* target,
                                                  std::string const& lang,
                                                  cmSourceFile* sf);
  cmXCodeObject* CreateXCodeBuildFileFromPath(std::string const& fullpath,
                                              cmGeneratorTarget* target,
                                              std::string const& lang,
                                              cmSourceFile* sf);
  cmXCodeObject* CreateXCodeFileReference(cmSourceFile* sf,
                                          cmGeneratorTarget* target);
  cmXCodeObject* CreateXCodeSourceFile(cmLocalGenerator* gen, cmSourceFile* sf,
                                       cmGeneratorTarget* gtgt);
  void AddXCodeProjBuildRule(cmGeneratorTarget* target,
                             std::vector<cmSourceFile*>& sources) const;
  bool CreateXCodeTargets(cmLocalGenerator* gen, std::vector<cmXCodeObject*>&);
  bool CreateXCodeTarget(cmGeneratorTarget* gtgt,
                         std::vector<cmXCodeObject*>&);
  bool IsHeaderFile(cmSourceFile*);
  void AddDependTarget(cmXCodeObject* target, cmXCodeObject* dependTarget);
  void CreateXCodeDependHackMakefile(std::vector<cmXCodeObject*>& targets);
  bool SpecialTargetEmitted(std::string const& tname);
  void SetGenerationRoot(cmLocalGenerator* root);
  void AddExtraTargets(cmLocalGenerator* root,
                       std::vector<cmLocalGenerator*>& gens);
  cmXCodeObject* CreateLegacyRunScriptBuildPhase(
    char const* name, char const* name2, cmGeneratorTarget* target,
    std::vector<cmCustomCommand> const&);
  void CreateRunScriptBuildPhases(cmXCodeObject* buildPhases,
                                  cmGeneratorTarget const* gt);
  void CreateRunScriptBuildPhases(cmXCodeObject* buildPhases,
                                  cmSourceFile const* sf,
                                  cmGeneratorTarget const* gt,
                                  std::set<cmSourceFile const*>& visited);
  cmXCodeObject* CreateRunScriptBuildPhase(cmSourceFile const* sf,
                                           cmGeneratorTarget const* gt,
                                           cmCustomCommand const& cc);
  cmXCodeObject* CreateRunScriptBuildPhase(
    std::string const& name, cmGeneratorTarget const* gt,
    std::vector<cmCustomCommand> const& commands);
  std::string ConstructScript(cmCustomCommandGenerator const& ccg);
  void CreateReRunCMakeFile(cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*> const& gens);

  std::string LookupFlags(std::string const& varNamePrefix,
                          std::string const& varNameLang,
                          std::string const& varNameSuffix,
                          cmGeneratorTarget const* gt,
                          std::string const& default_flags);

  class Factory;
  class BuildObjectListOrString;
  friend class BuildObjectListOrString;

  void AppendDefines(BuildObjectListOrString& defs, char const* defines_list,
                     bool dflag = false);
  void AppendDefines(BuildObjectListOrString& defs,
                     std::vector<std::string> const& defines,
                     bool dflag = false);

  void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

protected:
  char const* GetInstallTargetName() const override { return "install"; }
  char const* GetPackageTargetName() const override { return "package"; }

  unsigned int XcodeVersion;
  std::string VersionString;
  std::set<std::string> XCodeObjectIDs;
  std::vector<std::unique_ptr<cmXCodeObject>> XCodeObjects;
  cmXCodeObject* RootObject;

  BuildSystem XcodeBuildSystem = BuildSystem::One;

private:
  std::string const& GetXcodeBuildCommand();
  std::string FindXcodeBuildCommand();
  std::string XcodeBuildCommand;
  bool XcodeBuildCommandInitialized;

  void PrintCompilerAdvice(std::ostream&, std::string const&,
                           cmValue) const override
  {
  }

  std::string GetLibraryOrFrameworkPath(std::string const& path) const;

  std::string GetSymrootDir() const;
  std::string GetTargetTempDir(cmGeneratorTarget const* gt,
                               std::string const& configName) const;

  static std::string GetDeploymentPlatform(cmMakefile const* mf);

  void ComputeArchitectures(cmMakefile* mf);
  void ComputeObjectDirArch(cmMakefile* mf);

  void addObject(std::unique_ptr<cmXCodeObject> obj);
  std::string PostBuildMakeTarget(std::string const& tName,
                                  std::string const& configName);
  cmXCodeObject* MainGroupChildren;
  cmXCodeObject* FrameworkGroup;
  cmXCodeObject* ResourcesGroup;
  cmMakefile* CurrentMakefile;
  cmLocalGenerator* CurrentLocalGenerator;
  cmLocalGenerator* CurrentRootGenerator = nullptr;
  std::vector<std::string> CurrentConfigurationTypes;
  std::string CurrentReRunCMakeMakefile;
  std::string CurrentXCodeHackMakefile;
  std::string CurrentProject;
  std::set<std::string> TargetDoneSet;
  std::map<std::string, cmXCodeObject*> GroupMap;
  std::map<std::string, cmXCodeObject*> GroupNameMap;
  std::map<std::string, cmXCodeObject*> TargetGroup;
  std::map<std::string, cmXCodeObject*> FileRefs;
  std::map<std::string, cmXCodeObject*> ExternalLibRefs;
  std::map<cmGeneratorTarget const*, cmXCodeObject*> XCodeObjectMap;
  std::map<cmXCodeObject*, cmXCodeObject*> FileRefToBuildFileMap;
  std::map<cmXCodeObject*, cmXCodeObject*> FileRefToEmbedBuildFileMap;
  std::vector<std::string> Architectures;
  std::string ObjectDirArchDefault;
  std::string ObjectDirArch;
  std::string SystemName;
  std::string GeneratorToolset;
  std::vector<std::string> EnabledLangs;
  std::map<cmGeneratorTarget const*, std::set<cmSourceFile const*>>
    CommandsVisited;
  std::map<cmSourceFile const*, std::set<cmGeneratorTarget const*>>
    CustomCommandRoots;
};
