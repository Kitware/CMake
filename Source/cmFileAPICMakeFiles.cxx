/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPICMakeFiles.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmGlobCacheEntry.h"
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
  Json::Value DumpGlobsDependent();
  Json::Value DumpGlobDependent(cmGlobCacheEntry const& entry);

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
  Json::Value globsDependent = this->DumpGlobsDependent();
  if (!globsDependent.empty()) {
    cmakeFiles["globsDependent"] = std::move(globsDependent);
  }
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

Json::Value CMakeFiles::DumpGlobsDependent()
{
  Json::Value globsDependent = Json::arrayValue;
  for (cmGlobCacheEntry const& entry :
       this->FileAPI.GetCMakeInstance()->GetGlobCacheEntries()) {
    globsDependent.append(this->DumpGlobDependent(entry));
  }
  return globsDependent;
}

Json::Value CMakeFiles::DumpGlobDependent(cmGlobCacheEntry const& entry)
{
  Json::Value globDependent = Json::objectValue;
  globDependent["expression"] = entry.Expression;
  if (entry.Recurse) {
    globDependent["recurse"] = true;
  }
  if (entry.ListDirectories) {
    globDependent["listDirectories"] = true;
  }
  if (entry.FollowSymlinks) {
    globDependent["followSymlinks"] = true;
  }
  if (!entry.Relative.empty()) {
    globDependent["relative"] = entry.Relative;
  }
  Json::Value paths = Json::arrayValue;
  for (std::string const& file : entry.Files) {
    paths.append(file);
  }
  globDependent["paths"] = std::move(paths);
  return globDependent;
}
}

Json::Value cmFileAPICMakeFilesDump(cmFileAPI& fileAPI, unsigned long version)
{
  CMakeFiles cmakeFiles(fileAPI, version);
  return cmakeFiles.Dump();
}
