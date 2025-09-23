/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/optional>

#include "cmsys/Status.hxx"

#include "cmCryptoHash.h"

namespace cm {
namespace StdIo {
class Console;
}
}

class cmcmd
{
public:
  /**
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int ExecuteCMakeCommand(std::vector<std::string> const&,
                                 cm::optional<cm::StdIo::Console> console);

protected:
  static int HandleCoCompileCommands(std::vector<std::string> const& args);
  static int HashSumFile(std::vector<std::string> const& args,
                         cmCryptoHash::Algo algo);
  static int SymlinkLibrary(std::vector<std::string> const& args);
  static int SymlinkExecutable(std::vector<std::string> const& args);
  static cmsys::Status SymlinkInternal(std::string const& file,
                                       std::string const& link);
  static int ExecuteEchoColor(std::vector<std::string> const& args);
  static int ExecuteLinkScript(std::vector<std::string> const& args);
  static int WindowsCEEnvironment(char const* version,
                                  std::string const& name);
  static int RunPreprocessor(std::vector<std::string> const& command,
                             std::string const& intermediate_file);
  static int RunLLVMRC(std::vector<std::string> const& args);
  static int VisualStudioLink(std::vector<std::string> const& args, int type,
                              cm::optional<cm::StdIo::Console> console);
};
