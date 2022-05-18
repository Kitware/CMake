/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmGlobalVisualStudio11Generator.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudio12Generator  */
class cmGlobalVisualStudio12Generator : public cmGlobalVisualStudio11Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  bool MatchesGeneratorName(const std::string& name) const override;

protected:
  cmGlobalVisualStudio12Generator(cmake* cm, const std::string& name,
                                  std::string const& platformInGeneratorName);

  bool ProcessGeneratorToolsetField(std::string const& key,
                                    std::string const& value) override;

  bool InitializeWindowsPhone(cmMakefile* mf) override;
  bool InitializeWindowsStore(cmMakefile* mf) override;
  bool SelectWindowsPhoneToolset(std::string& toolset) const override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsPhoneToolsetInstalled() const;
  bool IsWindowsStoreToolsetInstalled() const;

private:
  class Factory;
  friend class Factory;
};
