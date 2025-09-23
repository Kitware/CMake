/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmGlobalVisualStudio7Generator.h"
#include "cmValue.h"

class cmGeneratorTarget;
class cmLocalGenerator;
class cmake;
template <typename T>
class BT;

/** \class cmGlobalVisualStudio71Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio71Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio71Generator : public cmGlobalVisualStudio7Generator
{
public:
  cmGlobalVisualStudio71Generator(cmake* cm);

protected:
  void WriteSLNFile(std::ostream& fout, cmLocalGenerator* root,
                    std::vector<cmLocalGenerator*>& generators) override;
  virtual void WriteSolutionConfigurations(
    std::ostream& fout, std::vector<std::string> const& configs);
  void WriteProject(std::ostream& fout, std::string const& name,
                    std::string const& path,
                    cmGeneratorTarget const* t) override;
  void WriteProjectDepends(std::ostream& fout, std::string const& name,
                           std::string const& path,
                           cmGeneratorTarget const* t) override;
  void WriteProjectConfigurations(
    std::ostream& fout, std::string const& name,
    cmGeneratorTarget const& target, std::vector<std::string> const& configs,
    std::set<std::string> const& configsPartOfDefaultBuild,
    std::string const& platformMapping = "") override;
  void WriteExternalProject(
    std::ostream& fout, std::string const& name, std::string const& path,
    cmValue typeGuid,
    std::set<BT<std::pair<std::string, bool>>> const& depends) override;

  // Folders are not supported by VS 7.1.
  bool UseFolderProperty() const override { return false; }

  std::string ProjectConfigurationSectionName;
};
