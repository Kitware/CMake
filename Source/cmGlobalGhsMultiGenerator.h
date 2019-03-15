/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGhsMultiGenerator_h
#define cmGhsMultiGenerator_h

#include "cmGlobalGenerator.h"

#include "cmGhsMultiGpj.h"
#include "cmGlobalGeneratorFactory.h"

class cmGeneratedFileStream;

class cmGlobalGhsMultiGenerator : public cmGlobalGenerator
{
public:
  // The default filename extension of GHS MULTI's build files.
  static const char* FILE_EXTENSION;

  cmGlobalGhsMultiGenerator(cmake* cm);
  ~cmGlobalGhsMultiGenerator();

  static cmGlobalGeneratorFactory* NewFactory()
  {
    return new cmGlobalGeneratorSimpleFactory<cmGlobalGhsMultiGenerator>();
  }

  ///! create the correct local generator
  cmLocalGenerator* CreateLocalGenerator(cmMakefile* mf) override;

  /// @return the name of this generator.
  static std::string GetActualName() { return "Green Hills MULTI"; }

  ///! Get the name for this generator
  std::string GetName() const override { return this->GetActualName(); }

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
  bool SetGeneratorToolset(std::string const& ts, cmMakefile* mf) override;
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

  // Target dependency sorting
  class TargetSet : public std::set<cmGeneratorTarget const*>
  {
  };
  class TargetCompare
  {
    std::string First;

  public:
    TargetCompare(std::string const& first)
      : First(first)
    {
    }
    bool operator()(cmGeneratorTarget const* l,
                    cmGeneratorTarget const* r) const;
  };
  class OrderedTargetDependSet;

protected:
  void Generate() override;
  void GenerateBuildCommand(GeneratedMakeCommand& makeCommand,
                            const std::string& makeProgram,
                            const std::string& projectName,
                            const std::string& projectDir,
                            const std::string& targetName,
                            const std::string& config, bool fast, int jobs,
                            bool verbose,
                            std::vector<std::string> const& makeOptions =
                              std::vector<std::string>()) override;

private:
  void GetToolset(cmMakefile* mf, std::string& tsd, const std::string& ts);

  /* top-level project */
  void OutputTopLevelProject(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  void WriteTopLevelProject(std::ostream& fout, cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  void WriteMacros(std::ostream& fout);
  void WriteHighLevelDirectives(std::ostream& fout);
  void WriteSubProjects(std::ostream& fout, cmLocalGenerator* root,
                        std::vector<cmLocalGenerator*>& generators);

  std::string trimQuotes(std::string const& str);

  static const char* DEFAULT_BUILD_PROGRAM;
  static const char* DEFAULT_TOOLSET_ROOT;
};

class cmGlobalGhsMultiGenerator::OrderedTargetDependSet
  : public std::multiset<cmTargetDepend,
                         cmGlobalGhsMultiGenerator::TargetCompare>
{
  typedef std::multiset<cmTargetDepend,
                        cmGlobalGhsMultiGenerator::TargetCompare>
    derived;

public:
  typedef cmGlobalGenerator::TargetDependSet TargetDependSet;
  OrderedTargetDependSet(TargetDependSet const&, std::string const& first);
};

#endif
