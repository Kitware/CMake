/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmcmd_h
#define cmcmd_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCryptoHash.h"

class cmcmd
{
public:
  /**
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int ExecuteCMakeCommand(std::vector<std::string> const&);

protected:
  static int HandleCoCompileCommands(std::vector<std::string> const& args);
  static int HashSumFile(std::vector<std::string> const& args,
                         cmCryptoHash::Algo algo);
  static int SymlinkLibrary(std::vector<std::string> const& args);
  static int SymlinkExecutable(std::vector<std::string> const& args);
  static bool SymlinkInternal(std::string const& file,
                              std::string const& link);
  static int ExecuteEchoColor(std::vector<std::string> const& args);
  static int ExecuteLinkScript(std::vector<std::string> const& args);
  static int WindowsCEEnvironment(const char* version,
                                  const std::string& name);
  static int RunPreprocessor(const std::vector<std::string>& command,
                             const std::string& intermediate_file);
  static int RunLLVMRC(std::vector<std::string> const& args);
  static int VisualStudioLink(std::vector<std::string> const& args, int type);
};

#endif
