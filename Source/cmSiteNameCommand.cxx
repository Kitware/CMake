/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSiteNameCommand.h"

#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmSiteNameCommand
bool cmSiteNameCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 1) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::vector<std::string> paths;
  paths.emplace_back("/usr/bsd");
  paths.emplace_back("/usr/sbin");
  paths.emplace_back("/usr/bin");
  paths.emplace_back("/bin");
  paths.emplace_back("/sbin");
  paths.emplace_back("/usr/local/bin");

  const char* cacheValue = status.GetMakefile().GetDefinition(args[0]);
  if (cacheValue) {
    return true;
  }

  const char* temp = status.GetMakefile().GetDefinition("HOSTNAME");
  std::string hostname_cmd;
  if (temp) {
    hostname_cmd = temp;
  } else {
    hostname_cmd = cmSystemTools::FindProgram("hostname", paths);
  }

  std::string siteName = "unknown";
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string host;
  if (cmSystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\"
        "Control\\ComputerName\\ComputerName;ComputerName",
        host)) {
    siteName = host;
  }
#else
  // try to find the hostname for this computer
  if (!cmIsOff(hostname_cmd)) {
    std::string host;
    cmSystemTools::RunSingleCommand(hostname_cmd, &host, nullptr, nullptr,
                                    nullptr, cmSystemTools::OUTPUT_NONE);

    // got the hostname
    if (!host.empty()) {
      // remove any white space from the host name
      std::string hostRegExp = "[ \t\n\r]*([^\t\n\r ]*)[ \t\n\r]*";
      cmsys::RegularExpression hostReg(hostRegExp.c_str());
      if (hostReg.find(host.c_str())) {
        // strip whitespace
        host = hostReg.match(1);
      }

      if (!host.empty()) {
        siteName = host;
      }
    }
  }
#endif
  status.GetMakefile().AddCacheDefinition(
    args[0], siteName.c_str(),
    "Name of the computer/site where compile is being run",
    cmStateEnums::STRING);

  return true;
}
