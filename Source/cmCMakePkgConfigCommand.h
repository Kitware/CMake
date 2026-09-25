/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmExecutionStatus;
class cmMakefile;

bool cmCMakePkgConfigCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status);

bool cmImportPkgConfigPackage(cmMakefile& mf, cmExecutionStatus& status,
                              std::string const& name);
