/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallCommand_h
#define cmInstallCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;
class cmInstallCommandArguments;

/** \class cmInstallCommand
 * \brief Specifies where to install some files
 *
 * cmInstallCommand is a general-purpose interface command for
 * specifying install rules.
 */
class cmInstallCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() override { return new cmInstallCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

private:
  bool HandleScriptMode(std::vector<std::string> const& args);
  bool HandleTargetsMode(std::vector<std::string> const& args);
  bool HandleFilesMode(std::vector<std::string> const& args);
  bool HandleDirectoryMode(std::vector<std::string> const& args);
  bool HandleExportMode(std::vector<std::string> const& args);
  bool HandleExportAndroidMKMode(std::vector<std::string> const& args);
  bool MakeFilesFullPath(const char* modeName,
                         const std::vector<std::string>& relFiles,
                         std::vector<std::string>& absFiles);
  bool CheckCMP0006(bool& failure);

  std::string GetDestination(const cmInstallCommandArguments* args,
                             const std::string& varName,
                             const std::string& guess);
  std::string GetRuntimeDestination(const cmInstallCommandArguments* args);
  std::string GetSbinDestination(const cmInstallCommandArguments* args);
  std::string GetArchiveDestination(const cmInstallCommandArguments* args);
  std::string GetLibraryDestination(const cmInstallCommandArguments* args);
  std::string GetIncludeDestination(const cmInstallCommandArguments* args);
  std::string GetSysconfDestination(const cmInstallCommandArguments* args);
  std::string GetSharedStateDestination(const cmInstallCommandArguments* args);
  std::string GetLocalStateDestination(const cmInstallCommandArguments* args);
  std::string GetRunStateDestination(const cmInstallCommandArguments* args);
  std::string GetDataRootDestination(const cmInstallCommandArguments* args);
  std::string GetDataDestination(const cmInstallCommandArguments* args);
  std::string GetInfoDestination(const cmInstallCommandArguments* args);
  std::string GetLocaleDestination(const cmInstallCommandArguments* args);
  std::string GetManDestination(const cmInstallCommandArguments* args);
  std::string GetDocDestination(const cmInstallCommandArguments* args);
  std::string GetDestinationForType(const cmInstallCommandArguments* args,
                                    const std::string& type);

  std::string DefaultComponentName;
};

#endif
