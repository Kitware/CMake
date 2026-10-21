/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/optional>

namespace cm {
namespace StdIo {
class Console;
}
}

namespace cmcmd {
/**
 * Execute commands during the build process. Supports options such
 * as echo, remove file etc.
 */
int ExecuteCMakeCommand(std::vector<std::string> args,
                        cm::optional<cm::StdIo::Console> console);
}
