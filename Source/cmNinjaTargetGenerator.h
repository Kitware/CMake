/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmNinjaTargetGenerator_h
#define cmNinjaTargetGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cm_jsoncpp_value.h"

#include "cmCommonTargetGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmNinjaTypes.h"
#include "cmOSXBundleGenerator.h"

class cmCustomCommand;
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

  bool NeedDepTypeMSVC(const std::string& lang) const;

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

  std::string LanguageCompilerRule(const std::string& lang,
                                   const std::string& config) const;
  std::string LanguagePreprocessRule(std::string const& lang,
                                     const std::string& config) const;
  bool NeedExplicitPreprocessing(std::string const& lang) const;
  std::string LanguageDyndepRule(std::string const& lang,
                                 const std::string& config) const;
  bool NeedDyndep(std::string const& lang) const;
  bool UsePreprocessedSource(std::string const& lang) const;
  bool CompilePreprocessedSourceWithDefines(std::string const& lang) const;

  std::string OrderDependsTargetForTarget(const std::string& config);

  std::string ComputeOrderDependsForTarget();

  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile const* source,
                                    const std::string& language,
                                    const std::string& config);

  void AddIncludeFlags(std::string& flags, std::string const& lang,
                       const std::string& config) override;

  std::string ComputeDefines(cmSourceFile const* source,
                             const std::string& language,
                             const std::string& config);

  std::string ComputeIncludes(cmSourceFile const* source,
                              const std::string& language,
                              const std::string& config);

  std::string ConvertToNinjaPath(const std::string& path) const
  {
    return this->GetGlobalGenerator()->ConvertToNinjaPath(path);
  }
  cmGlobalNinjaGenerator::MapToNinjaPathImpl MapToNinjaPath() const
  {
    return this->GetGlobalGenerator()->MapToNinjaPath();
  }

  /// @return the list of link dependency for the given target @a target.
  cmNinjaDeps ComputeLinkDeps(const std::string& linkLanguage,
                              const std::string& config) const;

  /// @return the source file path for the given @a source.
  std::string GetSourceFilePath(cmSourceFile const* source) const;

  /// @return the object file path for the given @a source.
  std::string GetObjectFilePath(cmSourceFile const* source,
                                const std::string& config) const;

  /// @return the preprocessed source file path for the given @a source.
  std::string GetPreprocessedFilePath(cmSourceFile const* source,
                                      const std::string& config) const;

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
  void WriteObjectBuildStatements(const std::string& config,
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

  void ExportObjectCompileCommand(
    std::string const& language, std::string const& sourceFileName,
    std::string const& objectDir, std::string const& objectFileName,
    std::string const& objectFileDir, std::string const& flags,
    std::string const& defines, std::string const& includes);

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

  struct ByConfig
  {
    /// List of object files for this target.
    cmNinjaDeps Objects;
    // Fortran Support
    std::map<std::string, cmNinjaDeps> DDIFiles;
    // Swift Support
    Json::Value SwiftOutputMap;
    std::vector<cmCustomCommand const*> CustomCommands;
    cmNinjaDeps ExtraFiles;
    std::unique_ptr<MacOSXContentGeneratorType> MacOSXContentGenerator;
  };

  std::map<std::string, ByConfig> Configs;
};

#endif // ! cmNinjaTargetGenerator_h
