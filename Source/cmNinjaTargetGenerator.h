/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmCommonTargetGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmImportedCxxModuleInfo.h"
#include "cmNinjaTypes.h"
#include "cmOSXBundleGenerator.h"

class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmLocalNinjaGenerator;
class cmMakefile;
class cmSourceFile;

class cmNinjaTargetGenerator : public cmCommonTargetGenerator
{
public:
  /// Create a cmNinjaTargetGenerator according to the @a target's type.
  static std::unique_ptr<cmNinjaTargetGenerator> New(
    cmGeneratorTarget* target);

  /// Build a NinjaTargetGenerator.
  cmNinjaTargetGenerator(cmGeneratorTarget* target);

  /// Destructor.
  ~cmNinjaTargetGenerator() override;

  virtual void Generate(const std::string& config) = 0;

  std::string GetTargetName() const;

protected:
  bool SetMsvcTargetPdbVariable(cmNinjaVars&, const std::string& config) const;

  cmGeneratedFileStream& GetImplFileStream(const std::string& config) const;
  cmGeneratedFileStream& GetCommonFileStream() const;
  cmGeneratedFileStream& GetRulesFileStream() const;

  cmGeneratorTarget* GetGeneratorTarget() const
  {
    return this->GeneratorTarget;
  }

  cmLocalNinjaGenerator* GetLocalGenerator() const
  {
    return this->LocalGenerator;
  }

  cmGlobalNinjaGenerator* GetGlobalGenerator() const;

  cmMakefile* GetMakefile() const { return this->Makefile; }

  enum class WithScanning
  {
    No,
    Yes,
  };
  std::string LanguageCompilerRule(const std::string& lang,
                                   const std::string& config,
                                   WithScanning withScanning) const;
  std::string LanguagePreprocessAndScanRule(std::string const& lang,
                                            const std::string& config) const;
  std::string LanguageScanRule(std::string const& lang,
                               const std::string& config) const;
  std::string LanguageDyndepRule(std::string const& lang,
                                 const std::string& config) const;
  bool NeedExplicitPreprocessing(std::string const& lang) const;
  bool CompileWithDefines(std::string const& lang) const;

  std::string OrderDependsTargetForTarget(const std::string& config);
  std::string OrderDependsTargetForTargetPrivate(const std::string& config);

  std::string ComputeOrderDependsForTarget();

  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile const* source,
                                    const std::string& language,
                                    const std::string& config,
                                    const std::string& objectFileName);

  void AddIncludeFlags(std::string& flags, std::string const& lang,
                       const std::string& config) override;

  std::string ComputeDefines(cmSourceFile const* source,
                             const std::string& language,
                             const std::string& config);

  std::string ComputeIncludes(cmSourceFile const* source,
                              const std::string& language,
                              const std::string& config);

  std::string const& ConvertToNinjaPath(const std::string& path) const
  {
    return this->GetGlobalGenerator()->ConvertToNinjaPath(path);
  }
  cmGlobalNinjaGenerator::MapToNinjaPathImpl MapToNinjaPath() const
  {
    return this->GetGlobalGenerator()->MapToNinjaPath();
  }

  std::string ConvertToNinjaAbsPath(std::string path) const
  {
    return this->GetGlobalGenerator()->ConvertToNinjaAbsPath(std::move(path));
  }

  /// @return the list of link dependency for the given target @a target.
  cmNinjaDeps ComputeLinkDeps(const std::string& linkLanguage,
                              const std::string& config,
                              bool ignoreType = false) const;

  /// @return the source file path for the given @a source.
  std::string GetCompiledSourceNinjaPath(cmSourceFile const* source) const;

  /// @return the object file path for the given @a source.
  std::string GetObjectFilePath(cmSourceFile const* source,
                                const std::string& config) const;
  std::string GetBmiFilePath(cmSourceFile const* source,
                             const std::string& config) const;

  /// @return the preprocessed source file path for the given @a source.
  std::string GetPreprocessedFilePath(cmSourceFile const* source,
                                      const std::string& config) const;

  /// @return the clang-tidy replacements file path for the given @a source.
  std::string GetClangTidyReplacementsFilePath(
    std::string const& directory, cmSourceFile const& source,
    std::string const& config) const override;

  /// @return the dyndep file path for this target.
  std::string GetDyndepFilePath(std::string const& lang,
                                const std::string& config) const;

  /// @return the target dependency scanner info file path
  std::string GetTargetDependInfoPath(std::string const& lang,
                                      const std::string& config) const;

  /// @return the file path where the target named @a name is generated.
  std::string GetTargetFilePath(const std::string& name,
                                const std::string& config) const;

  /// @return the output path for the target.
  virtual std::string GetTargetOutputDir(const std::string& config) const;

  void WriteLanguageRules(const std::string& language,
                          const std::string& config);
  void WriteCompileRule(const std::string& language,
                        const std::string& config);
  void WriteCompileRule(const std::string& language, const std::string& config,
                        WithScanning withScanning);
  void WriteObjectBuildStatements(const std::string& config,
                                  const std::string& fileConfig,
                                  bool firstForConfig);
  void WriteCxxModuleBmiBuildStatement(cmSourceFile const* source,
                                       const std::string& config,
                                       const std::string& fileConfig,
                                       bool firstForConfig);
  void WriteObjectBuildStatement(cmSourceFile const* source,
                                 const std::string& config,
                                 const std::string& fileConfig,
                                 bool firstForConfig);
  void WriteTargetDependInfo(std::string const& lang,
                             const std::string& config);

  void EmitSwiftDependencyInfo(cmSourceFile const* source,
                               const std::string& config);

  void GenerateSwiftOutputFileMap(const std::string& config,
                                  std::string& flags);

  void ExportObjectCompileCommand(
    std::string const& language, std::string const& sourceFileName,
    std::string const& objectDir, std::string const& objectFileName,
    std::string const& objectFileDir, std::string const& flags,
    std::string const& defines, std::string const& includes,
    std::string const& outputConfig, WithScanning withScanning);

  void AdditionalCleanFiles(const std::string& config);

  cmNinjaDeps GetObjects(const std::string& config) const;

  void EnsureDirectoryExists(const std::string& dir) const;
  void EnsureParentDirectoryExists(const std::string& path) const;

  // write rules for macOS Application Bundle content.
  struct MacOSXContentGeneratorType
    : cmOSXBundleGenerator::MacOSXContentGeneratorType
  {
    MacOSXContentGeneratorType(cmNinjaTargetGenerator* g,
                               std::string fileConfig)
      : Generator(g)
      , FileConfig(std::move(fileConfig))
    {
    }

    void operator()(cmSourceFile const& source, const char* pkgloc,
                    const std::string& config) override;

  private:
    cmNinjaTargetGenerator* Generator;
    std::string FileConfig;
  };
  friend struct MacOSXContentGeneratorType;

  // Properly initialized by sub-classes.
  std::unique_ptr<cmOSXBundleGenerator> OSXBundleGenerator;
  std::set<std::string> MacContentFolders;

  void addPoolNinjaVariable(const std::string& pool_property,
                            cmGeneratorTarget* target, cmNinjaVars& vars);

  bool ForceResponseFile();

private:
  cmLocalNinjaGenerator* LocalGenerator;
  bool HasPrivateGeneratedSources = false;

  struct ScanningFiles
  {
    bool IsEmpty() const
    {
      return this->ScanningOutput.empty() && this->ModuleMapFile.empty();
    }

    std::string ScanningOutput;
    std::string ModuleMapFile;
  };

  struct ByConfig
  {
    /// List of object files for this target.
    cmNinjaDeps Objects;
    // Dyndep Support
    std::map<std::string, std::vector<ScanningFiles>> ScanningInfo;
    // Imported C++ module info.
    mutable ImportedCxxModuleLookup ImportedCxxModules;
    // Swift Support
    Json::Value SwiftOutputMap;
    cmNinjaDeps ExtraFiles;
    std::unique_ptr<MacOSXContentGeneratorType> MacOSXContentGenerator;
  };

  std::map<std::string, ByConfig> Configs;
};
