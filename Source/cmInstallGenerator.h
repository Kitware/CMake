/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmScriptGenerator.h"

class cmLocalGenerator;
class cmMakefile;

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator : public cmScriptGenerator
{
public:
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
                     bool exclude_from_all, bool all_components,
                     cmListFileBacktrace backtrace);
  ~cmInstallGenerator() override;

  cmInstallGenerator(cmInstallGenerator const&) = delete;
  cmInstallGenerator& operator=(cmInstallGenerator const&) = delete;

  virtual bool HaveInstall();
  virtual void CheckCMP0082(bool& haveSubdirectoryInstall,
                            bool& haveInstallAfterSubdirectory);

  void AddInstallRule(
    std::ostream& os, std::string const& dest, cmInstallType type,
    std::vector<std::string> const& files, bool optional = false,
    const char* permissions_file = nullptr,
    const char* permissions_dir = nullptr, const char* rename = nullptr,
    const char* literal_args = nullptr, Indent indent = Indent(),
    const char* files_var = nullptr);

  /** Get the install destination as it should appear in the
      installation script.  */
  static std::string ConvertToAbsoluteDestination(std::string const& dest);

  /** Test if this generator installs something for a given configuration.  */
  bool InstallsForConfig(const std::string& config);

  /** Select message level from CMAKE_INSTALL_MESSAGE or 'never'.  */
  static MessageLevel SelectMessageLevel(cmMakefile* mf, bool never = false);

  virtual bool Compute(cmLocalGenerator*) { return true; }

  std::string const& GetComponent() const { return this->Component; }

  bool GetExcludeFromAll() const { return this->ExcludeFromAll; }
  bool GetAllComponentsFlag() const { return this->AllComponents; }

  cmListFileBacktrace const& GetBacktrace() const { return this->Backtrace; }

  static std::string GetDestDirPath(std::string const& file);

protected:
  void GenerateScript(std::ostream& os) override;

  std::string CreateComponentTest(const std::string& component,
                                  bool exclude_from_all,
                                  bool all_components = false);

  using TweakMethod =
    std::function<void(std::ostream& os, Indent indent,
                       const std::string& config, const std::string& file)>;
  static void AddTweak(std::ostream& os, Indent indent,
                       const std::string& config, std::string const& file,
                       const TweakMethod& tweak);
  static void AddTweak(std::ostream& os, Indent indent,
                       const std::string& config, std::string const& dir,
                       std::vector<std::string> const& files,
                       const TweakMethod& tweak);

  // Information shared by most generator types.
  std::string const Destination;
  std::string const Component;
  MessageLevel const Message;
  bool const ExcludeFromAll;
  bool const AllComponents;
  cmListFileBacktrace const Backtrace;
};
