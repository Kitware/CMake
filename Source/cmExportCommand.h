/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmExportCommand_h
#define cmExportCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"
#include "cmCommandArgumentsHelper.h"

class cmExecutionStatus;
class cmExportSet;

/** \class cmExportLibraryDependenciesCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmExportLibraryDependenciesCommand adds a test to the list of tests to run
 *
 */
class cmExportCommand : public cmCommand
{
public:
  cmExportCommand();
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmExportCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "export"; }

private:
  cmCommandArgumentsHelper Helper;
  cmCommandArgumentGroup ArgumentGroup;
  cmCAStringVector Targets;
  cmCAEnabler Append;
  cmCAString ExportSetName;
  cmCAString Namespace;
  cmCAString Filename;
  cmCAEnabler ExportOld;
  cmCAString AndroidMKFile;

  cmExportSet* ExportSet;

  friend class cmExportBuildFileGenerator;
  std::string ErrorMessage;

  bool HandlePackage(std::vector<std::string> const& args);
  void StorePackageRegistryWin(std::string const& package, const char* content,
                               const char* hash);
  void StorePackageRegistryDir(std::string const& package, const char* content,
                               const char* hash);
  void ReportRegistryError(std::string const& msg, std::string const& key,
                           long err);
};

#endif
