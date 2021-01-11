/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPICMakeFiles.h"

#include <memory>
#include <string>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmake.h"

namespace {

class CMakeFiles
{
  cmFileAPI& FileAPI;
  unsigned long Version;
  std::string CMakeModules;
  std::string const& TopSource;
  std::string const& TopBuild;
  bool OutOfSource;

  Json::Value DumpPaths();
  Json::Value DumpInputs();
  Json::Value DumpInput(std::string const& file);

public:
  CMakeFiles(cmFileAPI& fileAPI, unsigned long version);
  Json::Value Dump();
};

CMakeFiles::CMakeFiles(cmFileAPI& fileAPI, unsigned long version)
  : FileAPI(fileAPI)
  , Version(version)
  , CMakeModules(cmSystemTools::GetCMakeRoot() + "/Modules")
  , TopSource(this->FileAPI.GetCMakeInstance()->GetHomeDirectory())
  , TopBuild(this->FileAPI.GetCMakeInstance()->GetHomeOutputDirectory())
  , OutOfSource(this->TopBuild != this->TopSource)
{
  static_cast<void>(this->Version);
}

Json::Value CMakeFiles::Dump()
{
  Json::Value cmakeFiles = Json::objectValue;
  cmakeFiles["paths"] = this->DumpPaths();
  cmakeFiles["inputs"] = this->DumpInputs();
  return cmakeFiles;
}

Json::Value CMakeFiles::DumpPaths()
{
  Json::Value paths = Json::objectValue;
  paths["source"] = this->TopSource;
  paths["build"] = this->TopBuild;
  return paths;
}

Json::Value CMakeFiles::DumpInputs()
{
  Json::Value inputs = Json::arrayValue;

  cmGlobalGenerator* gg =
    this->FileAPI.GetCMakeInstance()->GetGlobalGenerator();
  for (const auto& lg : gg->GetLocalGenerators()) {
    cmMakefile const* mf = lg->GetMakefile();
    for (std::string const& file : mf->GetListFiles()) {
      inputs.append(this->DumpInput(file));
    }
  }

  return inputs;
}

Json::Value CMakeFiles::DumpInput(std::string const& file)
{
  Json::Value input = Json::objectValue;

  bool const isCMake = cmSystemTools::IsSubDirectory(file, this->CMakeModules);
  if (isCMake) {
    input["isCMake"] = true;
  }

  if (!cmSystemTools::IsSubDirectory(file, this->TopSource) &&
      !cmSystemTools::IsSubDirectory(file, this->TopBuild)) {
    input["isExternal"] = true;
  }

  if (this->OutOfSource &&
      cmSystemTools::IsSubDirectory(file, this->TopBuild)) {
    input["isGenerated"] = true;
  }

  std::string path = file;
  if (!isCMake && cmSystemTools::IsSubDirectory(path, this->TopSource)) {
    // Use a relative path within the source directory.
    path = cmSystemTools::RelativePath(this->TopSource, path);
  }
  input["path"] = path;

  return input;
}
}

Json::Value cmFileAPICMakeFilesDump(cmFileAPI& fileAPI, unsigned long version)
{
  CMakeFiles cmakeFiles(fileAPI, version);
  return cmakeFiles.Dump();
}
