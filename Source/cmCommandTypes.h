/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <string>
#include <vector>

class cmExecutionStatus;
struct cmListFileArgument;

namespace cm {

using Command = std::function<bool(std::vector<cmListFileArgument> const&,
                                   cmExecutionStatus&)>;
using BuiltinCommand = bool (*)(std::vector<std::string> const&,
                                cmExecutionStatus&);

};
