/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmCPackGenerator.h"
#include "cmCPackPKGGenerator.h"
#include "cmValue.h"

class cmCPackComponent;

/** \class cmCPackProductBuildGenerator
 * \brief A generator for ProductBuild files
 *
 */
class cmCPackProductBuildGenerator : public cmCPackPKGGenerator
{
public:
  cmCPackTypeMacro(cmCPackProductBuildGenerator, cmCPackPKGGenerator);

  /**
   * Construct generator
   */
  cmCPackProductBuildGenerator();
  ~cmCPackProductBuildGenerator() override;

protected:
  int InitializeInternal() override;
  int PackageFiles() override;
  char const* GetOutputExtension() override { return ".pkg"; }

  // Run ProductBuild with the given command line, which will (if
  // successful) produce the given package file. Returns true if
  // ProductBuild succeeds, false otherwise.
  bool RunProductBuild(std::string const& command);

  // Generate a package in the file packageFile for the given
  // component.  All of the files within this component are stored in
  // the directory packageDir. Returns true if successful, false
  // otherwise.
  bool GenerateComponentPackage(std::string const& packageFileDir,
                                std::string const& packageFileName,
                                std::string const& packageDir,
                                cmCPackComponent const* component);

  cmValue GetComponentScript(char const* script, char const* script_component);
};
