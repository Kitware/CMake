/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

// cmSetCommand
bool cmSetCommand(std::vector<std::string> const& args,
                  cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // watch for ENV signatures
  auto const& variable = args[0]; // VAR is always first
  if (cmHasLiteralPrefix(variable, "ENV{") && variable.size() > 5) {
    // what is the variable name
    auto const& varName = variable.substr(4, variable.size() - 5);
    std::string putEnvArg = varName + "=";

    // what is the current value if any
    std::string currValue;
    const bool currValueSet = cmSystemTools::GetEnv(varName, currValue);

    // will it be set to something, then set it
    if (args.size() > 1 && !args[1].empty()) {
      // but only if it is different from current value
      if (!currValueSet || currValue != args[1]) {
        putEnvArg += args[1];
        cmSystemTools::PutEnv(putEnvArg);
      }
      // if there's extra arguments, warn user
      // that they are ignored by this command.
      if (args.size() > 2) {
        std::string m = "Only the first value argument is used when setting "
                        "an environment variable.  Argument '" +
          args[2] + "' and later are unused.";
        status.GetMakefile().IssueMessage(MessageType::AUTHOR_WARNING, m);
      }
      return true;
    }

    // if it will be cleared, then clear it if it isn't already clear
    if (currValueSet) {
      cmSystemTools::PutEnv(putEnvArg);
    }
    return true;
  }

  // SET (VAR) // Removes the definition of VAR.
  if (args.size() == 1) {
    status.GetMakefile().RemoveDefinition(variable);
    return true;
  }
  // SET (VAR PARENT_SCOPE) // Removes the definition of VAR
  // in the parent scope.
  if (args.size() == 2 && args.back() == "PARENT_SCOPE") {
    status.GetMakefile().RaiseScope(variable, nullptr);
    return true;
  }

  // here are the remaining options
  //  SET (VAR value )
  //  SET (VAR value PARENT_SCOPE)
  //  SET (VAR CACHE TYPE "doc String" [FORCE])
  //  SET (VAR value CACHE TYPE "doc string" [FORCE])
  std::string value;  // optional
  bool cache = false; // optional
  bool force = false; // optional
  bool parentScope = false;
  cmStateEnums::CacheEntryType type =
    cmStateEnums::STRING; // required if cache
  cmValue docstring;      // required if cache

  unsigned int ignoreLastArgs = 0;
  // look for PARENT_SCOPE argument
  if (args.size() > 1 && args.back() == "PARENT_SCOPE") {
    parentScope = true;
    ignoreLastArgs++;
  } else {
    // look for FORCE argument
    if (args.size() > 4 && args.back() == "FORCE") {
      force = true;
      ignoreLastArgs++;
    }

    // check for cache signature
    if (args.size() > 3 &&
        args[args.size() - 3 - (force ? 1 : 0)] == "CACHE") {
      cache = true;
      ignoreLastArgs += 3;
    }
  }

  // collect any values into a single semi-colon separated value list
  value = cmJoin(cmMakeRange(args).advance(1).retreat(ignoreLastArgs), ";");

  if (parentScope) {
    status.GetMakefile().RaiseScope(variable, value.c_str());
    return true;
  }

  // we should be nice and try to catch some simple screwups if the last or
  // next to last args are CACHE then they screwed up.  If they used FORCE
  // without CACHE they screwed up
  if ((args.back() == "CACHE") ||
      (args.size() > 1 && args[args.size() - 2] == "CACHE") ||
      (force && !cache)) {
    status.SetError("given invalid arguments for CACHE mode.");
    return false;
  }

  if (cache) {
    std::string::size_type cacheStart = args.size() - 3 - (force ? 1 : 0);
    if (!cmState::StringToCacheEntryType(args[cacheStart + 1], type)) {
      std::string m = "implicitly converting '" + args[cacheStart + 1] +
        "' to 'STRING' type.";
      status.GetMakefile().IssueMessage(MessageType::AUTHOR_WARNING, m);
      // Setting this may not be required, since it's
      // initialized as a string. Keeping this here to
      // ensure that the type is actually converting to a string.
      type = cmStateEnums::STRING;
    }
    docstring = cmValue{ args[cacheStart + 2] };
  }

  // see if this is already in the cache
  cmState* state = status.GetMakefile().GetState();
  cmValue existingValue = state->GetCacheEntryValue(variable);
  if (existingValue &&
      (state->GetCacheEntryType(variable) != cmStateEnums::UNINITIALIZED)) {
    // if the set is trying to CACHE the value but the value
    // is already in the cache and the type is not internal
    // then leave now without setting any definitions in the cache
    // or the makefile
    if (cache && type != cmStateEnums::INTERNAL && !force) {
      return true;
    }
  }

  // if it is meant to be in the cache then define it in the cache
  if (cache) {
    status.GetMakefile().AddCacheDefinition(variable, cmValue{ value },
                                            docstring, type, force);
  } else {
    // add the definition
    status.GetMakefile().AddDefinition(variable, value);
  }
  return true;
}
