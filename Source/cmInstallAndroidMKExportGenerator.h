/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmExportSet;
class cmListFileBacktrace;

/** \class cmInstallAndroidMKExportGenerator
 * \brief Generate rules for creating Android .mk export files.
 */
class cmInstallAndroidMKExportGenerator : public cmInstallExportGenerator
{
public:
  cmInstallAndroidMKExportGenerator(
    cmExportSet* exportSet, std::string destination,
    std::string filePermissions,
    std::vector<std::string> const& configurations, std::string component,
    MessageLevel message, bool excludeFromAll, std::string filename,
    std::string targetNamespace, cmListFileBacktrace backtrace);
  cmInstallAndroidMKExportGenerator(cmInstallAndroidMKExportGenerator const&) =
    delete;
  ~cmInstallAndroidMKExportGenerator() override;

  cmInstallAndroidMKExportGenerator& operator=(
    cmInstallAndroidMKExportGenerator const&) = delete;

  char const* InstallSubcommand() const override
  {
    return "EXPORT_ANDROID_MK";
  }
};
