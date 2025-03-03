/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestReadCustomFilesCommand.h"

#include "cmCTest.h"
#include "cmExecutionStatus.h"

class cmMakefile;

bool cmCTestReadCustomFilesCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus& status) const
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  for (std::string const& arg : args) {
    this->CTest->ReadCustomConfigurationFileTree(arg, &mf);
  }

  return true;
}
