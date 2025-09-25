/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyComputer.h"

#include "cmMakefile.h"
#include "cmMessageType.h"

void cmTargetPropertyComputer::IssueLocationPropertyError(
  std::string const& tgtName, cmMakefile const& mf)
{
  mf.IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat(
      "The LOCATION property may not be read from target \"", tgtName,
      "\".  Use the target name directly with "
      "add_custom_command, or use the generator expression $<TARGET_FILE>, "
      "as appropriate.\n"));
}
