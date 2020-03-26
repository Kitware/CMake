/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGhsMultiGenerator_h
#define cmGhsMultiGenerator_h

#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmTargetDepend.h"

class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmake;
struct cmDocumentationEntry;

class cmGlobalGhsMultiGenerator : public cmGlobalGenerator
{
public:
  // The default filename extension of GHS MULTI's build files.
  static const char* FILE_EXTENSION;

  cmGlobalGhsMultiGenerator(cmake* cm);
  ~cmGlobalGhsMultiGenerator() override;

  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalGhsMultiGenerator>());
  }

  //! create the correct local generator
  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf) override;

  /// @return the name of this generator.
  static std::string GetActualName() { return "Green Hills MULTI"; }

  //! Get the name for this generator
  std::string GetName() const override { return GetActualName(); }

  /// Overloaded methods. @see cmGlobalGenerator::GetDocumentation()
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Utilized by the generator factory to determine if this generator
   * supports toolsets.
   */
  static bool SupportsToolset() { return true; }

  /**
   * Utilized by the generator factory to determine if this generator
   * supports platforms.
   */
  static bool SupportsPlatform() { return true; }

  // Toolset / Platform Support
  bool SetGeneratorToolset(std::string const& ts, bool build,
                           cmMakefile* mf) override;
  bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf) override;

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;
  /*
   * Determine what program to use for building the project.
   */
  bool FindMakeProgram(cmMakefile* mf) override;

  void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

  // Write the common disclaimer text at the top of each build file.
  void WriteFileHeader(std::ostream& fout);

  const char* GetInstallTargetName() const override { return "install"; }

protected:
  void Generate() override;
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, bool fast, int jobs, bool verbose,
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

private:
  void GetToolset(cmMakefile* mf, std::string& tsd, const std::string& ts);

  /* top-level project */
  void OutputTopLevelProject(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  void WriteTopLevelProject(std::ostream& fout, cmLocalGenerator* root);
  void WriteMacros(std::ostream& fout, cmLocalGenerator* root);
  void WriteHighLevelDirectives(cmLocalGenerator* root, std::ostream& fout);
  void WriteSubProjects(std::ostream& fout, std::string& all_target);
  void WriteTargets(cmLocalGenerator* root);
  void WriteProjectLine(std::ostream& fout, cmGeneratorTarget const* target,
                        cmLocalGenerator* root, std::string& rootBinaryDir);
  void WriteCustomRuleBOD(std::ostream& fout);
  void WriteCustomTargetBOD(std::ostream& fout);
  void WriteAllTarget(cmLocalGenerator* root,
                      std::vector<cmLocalGenerator*>& generators,
                      std::string& all_target);

  std::string TrimQuotes(std::string const& str);

  std::string OsDir;
  static const char* DEFAULT_BUILD_PROGRAM;
  static const char* DEFAULT_TOOLSET_ROOT;

  bool ComputeTargetBuildOrder(cmGeneratorTarget const* tgt,
                               std::vector<cmGeneratorTarget const*>& build);
  bool ComputeTargetBuildOrder(std::vector<cmGeneratorTarget const*>& tgt,
                               std::vector<cmGeneratorTarget const*>& build);
  bool VisitTarget(std::set<cmGeneratorTarget const*>& temp,
                   std::set<cmGeneratorTarget const*>& perm,
                   std::vector<cmGeneratorTarget const*>& order,
                   cmGeneratorTarget const* ti);

  std::vector<cmGeneratorTarget const*> ProjectTargets;

  // Target sorting
  class TargetSet : public std::set<cmGeneratorTarget const*>
  {
  };
  class TargetCompare
  {
    std::string First;

  public:
    TargetCompare(std::string first)
      : First(std::move(first))
    {
    }
    bool operator()(cmGeneratorTarget const* l,
                    cmGeneratorTarget const* r) const;
  };
  class OrderedTargetDependSet;
};

class cmGlobalGhsMultiGenerator::OrderedTargetDependSet
  : public std::multiset<cmTargetDepend,
                         cmGlobalGhsMultiGenerator::TargetCompare>
{
  using derived =
    std::multiset<cmTargetDepend, cmGlobalGhsMultiGenerator::TargetCompare>;

public:
  using TargetDependSet = cmGlobalGenerator::TargetDependSet;
  OrderedTargetDependSet(TargetDependSet const&, std::string const& first);
};

#endif
