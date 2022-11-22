/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cm_codecvt.hxx"

#include "cmGlobalGenerator.h"
#include "cmTargetDepend.h"
#include "cmValue.h"

class cmCustomCommand;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudioGenerator
 * \brief Base class for global Visual Studio generators.
 *
 * cmGlobalVisualStudioGenerator provides functionality common to all
 * global Visual Studio generators.
 */
class cmGlobalVisualStudioGenerator : public cmGlobalGenerator
{
public:
  /** Known versions of Visual Studio.  */
  enum class VSVersion : uint16_t
  {
    VS9 = 90,
    VS11 = 110,
    VS12 = 120,
    /* VS13 = 130 was skipped */
    VS14 = 140,
    VS15 = 150,
    VS16 = 160,
    VS17 = 170
  };

  ~cmGlobalVisualStudioGenerator() override;

  VSVersion GetVersion() const;
  void SetVersion(VSVersion v);

  /** Is the installed VS an Express edition?  */
  bool IsExpressEdition() const { return this->ExpressEdition; }

  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf) override;

  /**
   * Get the name of the target platform (architecture) for which we generate.
   * The names are as defined by VS, e.g. "Win32", "x64", "Itanium", "ARM".
   */
  std::string const& GetPlatformName() const;

  /**
   * Configure CMake's Visual Studio macros file into the user's Visual
   * Studio macros directory.
   */
  virtual void ConfigureCMakeVisualStudioMacros();

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();

  /**
   * What is the reg key path to "vsmacros" for this version of Visual
   * Studio?
   */
  virtual std::string GetUserMacrosRegKeyBase();

  enum MacroName
  {
    MacroReload,
    MacroStop
  };

  /**
   * Call the ReloadProjects macro if necessary based on
   * GetFilesReplacedDuringGenerate results.
   */
  void CallVisualStudioMacro(MacroName m, const std::string& vsSolutionFile);

  // return true if target is fortran only
  bool TargetIsFortranOnly(const cmGeneratorTarget* gt);

  // return true if target should be included in solution.
  virtual bool IsInSolution(const cmGeneratorTarget* gt) const;

  // return true if project dependency should be included in solution.
  virtual bool IsDepInSolution(const std::string& targetName) const;

  /** Get the top-level registry key for this VS version.  */
  std::string GetRegistryBase();

  /** Get the top-level registry key for the given VS version.  */
  static std::string GetRegistryBase(const char* version);

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  bool IsMultiConfig() const override { return true; }

  /** Return true if building for Windows CE */
  virtual bool TargetsWindowsCE() const { return false; }

  bool IsIncludeExternalMSProjectSupported() const override { return true; }

  /** Get encoding used by generator for generated source files
   */
  codecvt::Encoding GetMakefileEncoding() const override
  {
    return codecvt::ANSI;
  }

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

  bool FindMakeProgram(cmMakefile*) override;

  std::string ExpandCFGIntDir(const std::string& str,
                              const std::string& config) const override;

  void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

  std::string GetStartupProjectName(cmLocalGenerator const* root) const;

  void AddSymbolExportCommand(cmGeneratorTarget*,
                              std::vector<cmCustomCommand>& commands,
                              std::string const& configName);

  bool Open(const std::string& bindir, const std::string& projectName,
            bool dryRun) override;

  bool IsVisualStudio() const override { return true; }

protected:
  cmGlobalVisualStudioGenerator(cmake* cm,
                                std::string const& platformInGeneratorName);

  void AddExtraIDETargets() override;

  // Does this VS version link targets to each other if there are
  // dependencies in the SLN file?  This was done for VS versions
  // below 8.
  virtual bool VSLinksDependencies() const { return true; }

  const char* GetIDEVersion() const;

  void WriteSLNHeader(std::ostream& fout);

  bool ComputeTargetDepends() override;
  class VSDependSet : public std::set<std::string>
  {
  };
  class VSDependMap : public std::map<cmGeneratorTarget const*, VSDependSet>
  {
  };
  VSDependMap VSTargetDepends;
  void ComputeVSTargetDepends(cmGeneratorTarget*);

  bool CheckTargetLinks(cmGeneratorTarget& target, const std::string& name);
  std::string GetUtilityForTarget(cmGeneratorTarget& target,
                                  const std::string&);
  virtual std::string WriteUtilityDepend(cmGeneratorTarget const*) = 0;
  std::string GetUtilityDepend(const cmGeneratorTarget* target);
  using UtilityDependsMap = std::map<cmGeneratorTarget const*, std::string>;
  UtilityDependsMap UtilityDepends;

  VSVersion Version;
  bool ExpressEdition;

  std::string GeneratorPlatform;
  std::string DefaultPlatformName;
  bool PlatformInGeneratorName = false;

private:
  virtual std::string GetVSMakeProgram() = 0;
  void PrintCompilerAdvice(std::ostream&, std::string const&,
                           cmValue) const override
  {
  }

  void FollowLinkDepends(cmGeneratorTarget const* target,
                         std::set<cmGeneratorTarget const*>& linked);

  class TargetSetMap : public std::map<cmGeneratorTarget*, TargetSet>
  {
  };
  TargetSetMap TargetLinkClosure;
  void FillLinkClosure(const cmGeneratorTarget* target, TargetSet& linked);
  TargetSet const& GetTargetLinkClosure(cmGeneratorTarget* target);
};

class cmGlobalVisualStudioGenerator::OrderedTargetDependSet
  : public std::multiset<cmTargetDepend,
                         cmGlobalVisualStudioGenerator::TargetCompare>
{
  using derived = std::multiset<cmTargetDepend,
                                cmGlobalVisualStudioGenerator::TargetCompare>;

public:
  using TargetDependSet = cmGlobalGenerator::TargetDependSet;
  using TargetSet = cmGlobalVisualStudioGenerator::TargetSet;
  OrderedTargetDependSet(TargetDependSet const&, std::string const& first);
  OrderedTargetDependSet(TargetSet const&, std::string const& first);
};
