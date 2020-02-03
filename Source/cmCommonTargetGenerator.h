/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCommonTargetGenerator_h
#define cmCommonTargetGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

class cmGeneratorTarget;
class cmGlobalCommonGenerator;
class cmLinkLineComputer;
class cmLocalCommonGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmCommonTargetGenerator
 * \brief Common infrastructure for Makefile and Ninja per-target generators
 */
class cmCommonTargetGenerator
{
public:
  cmCommonTargetGenerator(cmGeneratorTarget* gt);
  virtual ~cmCommonTargetGenerator();

  std::vector<std::string> const& GetConfigNames() const;

protected:
  // Feature query methods.
  const char* GetFeature(const std::string& feature,
                         const std::string& config);

  // Helper to add flag for windows .def file.
  void AddModuleDefinitionFlag(cmLinkLineComputer* linkLineComputer,
                               std::string& flags, const std::string& config);

  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmLocalCommonGenerator* LocalCommonGenerator;
  cmGlobalCommonGenerator* GlobalCommonGenerator;
  std::vector<std::string> ConfigNames;

  void AppendFortranFormatFlags(std::string& flags,
                                cmSourceFile const& source);

  virtual void AddIncludeFlags(std::string& flags, std::string const& lang,
                               const std::string& config) = 0;

  void AppendOSXVerFlag(std::string& flags, const std::string& lang,
                        const char* name, bool so);

  std::string GetFlags(const std::string& l, const std::string& config);
  std::string GetDefines(const std::string& l, const std::string& config);
  std::string GetIncludes(std::string const& l, const std::string& config);
  std::string GetManifests(const std::string& config);
  std::string GetAIXExports(std::string const& config);

  std::vector<std::string> GetLinkedTargetDirectories(
    const std::string& config) const;
  std::string ComputeTargetCompilePDB(const std::string& config) const;

private:
  using ByLanguageMap = std::map<std::string, std::string>;
  struct ByConfig
  {
    ByLanguageMap FlagsByLanguage;
    ByLanguageMap DefinesByLanguage;
    ByLanguageMap IncludesByLanguage;
  };
  std::map<std::string, ByConfig> Configs;
};

#endif
