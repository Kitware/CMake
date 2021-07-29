/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeHostSystemInformationCommand.h"

#include <cstddef>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/SystemInformation.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

#ifdef _WIN32
#  include "cmAlgorithms.h"
#  include "cmGlobalGenerator.h"
#  include "cmGlobalVisualStudioVersionedGenerator.h"
#  include "cmSystemTools.h"
#  include "cmVSSetupHelper.h"
#  define HAVE_VS_SETUP_HELPER
#endif

namespace {
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

#ifdef HAVE_VS_SETUP_HELPER
cm::optional<std::string> GetValue(cmExecutionStatus& status,
                                   std::string const& key)
{
  std::string value;
  if (key == "VS_15_DIR") {
    // If generating for the VS 15 IDE, use the same instance.
    cmGlobalGenerator* gg = status.GetMakefile().GetGlobalGenerator();
    if (cmHasLiteralPrefix(gg->GetName(), "Visual Studio 15 ")) {
      cmGlobalVisualStudioVersionedGenerator* vs15gen =
        static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
      if (vs15gen->GetVSInstance(value)) {
        return value;
      }
    }

    // Otherwise, find a VS 15 instance ourselves.
    cmVSSetupAPIHelper vsSetupAPIHelper(15);
    if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
      cmSystemTools::ConvertToUnixSlashes(value);
    }
    return value;
  } else if (key == "VS_16_DIR") {
    // If generating for the VS 16 IDE, use the same instance.
    cmGlobalGenerator* gg = status.GetMakefile().GetGlobalGenerator();
    if (cmHasLiteralPrefix(gg->GetName(), "Visual Studio 16 ")) {
      cmGlobalVisualStudioVersionedGenerator* vs16gen =
        static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
      if (vs16gen->GetVSInstance(value)) {
        return value;
      }
    }

    // Otherwise, find a VS 16 instance ourselves.
    cmVSSetupAPIHelper vsSetupAPIHelper(16);
    if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
      cmSystemTools::ConvertToUnixSlashes(value);
    }
    return value;
  } else if (key == "VS_17_DIR") {
    // If generating for the VS 17 IDE, use the same instance.
    cmGlobalGenerator* gg = status.GetMakefile().GetGlobalGenerator();
    if (cmHasLiteralPrefix(gg->GetName(), "Visual Studio 17 ")) {
      cmGlobalVisualStudioVersionedGenerator* vs17gen =
        static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
      if (vs17gen->GetVSInstance(value)) {
        return value;
      }
    }

    // Otherwise, find a VS 17 instance ourselves.
    cmVSSetupAPIHelper vsSetupAPIHelper(17);
    if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
      cmSystemTools::ConvertToUnixSlashes(value);
    }
    return value;
  }

  return {};
}
#endif
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
    auto const& key = args[i];
    if (i != current_index + 1) {
      result_list += ";";
    }
    auto value = GetValue(info, key);
    if (!value) {
#ifdef HAVE_VS_SETUP_HELPER
      value = GetValue(status, key);
      if (!value) {
        status.SetError("does not recognize <key> " + key);
        return false;
      }
#else
      status.SetError("does not recognize <key> " + key);
      return false;
#endif
    }
    result_list += value.value();
  }

  status.GetMakefile().AddDefinition(variable, result_list);

  return true;
}
