/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

struct cmTestDiscoveryArgs;
class cmCTestTestHandler;
class cmExecutionStatus;

bool cmCTestDiscoverTests(cmTestDiscoveryArgs const& args,
                          cmCTestTestHandler* handler,
                          cmExecutionStatus& status);
