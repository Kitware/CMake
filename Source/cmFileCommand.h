/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileCommand_h
#define cmFileCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cm_memory.hxx"

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmFileCommand
 * \brief Command for manipulation of files
 *
 */
class cmFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmFileCommand>();
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

protected:
  bool HandleRename(std::vector<std::string> const& args,
                    cmExecutionStatus& status);
  bool HandleRemove(std::vector<std::string> const& args, bool recurse,
                    cmExecutionStatus& status);
  bool HandleWriteCommand(std::vector<std::string> const& args, bool append,
                          cmExecutionStatus& status);
  bool HandleReadCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status);
  bool HandleHashCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status);
  bool HandleStringsCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
  bool HandleGlobCommand(std::vector<std::string> const& args, bool recurse,
                         cmExecutionStatus& status);
  bool HandleTouchCommand(std::vector<std::string> const& args, bool create,
                          cmExecutionStatus& status);
  bool HandleMakeDirectoryCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status);

  bool HandleRelativePathCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status);
  bool HandleCMakePathCommand(std::vector<std::string> const& args,
                              bool nativePath, cmExecutionStatus& status);
  bool HandleReadElfCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
  bool HandleRPathChangeCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status);
  bool HandleRPathCheckCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status);
  bool HandleRPathRemoveCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status);
  bool HandleDifferentCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status);

  bool HandleCopyCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status);
  bool HandleInstallCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status);
  bool HandleDownloadCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status);
  bool HandleUploadCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status);

  bool HandleTimestampCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status);
  bool HandleGenerateCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status);
  bool HandleLockCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status);
  bool HandleSizeCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status);
  bool HandleReadSymlinkCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status);
  bool HandleCreateLinkCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status);
  bool HandleGetRuntimeDependenciesCommand(
    std::vector<std::string> const& args, cmExecutionStatus& status);

private:
  void AddEvaluationFile(const std::string& inputName,
                         const std::string& outputExpr,
                         const std::string& condition, bool inputIsContent,
                         cmExecutionStatus& status);
};

#endif
