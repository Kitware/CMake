/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include "cmDiagnosticContext.h"
#include "cmInstallType.h"
#include "cmScriptGenerator.h"

class cmLocalGenerator;
class cmMakefile;
class cmListFileBacktrace;
class cmGeneratorTarget;
class cmGlobalGenerator;

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator : public cmScriptGenerator
{
public:
  enum class BuildDependencyType
  {
    Normal,
    InstallArtifact,
    GeneratedFile,
  };

  struct BuildDependency
  {
    BuildDependency(cmGeneratorTarget const* target, BuildDependencyType type,
                    std::string file = std::string())
      : Target(target)
      , Type(type)
      , File(std::move(file))
    {
    }

    cmGeneratorTarget const* Target;
    BuildDependencyType Type;
    std::string File; // only for GeneratedFile dependencies
  };

  enum MessageLevel
  {
    MessageDefault,
    MessageAlways,
    MessageLazy,
    MessageNever
  };

  cmInstallGenerator(std::string destination,
                     std::vector<std::string> const& configurations,
                     std::string component, MessageLevel message,
                     bool excludeFromAll, bool allComponents,
                     cmDiagnosticContext context);
  ~cmInstallGenerator() override;

  cmInstallGenerator(cmInstallGenerator const&) = delete;
  cmInstallGenerator& operator=(cmInstallGenerator const&) = delete;

  virtual bool HaveInstall();
  virtual void CheckCMP0082(bool& haveSubdirectoryInstall,
                            bool& haveInstallAfterSubdirectory);

  void AddInstallRule(
    std::ostream& os, std::string const& dest, cmInstallType type,
    std::vector<std::string> const& files, bool optional = false,
    char const* permissionsFile = nullptr,
    char const* permissionsDir = nullptr, char const* rename = nullptr,
    char const* literalArgs = nullptr, Indent indent = Indent(),
    char const* filesVar = nullptr);

  /** Get the install destination as it should appear in the
      installation script.  */
  static std::string ConvertToAbsoluteDestination(std::string const& dest);
  void CheckAbsoluteDestination(std::string const& dest,
                                cmLocalGenerator const* lg) const;

  /** Test if this generator installs something for a given configuration.  */
  bool InstallsForConfig(std::string const& config);

  /** Select message level from CMAKE_INSTALL_MESSAGE or 'never'.  */
  static MessageLevel SelectMessageLevel(cmMakefile* mf, bool never = false);

  /** Capture context for generator.  */
  static cmDiagnosticContext CaptureContext(cmMakefile const& mf);
  static cmDiagnosticContext CaptureContext(cmMakefile const* mf)
  {
    return cmInstallGenerator::CaptureContext(*mf);
  }

  virtual bool Compute(cmLocalGenerator*) { return true; }

  virtual void GetInstallBuildDependencies(std::string const&,
                                           std::vector<BuildDependency>&) const
  {
  }

  virtual void GetInstallBuildDependencyComponents(
    std::vector<std::string>& components) const
  {
    if (!this->Component.empty()) {
      components.push_back(this->Component);
    }
  }

  std::string const& GetComponent() const { return this->Component; }

  bool GetExcludeFromAll() const { return this->ExcludeFromAll; }
  bool GetAllComponentsFlag() const { return this->AllComponents; }

  cmListFileBacktrace const& GetBacktrace() const
  {
    return this->Context.GetBacktrace();
  }

  cmDiagnosticContext const& GetDiagnosticContext() const
  {
    return this->Context;
  }

  static std::string GetDestDirPath(std::string const& file);

protected:
  void GenerateScript(std::ostream& os) override;

  std::string CreateComponentTest(std::string const& component,
                                  bool excludeFromAll,
                                  bool allComponents = false);

  using TweakMethod =
    std::function<void(std::ostream& os, Indent indent,
                       std::string const& config, std::string const& file)>;
  static void AddTweak(std::ostream& os, Indent indent,
                       std::string const& config, std::string const& file,
                       TweakMethod const& tweak);
  static void AddTweak(std::ostream& os, Indent indent,
                       std::string const& config, std::string const& dir,
                       std::vector<std::string> const& files,
                       TweakMethod const& tweak);

  // BUILD_DEPENDS entries of an install(<signature>) rule, resolved.
  struct BuildDependsEntries
  {
    std::vector<cmGeneratorTarget const*> Targets;
    std::vector<std::string> Files;
  };

  // Resolve each BUILD_DEPENDS entry to a target or file.  Returns false after
  // reporting an error if an entry names neither a target nor a file that
  // exists or that the build produces.
  bool ResolveBuildDepends(cmLocalGenerator* lg, std::string const& signature,
                           std::vector<std::string> const& names,
                           BuildDependsEntries& entries);

  // Add dependencies for resolved BUILD_DEPENDS entries.
  static void AddBuildDependsDependencies(
    cmLocalGenerator* lg, std::string const& config,
    BuildDependsEntries const& entries,
    std::vector<BuildDependency>& dependencies);

  // Add dependencies for the files that the build produces among the given
  // files.  A file linked by a target depends on building that target.  Any
  // other generated file is attributed to the target owning the custom
  // command that produces it or, if no single target owns it, such as for a
  // byproduct, to the installing target, if any.
  static void AddGeneratedFileDependencies(
    cmGlobalGenerator* gg, std::string const& config,
    cmGeneratorTarget const* target, std::vector<std::string> const& files,
    std::vector<BuildDependency>& dependencies);

  // Information shared by most generator types.
  std::string Destination;
  std::string const Component;
  MessageLevel const Message;
  bool const ExcludeFromAll;
  bool const AllComponents;
  cmDiagnosticContext const Context;
};
