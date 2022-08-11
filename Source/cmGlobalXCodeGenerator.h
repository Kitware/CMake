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

#include <cm/optional>
#include <cm/string_view>

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
struct cmDocumentationEntry;

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

  cmGlobalXCodeGenerator(const cmGlobalXCodeGenerator&) = delete;
  const cmGlobalXCodeGenerator& operator=(const cmGlobalXCodeGenerator&) =
    delete;

  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalXCodeGenerator::GetActualName();
  }
  static std::string GetActualName() { return "Xcode"; }

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

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
  bool Open(const std::string& bindir, const std::string& projectName,
            bool dryRun) override;

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, int jobs, bool verbose,
    const cmBuildOptions& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  /** Append the subdirectory for the given configuration.  */
  void AppendDirectoryForConfig(const std::string& prefix,
                                const std::string& config,
                                const std::string& suffix,
                                std::string& dir) override;

  bool FindMakeProgram(cmMakefile*) override;

  //! What is the configurations directory variable called?
  const char* GetCMakeCFGIntDir() const override;
  //! expand CFGIntDir
  std::string ExpandCFGIntDir(const std::string& str,
                              const std::string& config) const override;

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
  virtual cm::optional<cmDepfileFormat> DepfileFormat() const override
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
                                const std::string& name);
  bool CreateGroups(std::vector<cmLocalGenerator*>& generators);
  std::string XCodeEscapePath(const std::string& p);
  std::string RelativeToSource(const std::string& p);
  std::string RelativeToBinary(const std::string& p);
  std::string ConvertToRelativeForMake(std::string const& p);
  void CreateCustomCommands(
    cmXCodeObject* buildPhases, cmXCodeObject* sourceBuildPhase,
    cmXCodeObject* headerBuildPhase, cmXCodeObject* resourceBuildPhase,
    std::vector<cmXCodeObject*> const& contentBuildPhases,
    cmXCodeObject* frameworkBuildPhase, cmGeneratorTarget* gtgt);

  std::string ComputeInfoPListLocation(cmGeneratorTarget* target);

  void AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                               cmGeneratorTarget* target,
                               std::vector<cmCustomCommand> const& commands,
                               const char* commandFileName);

  void CreateCustomRulesMakefile(const char* makefileBasename,
                                 cmGeneratorTarget* target,
                                 std::vector<cmCustomCommand> const& commands,
                                 const std::string& configName);

  cmXCodeObject* FindXCodeTarget(const cmGeneratorTarget*);
  std::string GetObjectId(cmXCodeObject::PBXType ptype, cm::string_view key);
  std::string GetOrCreateId(const std::string& name, const std::string& id);

  // create cmXCodeObject from these functions so that memory can be managed
  // correctly.  All objects created are stored in this->XCodeObjects.
  cmXCodeObject* CreateObject(cmXCodeObject::PBXType ptype,
                              cm::string_view key = {});
  cmXCodeObject* CreateObject(cmXCodeObject::Type type);
  cmXCodeObject* CreateString(const std::string& s);
  cmXCodeObject* CreateObjectReference(cmXCodeObject*);
  cmXCodeObject* CreateFlatClone(cmXCodeObject*);
  cmXCodeObject* CreateXCodeTarget(cmGeneratorTarget* gtgt,
                                   cmXCodeObject* buildPhases);
  void ForceLinkerLanguages() override;
  void ForceLinkerLanguage(cmGeneratorTarget* gtgt);
  const char* GetTargetLinkFlagsVar(const cmGeneratorTarget* target) const;
  const char* GetTargetFileType(cmGeneratorTarget* target);
  const char* GetTargetProductType(cmGeneratorTarget* target);
  std::string AddConfigurations(cmXCodeObject* target,
                                cmGeneratorTarget* gtgt);
  void AppendOrAddBuildSetting(cmXCodeObject* settings, const char* attr,
                               cmXCodeObject* value);
  void AppendBuildSettingAttribute(cmXCodeObject* settings,
                                   const char* attribute, cmXCodeObject* attr,
                                   cmXCodeObject* value);
  void AppendBuildSettingAttribute(cmXCodeObject* target, const char* attr,
                                   cmXCodeObject* value,
                                   const std::string& configName);
  void InheritBuildSettingAttribute(cmXCodeObject* target,
                                    const char* attribute);
  cmXCodeObject* CreateUtilityTarget(cmGeneratorTarget* gtgt);
  void AddDependAndLinkInformation(cmXCodeObject* target);
  void AddEmbeddedObjects(cmXCodeObject* target,
                          const std::string& copyFilesBuildPhaseName,
                          const std::string& embedPropertyName,
                          const std::string& dstSubfolderSpec,
                          int actionsOnByDefault);
  void AddEmbeddedFrameworks(cmXCodeObject* target);
  void AddEmbeddedPlugIns(cmXCodeObject* target);
  void AddEmbeddedAppExtensions(cmXCodeObject* target);
  void AddPositionIndependentLinkAttribute(cmGeneratorTarget* target,
                                           cmXCodeObject* buildSettings,
                                           const std::string& configName);
  void CreateGlobalXCConfigSettings(cmLocalGenerator* root,
                                    cmXCodeObject* config,
                                    const std::string& configName);
  void CreateTargetXCConfigSettings(cmGeneratorTarget* target,
                                    cmXCodeObject* config,
                                    const std::string& configName);
  void CreateBuildSettings(cmGeneratorTarget* gtgt,
                           cmXCodeObject* buildSettings,
                           const std::string& buildType);
  std::string ExtractFlag(const char* flag, std::string& flags);
  std::string ExtractFlagRegex(const char* exp, int matchIndex,
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
  bool OutputXCodeSharedSchemes(const std::string& xcProjDir,
                                cmLocalGenerator* root);
  void OutputXCodeWorkspaceSettings(const std::string& xcProjDir,
                                    bool hasGeneratedSchemes);
  void WriteXCodePBXProj(std::ostream& fout, cmLocalGenerator* root,
                         std::vector<cmLocalGenerator*>& generators);
  cmXCodeObject* CreateXCodeFileReferenceFromPath(const std::string& fullpath,
                                                  cmGeneratorTarget* target,
                                                  const std::string& lang,
                                                  cmSourceFile* sf);
  cmXCodeObject* CreateXCodeBuildFileFromPath(const std::string& fullpath,
                                              cmGeneratorTarget* target,
                                              const std::string& lang,
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
    const char* name, const char* name2, cmGeneratorTarget* target,
    const std::vector<cmCustomCommand>&);
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

  std::string LookupFlags(const std::string& varNamePrefix,
                          const std::string& varNameLang,
                          const std::string& varNameSuffix,
                          const std::string& default_flags);

  class Factory;
  class BuildObjectListOrString;
  friend class BuildObjectListOrString;

  void AppendDefines(BuildObjectListOrString& defs, const char* defines_list,
                     bool dflag = false);
  void AppendDefines(BuildObjectListOrString& defs,
                     std::vector<std::string> const& defines,
                     bool dflag = false);

  void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

protected:
  const char* GetInstallTargetName() const override { return "install"; }
  const char* GetPackageTargetName() const override { return "package"; }

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

  std::string GetLibraryOrFrameworkPath(const std::string& path) const;

  std::string GetObjectsDirectory(const std::string& projName,
                                  const std::string& configName,
                                  const cmGeneratorTarget* t,
                                  const std::string& variant) const;

  static std::string GetDeploymentPlatform(const cmMakefile* mf);

  void ComputeArchitectures(cmMakefile* mf);
  void ComputeObjectDirArch(cmMakefile* mf);

  void addObject(std::unique_ptr<cmXCodeObject> obj);
  std::string PostBuildMakeTarget(std::string const& tName,
                                  std::string const& configName);
  cmXCodeObject* MainGroupChildren;
  cmXCodeObject* FrameworkGroup;
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
  std::map<std::string, cmXCodeObject*> EmbeddedLibRefs;
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
