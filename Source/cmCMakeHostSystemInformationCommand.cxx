/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeHostSystemInformationCommand.h"

#include <cstddef>

#include "cmsys/SystemInformation.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

#if defined(_WIN32)
#  include "cmAlgorithms.h"
#  include "cmGlobalGenerator.h"
#  include "cmGlobalVisualStudioVersionedGenerator.h"
#  include "cmSystemTools.h"
#  include "cmVSSetupHelper.h"
#  define HAVE_VS_SETUP_HELPER
#endif

namespace {
bool GetValue(cmExecutionStatus& status, cmsys::SystemInformation& info,
              std::string const& key, std::string& value);
std::string ValueToString(size_t value);
std::string ValueToString(const char* value);
std::string ValueToString(std::string const& value);
}

// cmCMakeHostSystemInformation
bool cmCMakeHostSystemInformationCommand(std::vector<std::string> const& args,
                                         cmExecutionStatus& status)
{
  size_t current_index = 0;

  if (args.size() < (current_index + 2) || args[current_index] != "RESULT") {
    status.SetError("missing RESULT specification.");
    return false;
  }

  std::string const& variable = args[current_index + 1];
  current_index += 2;

  if (args.size() < (current_index + 2) || args[current_index] != "QUERY") {
    status.SetError("missing QUERY specification");
    return false;
  }

  cmsys::SystemInformation info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();

  std::string result_list;
  for (size_t i = current_index + 1; i < args.size(); ++i) {
    std::string const& key = args[i];
    if (i != current_index + 1) {
      result_list += ";";
    }
    std::string value;
    if (!GetValue(status, info, key, value)) {
      return false;
    }
    result_list += value;
  }

  status.GetMakefile().AddDefinition(variable, result_list);

  return true;
}

namespace {

bool GetValue(cmExecutionStatus& status, cmsys::SystemInformation& info,
              std::string const& key, std::string& value)
{
  if (key == "NUMBER_OF_LOGICAL_CORES") {
    value = ValueToString(info.GetNumberOfLogicalCPU());
  } else if (key == "NUMBER_OF_PHYSICAL_CORES") {
    value = ValueToString(info.GetNumberOfPhysicalCPU());
  } else if (key == "HOSTNAME") {
    value = ValueToString(info.GetHostname());
  } else if (key == "FQDN") {
    value = ValueToString(info.GetFullyQualifiedDomainName());
  } else if (key == "TOTAL_VIRTUAL_MEMORY") {
    value = ValueToString(info.GetTotalVirtualMemory());
  } else if (key == "AVAILABLE_VIRTUAL_MEMORY") {
    value = ValueToString(info.GetAvailableVirtualMemory());
  } else if (key == "TOTAL_PHYSICAL_MEMORY") {
    value = ValueToString(info.GetTotalPhysicalMemory());
  } else if (key == "AVAILABLE_PHYSICAL_MEMORY") {
    value = ValueToString(info.GetAvailablePhysicalMemory());
  } else if (key == "IS_64BIT") {
    value = ValueToString(info.Is64Bits());
  } else if (key == "HAS_FPU") {
    value = ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_FPU));
  } else if (key == "HAS_MMX") {
    value = ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_MMX));
  } else if (key == "HAS_MMX_PLUS") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_MMX_PLUS));
  } else if (key == "HAS_SSE") {
    value = ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_SSE));
  } else if (key == "HAS_SSE2") {
    value = ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_SSE2));
  } else if (key == "HAS_SSE_FP") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SSE_FP));
  } else if (key == "HAS_SSE_MMX") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SSE_MMX));
  } else if (key == "HAS_AMD_3DNOW") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_AMD_3DNOW));
  } else if (key == "HAS_AMD_3DNOW_PLUS") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_AMD_3DNOW_PLUS));
  } else if (key == "HAS_IA64") {
    value = ValueToString(
      info.DoesCPUSupportFeature(cmsys::SystemInformation::CPU_FEATURE_IA64));
  } else if (key == "HAS_SERIAL_NUMBER") {
    value = ValueToString(info.DoesCPUSupportFeature(
      cmsys::SystemInformation::CPU_FEATURE_SERIALNUMBER));
  } else if (key == "PROCESSOR_NAME") {
    value = ValueToString(info.GetExtendedProcessorName());
  } else if (key == "PROCESSOR_DESCRIPTION") {
    value = info.GetCPUDescription();
  } else if (key == "PROCESSOR_SERIAL_NUMBER") {
    value = ValueToString(info.GetProcessorSerialNumber());
  } else if (key == "OS_NAME") {
    value = ValueToString(info.GetOSName());
  } else if (key == "OS_RELEASE") {
    value = ValueToString(info.GetOSRelease());
  } else if (key == "OS_VERSION") {
    value = ValueToString(info.GetOSVersion());
  } else if (key == "OS_PLATFORM") {
    value = ValueToString(info.GetOSPlatform());
#ifdef HAVE_VS_SETUP_HELPER
  } else if (key == "VS_15_DIR") {
    // If generating for the VS 15 IDE, use the same instance.
    cmGlobalGenerator* gg = status.GetMakefile().GetGlobalGenerator();
    if (cmHasLiteralPrefix(gg->GetName(), "Visual Studio 15 ")) {
      cmGlobalVisualStudioVersionedGenerator* vs15gen =
        static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
      if (vs15gen->GetVSInstance(value)) {
        return true;
      }
    }

    // Otherwise, find a VS 15 instance ourselves.
    cmVSSetupAPIHelper vsSetupAPIHelper(15);
    if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
      cmSystemTools::ConvertToUnixSlashes(value);
    }
  } else if (key == "VS_16_DIR") {
    // If generating for the VS 16 IDE, use the same instance.
    cmGlobalGenerator* gg = status.GetMakefile().GetGlobalGenerator();
    if (cmHasLiteralPrefix(gg->GetName(), "Visual Studio 16 ")) {
      cmGlobalVisualStudioVersionedGenerator* vs16gen =
        static_cast<cmGlobalVisualStudioVersionedGenerator*>(gg);
      if (vs16gen->GetVSInstance(value)) {
        return true;
      }
    }

    // Otherwise, find a VS 16 instance ourselves.
    cmVSSetupAPIHelper vsSetupAPIHelper(16);
    if (vsSetupAPIHelper.GetVSInstanceInfo(value)) {
      cmSystemTools::ConvertToUnixSlashes(value);
    }
#endif
  } else {
    std::string e = "does not recognize <key> " + key;
    status.SetError(e);
    return false;
  }

  return true;
}

std::string ValueToString(size_t value)
{
  return std::to_string(value);
}

std::string ValueToString(const char* value)
{
  std::string safe_string = value ? value : "";
  return safe_string;
}

std::string ValueToString(std::string const& value)
{
  return value;
}
}
