/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOptionCommand.h"

#include "cmAlgorithms.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

// cmOptionCommand
bool cmOptionCommand::InitialPass(std::vector<std::string> const& args,
                                  cmExecutionStatus&)
{
  const bool argError = (args.size() < 2) || (args.size() > 3);
  if (argError) {
    std::string m = "called with incorrect number of arguments: ";
    m += cmJoin(args, " ");
    this->SetError(m);
    return false;
  }

  std::string initialValue = "Off";
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  cmState* state = this->Makefile->GetState();
  const char* existingValue = state->GetCacheEntryValue(args[0]);
  if (existingValue) {
    if (state->GetCacheEntryType(args[0]) != cmStateEnums::UNINITIALIZED) {
      state->SetCacheEntryProperty(args[0], "HELPSTRING", args[1]);
      return true;
    }
    initialValue = existingValue;
  }
  if (args.size() == 3) {
    initialValue = args[2];
  }
  bool init = cmSystemTools::IsOn(initialValue.c_str());
  this->Makefile->AddCacheDefinition(args[0], init ? "ON" : "OFF",
                                     args[1].c_str(), cmStateEnums::BOOL);
  return true;
}
