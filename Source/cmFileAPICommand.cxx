/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPICommand.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmFileAPI.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmake.h"

namespace {

bool isCharDigit(char ch)
{
  return std::isdigit(static_cast<unsigned char>(ch));
}

std::string processObjectKindVersions(cmFileAPI& fileApi,
                                      cmFileAPI::ObjectKind objectKind,
                                      cm::string_view keyword,
                                      const std::vector<std::string>& versions)
{
  // The "versions" vector is empty only when the keyword was not present.
  // It is an error to provide the keyword with no versions after it, and that
  // is enforced by the argument parser before we get here.
  if (versions.empty()) {
    return {};
  }

  // The first supported version listed is what we use
  for (const std::string& ver : versions) {
    const char* vStart = ver.c_str();
    int majorVersion = std::atoi(vStart);
    int minorVersion = 0;
    std::string::size_type pos = ver.find('.');
    if (pos != std::string::npos) {
      vStart += pos + 1;
      minorVersion = std::atoi(vStart);
    }
    if (majorVersion < 1 || minorVersion < 0) {
      return cmStrCat("Given a malformed version \"", ver, "\" for ", keyword,
                      ".");
    }
    if (fileApi.AddProjectQuery(objectKind,
                                static_cast<unsigned>(majorVersion),
                                static_cast<unsigned>(minorVersion))) {
      return {};
    }
  }
  return cmStrCat("None of the specified ", keyword,
                  " versions is supported by this version of CMake.");
}

bool handleQueryCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("QUERY subcommand called without required arguments.");
    return false;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    ArgumentParser::NonEmpty<std::string> ApiVersion;
    ArgumentParser::NonEmpty<std::vector<std::string>> CodeModelVersions;
    ArgumentParser::NonEmpty<std::vector<std::string>> CacheVersions;
    ArgumentParser::NonEmpty<std::vector<std::string>> CMakeFilesVersions;
    ArgumentParser::NonEmpty<std::vector<std::string>> ToolchainsVersions;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("API_VERSION"_s, &Arguments::ApiVersion)
      .Bind("CODEMODEL"_s, &Arguments::CodeModelVersions)
      .Bind("CACHE"_s, &Arguments::CacheVersions)
      .Bind("CMAKEFILES"_s, &Arguments::CMakeFilesVersions)
      .Bind("TOOLCHAINS"_s, &Arguments::ToolchainsVersions);

  std::vector<std::string> unparsedArguments;
  Arguments const arguments =
    parser.Parse(cmMakeRange(args).advance(1), &unparsedArguments);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (!unparsedArguments.empty()) {
    status.SetError("QUERY subcommand given unknown argument \"" +
                    unparsedArguments.front() + "\".");
    return false;
  }

  if (!std::all_of(arguments.ApiVersion.begin(), arguments.ApiVersion.end(),
                   isCharDigit)) {
    status.SetError("QUERY subcommand given a non-integer API_VERSION.");
    return false;
  }
  const int apiVersion = std::atoi(arguments.ApiVersion.c_str());
  if (apiVersion != 1) {
    status.SetError(
      cmStrCat("QUERY subcommand given an unsupported API_VERSION \"",
               arguments.ApiVersion,
               "\" (the only currently supported version is 1)."));
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  cmake* cmi = mf.GetCMakeInstance();
  cmFileAPI* fileApi = cmi->GetFileAPI();

  // We want to check all keywords and report all errors, not just the first.
  // Record each result rather than short-circuiting on the first error.

  // NOTE: Double braces are needed here for compilers that don't implement the
  // CWG 1270 revision to C++11.
  std::array<std::string, 4> errors{
    { processObjectKindVersions(*fileApi, cmFileAPI::ObjectKind::CodeModel,
                                "CODEMODEL"_s, arguments.CodeModelVersions),
      processObjectKindVersions(*fileApi, cmFileAPI::ObjectKind::Cache,
                                "CACHE"_s, arguments.CacheVersions),
      processObjectKindVersions(*fileApi, cmFileAPI::ObjectKind::CMakeFiles,
                                "CMAKEFILES"_s, arguments.CMakeFilesVersions),
      processObjectKindVersions(*fileApi, cmFileAPI::ObjectKind::Toolchains,
                                "TOOLCHAINS"_s, arguments.ToolchainsVersions) }
  };

  if (!std::all_of(errors.begin(), errors.end(),
                   [](const std::string& s) -> bool { return s.empty(); })) {
    std::string message("QUERY subcommand was given invalid arguments:");
    for (const std::string& s : errors) {
      if (!s.empty()) {
        message = cmStrCat(message, "\n  ", s);
      }
    }
    status.SetError(message);
    return false;
  }

  return true;
}

}

bool cmFileAPICommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be called with arguments.");
    return false;
  }

  // clang-format off
  static cmSubcommandTable const subcommand{
    { "QUERY"_s, handleQueryCommand }
  };
  // clang-format on

  return subcommand(args[0], args, status);
}
