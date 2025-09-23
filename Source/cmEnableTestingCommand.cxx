/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmEnableTestingCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmEnableTestingCommand(std::vector<std::string> const&,
                            cmExecutionStatus& status)
{
  status.GetMakefile().AddDefinition("CMAKE_TESTING_ENABLED", "1");
  return true;
}
