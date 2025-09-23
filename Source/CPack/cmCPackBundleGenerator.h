/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmCPackDragNDropGenerator.h"
#include "cmCPackGenerator.h"

/** \class cmCPackBundleGenerator
 * \brief A generator for OSX bundles
 *
 * Based on Gimp.app
 */
class cmCPackBundleGenerator : public cmCPackDragNDropGenerator
{
public:
  cmCPackTypeMacro(cmCPackBundleGenerator, cmCPackDragNDropGenerator);

  cmCPackBundleGenerator();
  ~cmCPackBundleGenerator() override;

protected:
  int InitializeInternal() override;
  char const* GetPackagingInstallPrefix() override;
  int ConstructBundle();
  int SignBundle(std::string const& src_dir);
  int PackageFiles() override;
  bool SupportsComponentInstallation() const override;

  std::string InstallPrefix;
};
