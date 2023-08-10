/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeHostSystemInformationCommand.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/SystemInformation.hxx"

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmWindowsRegistry.h"

#ifdef _WIN32
#  include "cmAlgorithms.h"
#  include "cmGlobalGenerator.h"
#  include "cmGlobalVisualStudio10Generator.h"
#  include "cmGlobalVisualStudioVersionedGenerator.h"
#  include "cmVSSetupHelper.h"
#  define HAVE_VS_SETUP_HELPER
#endif

namespace {
std::string const DELIM[2] = { {}, ";" };

// BEGIN Private functions
std::string ValueToString(std::size_t const value)
{
  return std::to_string(value);
}

std::string ValueToString(const char* const value)
{
  return value ? value : std::string{};
}

std::string ValueToString(std::string const& value)
{
  return value;
}

cm::optional<std::string> GetValue(cmsys::SystemInformation& info,
                                   std::string const& key)
{
  if (key == "NUMBER_OF_LOGICAL_CORES"_s) {
    return ValueToString(info.GetNumberOfLogicalCPU());
  }
  if (key == "NUMBER_OF_PHYSICAL_CORES"_s) {
    return ValueToString(info.GetNumberOfPhysicalCPU());
  }
  if (key == "HOSTNAME"_s) {
    return ValueToString(info.GetHostname());
  }
  if (key == "FQDN"_s) {
    return ValueToString(info.GetFullyQualifiedDomainName());
  }
  if (key == "TOTAL_VIRTUAL_MEMORY"_s) {
    return ValueToString(info.GetTotalVirtualMemory());
  }
  if (key == "AVAILABLE_VIRTUAL_MEMORY"_s) {
    return ValueToString(info.GetAvailableVirtualMemory());
  }
  if (key == "TOTAL_PHYSICAL_MEMORY"_s) {
    return ValueToString(info.GetTotalPhysicalMemory());
  }
  if (key == "AVAILABLE_PHYSICAL_MEMORY"_s) {
    return ValueToString(info.GetAvailablePhysicalMemory());
  }
  if (key == "IS_64BIT"_s) {
    return ValueToString(info.Is64Bits());
  }
  if (key == "HAS_FPU"_s) {
    return ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_FPU));
  }
  if (key == "HAS_MMX"_s) {
    return ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_MMX));
  }
  if (key == "HAS_MMX_PLUS"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_MMX_PLUS));
  }
  if (key == "HAS_SSE"_s) {
    return ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_SSE));
  }
  if (key == "HAS_SSE2"_s) {
    return ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_SSE2));
  }
  if (key == "HAS_SSE_FP"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SSE_FP));
  }
  if (key == "HAS_SSE_MMX"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SSE_MMX));
  }
  if (key == "HAS_AMD_3DNOW"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_AMD_3DNOW));
  }
  if (key == "HAS_AMD_3DNOW_PLUS"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_AMD_3DNOW_PLUS));
  }
  if (key == "HAS_IA64"_s) {
    return ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_IA64));
  }
  if (key == "HAS_SERIAL_NUMBER"_s) {
    return ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SERIALNUMBER));
  }
  if (key == "PROCESSOR_NAME"_s) {
    return ValueToString(info.GetExtendedProcessorName());
  }
  if (key == "PROCESSOR_DESCRIPTION"_s) {
    return info.GetCPUDescription();
  }
  if (key == "PROCESSOR_SERIAL_NUMBER"_s) {
    return ValueToString(info.GetProcessorSerialNumber());
  }
  if (key == "OS_NAME"_s) {
    return ValueToString(info.GetOSName());
  }
  if (key == "OS_RELEASE"_s) {
    return ValueToString(info.GetOSRelease());
  }
  if (key == "OS_VERSION"_s) {
    return ValueToString(info.GetOSVersion());
  }
  if (key == "OS_PLATFORM"_s) {
    return ValueToString(info.GetOSPlatform());
  }
  return {};
}

cm::optional<std::pair<std::string, std::string>> ParseOSReleaseLine(
  std::string const& line)
{
  std::string key;
  std::string value;

  char prev = 0;
  enum ParserState
  {
    PARSE_KEY_1ST,
    PARSE_KEY,
    FOUND_EQ,
    PARSE_SINGLE_QUOTE_VALUE,
    PARSE_DBL_QUOTE_VALUE,
    PARSE_VALUE,
    IGNORE_REST
  } state = PARSE_KEY_1ST;

  for (auto ch : line) {
    switch (state) {
      case PARSE_KEY_1ST:
        if (std::isalpha(ch) || ch == '_') {
          key += ch;
          state = PARSE_KEY;
        } else if (!std::isspace(ch)) {
          state = IGNORE_REST;
        }
        break;

      case PARSE_KEY:
        if (ch == '=') {
          state = FOUND_EQ;
        } else if (std::isalnum(ch) || ch == '_') {
          key += ch;
        } else {
          state = IGNORE_REST;
        }
        break;

      case FOUND_EQ:
        switch (ch) {
          case '\'':
            state = PARSE_SINGLE_QUOTE_VALUE;
            break;
          case '"':
            state = PARSE_DBL_QUOTE_VALUE;
            break;
          case '#':
          case '\\':
            state = IGNORE_REST;
            break;
          default:
            value += ch;
            state = PARSE_VALUE;
        }
        break;

      case PARSE_SINGLE_QUOTE_VALUE:
        if (ch == '\'') {
          if (prev != '\\') {
            state = IGNORE_REST;
          } else {
            assert(!value.empty());
            value[value.size() - 1] = ch;
          }
        } else {
          value += ch;
        }
        break;

      case PARSE_DBL_QUOTE_VALUE:
        if (ch == '"') {
          if (prev != '\\') {
            state = IGNORE_REST;
          } else {
            assert(!value.empty());
            value[value.size() - 1] = ch;
          }
        } else {
          value += ch;
        }
        break;

      case PARSE_VALUE:
        if (ch == '#' || std::isspace(ch)) {
          state = IGNORE_REST;
        } else {
          value += ch;
        }
        break;

      default:
        // Unexpected os-release parser state!
        state = IGNORE_REST;
        break;
    }

    if (state == IGNORE_REST) {
      break;
    }
    prev = ch;
  }
  if (!(key.empty() || value.empty())) {
    return std::make_pair(key, value);
  }
  return {};
}

std::map<std::string, std::string> GetOSReleaseVariables(
  cmExecutionStatus& status)
{
  auto& makefile = status.GetMakefile();
  const auto& sysroot = makefile.GetSafeDefinition("CMAKE_SYSROOT");

  std::map<std::string, std::string> data;
  // Based on
  // https://www.freedesktop.org/software/systemd/man/os-release.html
  for (auto name : { "/etc/os-release"_s, "/usr/lib/os-release"_s }) {
    const auto& filename = cmStrCat(sysroot, name);
    if (cmSystemTools::FileExists(filename)) {
      cmsys::ifstream fin(filename.c_str());
      for (std::string line; !std::getline(fin, line).fail();) {
        auto kv = ParseOSReleaseLine(line);
        if (kv.has_value()) {
          data.emplace(kv.value());
        }
      }
      break;
    }
  }
  // Got smth?
  if (!data.empty()) {
    return data;
  }

  // Ugh, it could be some pre-os-release distro.
  // Lets try some fallback getters.
  // See also:
  //  - http://linuxmafia.com/faq/Admin/release-files.html

  // 1. CMake provided
  cmsys::Glob gl;
  std::vector<std::string> scripts;
  auto const findExpr = cmStrCat(cmSystemTools::GetCMakeRoot(),
                                 "/Modules/Internal/OSRelease/*.cmake");
  if (gl.FindFiles(findExpr)) {
    scripts = gl.GetFiles();
  }

  // 2. User provided (append to the CMake prvided)
  cmList::append(
    scripts, makefile.GetDefinition("CMAKE_GET_OS_RELEASE_FALLBACK_SCRIPTS"));

  // Filter out files that are not in format `NNN-name.cmake`
  auto checkName = [](std::string const& filepath) -> bool {
    auto const& filename = cmSystemTools::GetFilenameName(filepath);
    // NOTE Minimum filename length expected:
    //   NNN-<at-least-one-char-name>.cmake  --> 11
    return (filename.size() < 11) || !std::isdigit(filename[0]) ||
      !std::isdigit(filename[1]) || !std::isdigit(filename[2]) ||
      filename[3] != '-';
  };
  scripts.erase(std::remove_if(scripts.begin(), scripts.end(), checkName),
                scripts.end());

  // Make sure scripts are running in desired order
  std::sort(scripts.begin(), scripts.end(),
            [](std::string const& lhs, std::string const& rhs) -> bool {
              long lhs_order;
              cmStrToLong(cmSystemTools::GetFilenameName(lhs).substr(0u, 3u),
                          &lhs_order);
              long rhs_order;
              cmStrToLong(cmSystemTools::GetFilenameName(rhs).substr(0u, 3u),
                          &rhs_order);
              return lhs_order < rhs_order;
            });

  // Name of the variable to put the results
  std::string const result_variable{ "CMAKE_GET_OS_RELEASE_FALLBACK_RESULT" };

  for (auto const& script : scripts) {
    // Unset the result variable
    makefile.RemoveDefinition(result_variable);

    // include FATAL_ERROR and ERROR in the return status
    if (!makefile.ReadListFile(script) ||
        cmSystemTools::GetErrorOccurredFlag()) {
      // Ok, no worries... go try the next script.
      continue;
    }

    cmList variables{ makefile.GetDefinition(result_variable) };
    if (variables.empty()) {
      // Heh, this script didn't found anything... go try the next one.
      continue;
    }

    for (auto const& variable : variables) {
      auto value = makefile.GetSafeDefinition(variable);
      makefile.RemoveDefinition(variable);

      if (!cmHasPrefix(variable, cmStrCat(result_variable, '_'))) {
        // Ignore unknown variable set by the script
        continue;
      }

      auto key = variable.substr(result_variable.size() + 1,
                                 variable.size() - result_variable.size() - 1);
      data.emplace(std::move(key), std::move(value));
    }

    // Try 'till some script can get anything
    if (!data.empty()) {
      data.emplace("USED_FALLBACK_SCRIPT", script);
      break;
    }
  }

  makefile.RemoveDefinition(result_variable);

  return data;
}

cm::optional<std::string> GetValue(cmExecutionStatus& status,
                                   std::string const& key,
                                   std::string const& variable)
{
  const auto prefix = "DISTRIB_"_s;
  if (!cmHasPrefix(key, prefix)) {
    return {};
  }

  static const std::map<std::string, std::string> s_os_release =
    GetOSReleaseVariables(status);

  auto& makefile = status.GetMakefile();

  const std::string subkey =
    key.substr(prefix.size(), key.size() - prefix.size());
  if (subkey == "INFO"_s) {
    std::string vars;
    for (const auto& kv : s_os_release) {
      auto cmake_var_name = cmStrCat(variable, '_', kv.first);
      vars += DELIM[!vars.empty()] + cmake_var_name;
      makefile.AddDefinition(cmake_var_name, kv.second);
    }
    return cm::optional<std::string>(std::move(vars));
  }

  // Query individual variable
  const auto it = s_os_release.find(subkey);
  if (it != s_os_release.cend()) {
    return it->second;
  }

  // NOTE Empty string means requested variable not set
  return std::string{};
}

#ifdef HAVE_VS_SETUP_HELPER
cm::optional<std::string> GetValue(cmExecutionStatus& status,
                                   std::string const& key)
{
  auto* const gg = status.GetMakefile().GetGlobalGenerator();
  for (auto vs : { 15, 16, 17 }) {
    if (key == cmStrCat("VS_"_s, vs, "_DIR"_s)) {
      std::string value;
      // If generating for the VS nn IDE, use the same instance.

      if (cmHasPrefix(gg->GetName(), cmStrCat("Visual Studio "_s, vs, ' '))) {
        cmGlobalVisualStudioVersionedGenerator* vsNNgen =
          static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
        if (vsNNgen->GetVSInstance(value)) {
          return value;
        }
      }

      // Otherwise, find a VS nn instance ourselves.
      cmVSSetupAPIHelper vsSetupAPIHelper(vs);
      if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
        cmSystemTools::ConvertToUnixSlashes(value);
      }
      return value;
    }
  }

  if (key == "VS_MSBUILD_COMMAND"_s && gg->IsVisualStudioAtLeast10()) {
    cmGlobalVisualStudio10Generator* vs10gen =
      static_cast<cmGlobalVisualStudio10Generator*>(gg);
    return vs10gen->FindMSBuildCommandEarly(&status.GetMakefile());
  }

  return {};
}
#endif

cm::optional<std::string> GetValueChained()
{
  return {};
}

template <typename GetterFn, typename... Next>
cm::optional<std::string> GetValueChained(GetterFn current, Next... chain)
{
  auto value = current();
  if (value.has_value()) {
    return value;
  }
  return GetValueChained(chain...);
}

template <typename Range>
bool QueryWindowsRegistry(Range args, cmExecutionStatus& status,
                          std::string const& variable)
{
  using View = cmWindowsRegistry::View;
  if (args.empty()) {
    status.SetError("missing <key> specification.");
    return false;
  }
  std::string const& key = *args.begin();

  struct Arguments : public ArgumentParser::ParseResult
  {
    std::string ValueName;
    bool ValueNames = false;
    bool SubKeys = false;
    std::string View;
    std::string Separator;
    std::string ErrorVariable;
  };
  cmArgumentParser<Arguments> parser;
  parser.Bind("VALUE"_s, &Arguments::ValueName)
    .Bind("VALUE_NAMES"_s, &Arguments::ValueNames)
    .Bind("SUBKEYS"_s, &Arguments::SubKeys)
    .Bind("VIEW"_s, &Arguments::View)
    .Bind("SEPARATOR"_s, &Arguments::Separator)
    .Bind("ERROR_VARIABLE"_s, &Arguments::ErrorVariable);
  std::vector<std::string> invalidArgs;

  Arguments const arguments = parser.Parse(args.advance(1), &invalidArgs);
  if (!invalidArgs.empty()) {
    status.SetError(cmStrCat("given invalid argument(s) \"",
                             cmJoin(invalidArgs, ", "_s), "\"."));
    return false;
  }
  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if ((!arguments.ValueName.empty() &&
       (arguments.ValueNames || arguments.SubKeys)) ||
      (arguments.ValueName.empty() && arguments.ValueNames &&
       arguments.SubKeys)) {
    status.SetError("given mutually exclusive sub-options \"VALUE\", "
                    "\"VALUE_NAMES\" or \"SUBKEYS\".");
    return false;
  }

  if (!arguments.View.empty() && !cmWindowsRegistry::ToView(arguments.View)) {
    status.SetError(
      cmStrCat("given invalid value for \"VIEW\": ", arguments.View, '.'));
    return false;
  }

  auto& makefile = status.GetMakefile();

  makefile.AddDefinition(variable, ""_s);

  auto view = arguments.View.empty()
    ? View::Both
    : *cmWindowsRegistry::ToView(arguments.View);
  cmWindowsRegistry registry(makefile);
  if (arguments.ValueNames) {
    auto result = registry.GetValueNames(key, view);
    if (result) {
      makefile.AddDefinition(variable, cmJoin(*result, ";"_s));
    }
  } else if (arguments.SubKeys) {
    auto result = registry.GetSubKeys(key, view);
    if (result) {
      makefile.AddDefinition(variable, cmJoin(*result, ";"_s));
    }
  } else {
    auto result =
      registry.ReadValue(key, arguments.ValueName, view, arguments.Separator);
    if (result) {
      makefile.AddDefinition(variable, *result);
    }
  }

  // return error message if requested
  if (!arguments.ErrorVariable.empty()) {
    makefile.AddDefinition(arguments.ErrorVariable, registry.GetLastError());
  }

  return true;
}

// END Private functions
} // anonymous namespace

// cmCMakeHostSystemInformation
bool cmCMakeHostSystemInformationCommand(std::vector<std::string> const& args,
                                         cmExecutionStatus& status)
{
  std::size_t current_index = 0;

  if (args.size() < (current_index + 2) || args[current_index] != "RESULT"_s) {
    status.SetError("missing RESULT specification.");
    return false;
  }

  auto const& variable = args[current_index + 1];
  current_index += 2;

  if (args.size() < (current_index + 2) || args[current_index] != "QUERY"_s) {
    status.SetError("missing QUERY specification");
    return false;
  }

  if (args[current_index + 1] == "WINDOWS_REGISTRY"_s) {
    return QueryWindowsRegistry(cmMakeRange(args).advance(current_index + 2),
                                status, variable);
  }

  static cmsys::SystemInformation info;
  static auto initialized = false;
  if (!initialized) {
    info.RunCPUCheck();
    info.RunOSCheck();
    info.RunMemoryCheck();
    initialized = true;
  }

  std::string result_list;
  for (auto i = current_index + 1; i < args.size(); ++i) {
    result_list += DELIM[!result_list.empty()];

    auto const& key = args[i];
    // clang-format off
    auto value =
      GetValueChained(
          [&]() { return GetValue(info, key); }
        , [&]() { return GetValue(status, key, variable); }
#ifdef HAVE_VS_SETUP_HELPER
        , [&]() { return GetValue(status, key); }
#endif
        );
    // clang-format on
    if (!value) {
      status.SetError("does not recognize <key> " + key);
      return false;
    }
    result_list += value.value();
  }

  status.GetMakefile().AddDefinition(variable, result_list);

  return true;
}
