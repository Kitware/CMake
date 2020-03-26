/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalVisualStudio14Generator_h
#define cmGlobalVisualStudio14Generator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#include "cmGlobalVisualStudio12Generator.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudio14Generator  */
class cmGlobalVisualStudio14Generator : public cmGlobalVisualStudio12Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  bool MatchesGeneratorName(const std::string& name) const override;

protected:
  cmGlobalVisualStudio14Generator(cmake* cm, const std::string& name,
                                  std::string const& platformInGeneratorName);

  bool InitializeWindows(cmMakefile* mf) override;
  bool InitializeWindowsStore(cmMakefile* mf) override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  // Used to make sure that the Windows 10 SDK selected can work with the
  // version of the toolset.
  virtual std::string GetWindows10SDKMaxVersion() const;

  virtual bool SelectWindows10SDK(cmMakefile* mf, bool required);

  void SetWindowsTargetPlatformVersion(std::string const& version,
                                       cmMakefile* mf);

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  std::string GetWindows10SDKVersion();

private:
  class Factory;
  friend class Factory;
};
#endif
