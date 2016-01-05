
#ifndef cmServerCompleter_h
#define cmServerCompleter_h

#include "cmCommand.h"
#include "cmStandardIncludes.h"
#include "cmState.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_value.h"
#endif

class cmServerCompleter
{
public:
  cmServerCompleter(cmake* cm, cmState::Snapshot snp);

  Json::Value Complete(cmState::Snapshot snp, cmListFileFunction fn,
                       std::string matcher, long fileLine, long fileColumn);

private:
  cmake* CMakeInstance;
  cmState::Snapshot Snapshot;

  Json::Value CodeCompleteCommand(cmState::Snapshot snp, std::string matcher);

  Json::Value CodeCompleteParameter(cmState::Snapshot snp,
                                    cmListFileFunction* fn, long fileLine,
                                    long fileColumn);

  Json::Value CodeCompleteVariable(cmState::Snapshot snp, std::string matcher);

  Json::Value doComplete(cmCommand::ParameterContext ctx, std::string matcher,
                         cmCommand* cmd, std::vector<std::string> params,
                         cmState::Snapshot snp);

  std::vector<std::string> GetPackageNames(cmState::Snapshot snp) const;
  std::vector<std::string> GetModuleNames(cmState::Snapshot snp) const;
};

#endif
