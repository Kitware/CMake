/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalVisualStudio11Generator_h
#define cmGlobalVisualStudio11Generator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <set>
#include <string>

#include "cmGlobalVisualStudio10Generator.h"
#include "cmStateTypes.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudio11Generator  */
class cmGlobalVisualStudio11Generator : public cmGlobalVisualStudio10Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  bool MatchesGeneratorName(const std::string& name) const override;

protected:
  cmGlobalVisualStudio11Generator(cmake* cm, const std::string& name,
                                  std::string const& platformInGeneratorName);

  bool InitializeWindowsPhone(cmMakefile* mf) override;
  bool InitializeWindowsStore(cmMakefile* mf) override;
  bool SelectWindowsPhoneToolset(std::string& toolset) const override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  virtual bool IsWindowsDesktopToolsetInstalled() const;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsPhoneToolsetInstalled() const;
  bool IsWindowsStoreToolsetInstalled() const;

  bool UseFolderProperty() const override;
  static std::set<std::string> GetInstalledWindowsCESDKs();

  /** Return true if target system supports debugging deployment. */
  bool TargetSystemSupportsDeployment() const override;

private:
  class Factory;
  friend class Factory;
};
#endif
