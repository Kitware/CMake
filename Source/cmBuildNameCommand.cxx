/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmBuildNameCommand.h"

#include <algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

bool cmBuildNameCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  cmMakefile& mf = status.GetMakefile();
  cmProp cacheValue = mf.GetDefinition(args[0]);
  if (cacheValue) {
    // do we need to correct the value?
    cmsys::RegularExpression reg("[()/]");
    std::string cv = *cacheValue;
    if (reg.find(cv)) {
      std::replace(cv.begin(), cv.end(), '/', '_');
      std::replace(cv.begin(), cv.end(), '(', '_');
      std::replace(cv.begin(), cv.end(), ')', '_');
      mf.AddCacheDefinition(args[0], cv, "Name of build.",
                            cmStateEnums::STRING);
    }
    return true;
  }

  std::string buildname = "WinNT";
  if (mf.GetDefinition("UNIX")) {
    buildname.clear();
    cmSystemTools::RunSingleCommand("uname -a", &buildname, &buildname);
    if (!buildname.empty()) {
      std::string RegExp = "([^ ]*) [^ ]* ([^ ]*) ";
      cmsys::RegularExpression reg(RegExp);
      if (reg.find(buildname)) {
        buildname = reg.match(1) + "-" + reg.match(2);
      }
    }
  }
  std::string compiler = "${CMAKE_CXX_COMPILER}";
  mf.ExpandVariablesInString(compiler);
  buildname += "-";
  buildname += cmSystemTools::GetFilenameName(compiler);
  std::replace(buildname.begin(), buildname.end(), '/', '_');
  std::replace(buildname.begin(), buildname.end(), '(', '_');
  std::replace(buildname.begin(), buildname.end(), ')', '_');

  mf.AddCacheDefinition(args[0], buildname, "Name of build.",
                        cmStateEnums::STRING);
  return true;
}
