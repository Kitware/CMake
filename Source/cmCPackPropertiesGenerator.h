/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmScriptGenerator.h"

class cmInstalledFile;
class cmLocalGenerator;

/** \class cmCPackPropertiesGenerator
 * \brief Support class for generating CPackProperties.cmake.
 *
 */
class cmCPackPropertiesGenerator : public cmScriptGenerator
{
public:
  cmCPackPropertiesGenerator(cmLocalGenerator* lg,
                             cmInstalledFile const& installedFile,
                             std::vector<std::string> const& configurations);

  cmCPackPropertiesGenerator(cmCPackPropertiesGenerator const&) = delete;
  cmCPackPropertiesGenerator& operator=(cmCPackPropertiesGenerator const&) =
    delete;

protected:
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;

  cmLocalGenerator* LG;
  cmInstalledFile const& InstalledFile;
};
