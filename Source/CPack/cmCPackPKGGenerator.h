/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <set>
#include <sstream>
#include <string>

#include <cm/string_view>

#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"

class cmXMLWriter;

/** \class cmCPackPKGGenerator
 * \brief A generator for pkg files
 *
 */
class cmCPackPKGGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackPKGGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackPKGGenerator();
  ~cmCPackPKGGenerator() override;

  bool SupportsComponentInstallation() const override;

protected:
  int InitializeInternal() override;
  char const* GetOutputPostfix() override { return "darwin"; }

  // Copies or creates the resource file with the given name to the
  // package or package staging directory dirName. The variable
  // CPACK_RESOURCE_FILE_${NAME} (where ${NAME} is the uppercased
  // version of name) specifies the input file to use for this file,
  // which will be configured via ConfigureFile.
  bool CopyCreateResourceFile(std::string const& name,
                              std::string const& dirName);
  bool CopyResourcePlistFile(std::string const& name,
                             char const* outName = nullptr);

  int CopyInstallScript(std::string const& resdir, std::string const& script,
                        std::string const& name);

  // Retrieve the name of package file that will be generated for this
  // component. The name is just the file name with extension, and
  // does not include the subdirectory.
  std::string GetPackageName(cmCPackComponent const& component);

  // Writes a distribution.dist file, which turns a metapackage into a
  // full-fledged distribution. This file is used to describe
  // inter-component dependencies. metapackageFile is the name of the
  // metapackage for the distribution. Only valid for a
  // component-based install.
  void WriteDistributionFile(char const* metapackageFile, char const* genName);

  // Subroutine of WriteDistributionFile that writes out the
  // dependency attributes for inter-component dependencies.
  void AddDependencyAttributes(cmCPackComponent const& component,
                               std::set<cmCPackComponent const*>& visited,
                               std::ostringstream& out);

  // Subroutine of WriteDistributionFile that writes out the
  // reverse dependency attributes for inter-component dependencies.
  void AddReverseDependencyAttributes(
    cmCPackComponent const& component,
    std::set<cmCPackComponent const*>& visited, std::ostringstream& out);

  // Generates XML that encodes the hierarchy of component groups and
  // their components in a form that can be used by distribution
  // metapackages.
  void CreateChoiceOutline(cmCPackComponentGroup const& group,
                           cmXMLWriter& xout);

  /// Create the "choice" XML element to describe a component group
  /// for the installer GUI.
  void CreateChoice(cmCPackComponentGroup const& group, cmXMLWriter& xout);

  /// Create the "choice" XML element to describe a component for the
  /// installer GUI.
  void CreateChoice(cmCPackComponent const& component, cmXMLWriter& xout);

  /// Creates a background in the distribution XML.
  void CreateBackground(char const* themeName, char const* metapackageFile,
                        cm::string_view genName, cmXMLWriter& xout);

  /// Create the "domains" XML element to indicate where the product can
  /// be installed
  void CreateDomains(cmXMLWriter& xout);

  // The PostFlight component when creating a metapackage
  cmCPackComponent PostFlightComponent;
};
