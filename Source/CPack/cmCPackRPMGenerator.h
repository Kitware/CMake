/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmCPackGenerator.h"

/** \class cmCPackRPMGenerator
 * \brief A generator for RPM packages
 * The idea of the CPack RPM generator is to use
 * as minimal C++ code as possible.
 * Ideally the C++ part of the CPack RPM generator
 * will only 'execute' (aka ->ReadListFile) several
 * CMake macros files.
 */
class cmCPackRPMGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackRPMGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackRPMGenerator();
  ~cmCPackRPMGenerator() override;

  static bool CanGenerate()
  {
#ifdef __APPLE__
    // on MacOS enable CPackRPM iff rpmbuild is found
    std::vector<std::string> locations;
    locations.emplace_back("/sw/bin");        // Fink
    locations.emplace_back("/opt/local/bin"); // MacPorts
    return !cmSystemTools::FindProgram("rpmbuild").empty();
#else
    // legacy behavior on other systems
    return true;
#endif
  }

protected:
  int InitializeInternal() override;
  int PackageFiles() override;
  /**
   * This method factors out the work done in component packaging case.
   */
  int PackageOnePack(std::string const& initialToplevel,
                     std::string const& packageName);
  /**
   * The method used to package files when component
   * install is used. This will create one
   * archive for each component group.
   */
  int PackageComponents(bool ignoreGroup);
  /**
   * Special case of component install where all
   * components will be put in a single installer.
   */
  int PackageComponentsAllInOne(const std::string& compInstDirName);
  const char* GetOutputExtension() override { return ".rpm"; }
  bool SupportsComponentInstallation() const override;
  std::string GetComponentInstallDirNameSuffix(
    const std::string& componentName) override;

  void AddGeneratedPackageNames();
};
