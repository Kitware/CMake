/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallGenerator_h
#define cmInstallGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallType.h"
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
                     bool exclude_from_all);
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
    const char* literal_args = nullptr, Indent indent = Indent());

  /** Get the install destination as it should appear in the
      installation script.  */
  std::string ConvertToAbsoluteDestination(std::string const& dest) const;

  /** Test if this generator installs something for a given configuration.  */
  bool InstallsForConfig(const std::string& config);

  /** Select message level from CMAKE_INSTALL_MESSAGE or 'never'.  */
  static MessageLevel SelectMessageLevel(cmMakefile* mf, bool never = false);

  virtual bool Compute(cmLocalGenerator*) { return true; }

protected:
  void GenerateScript(std::ostream& os) override;

  std::string CreateComponentTest(const std::string& component,
                                  bool exclude_from_all);

  // Information shared by most generator types.
  std::string const Destination;
  std::string const Component;
  MessageLevel const Message;
  bool const ExcludeFromAll;
};

#endif
