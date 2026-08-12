/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDirectoryPropertyHelper.h"

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmMakefile* cmResolveDirectoryMakefile(cmMakefile& callerMf,
                                       std::string const& dirName)
{
  if (dirName.empty()) {
    return &callerMf;
  }
  std::string const dir = cmSystemTools::CollapseFullPath(
    dirName, callerMf.GetCurrentSourceDirectory());
  return callerMf.GetGlobalGenerator()->FindMakefile(dir);
}

cmGetDirectoryPropertyResult cmGetDirectoryProperty(
  std::string const& dirName, std::string const& propertyName,
  cmMakefile& callerMf, cmValue& propertyValue)
{
  cmMakefile* mf = cmResolveDirectoryMakefile(callerMf, dirName);
  if (!mf) {
    return cmGetDirectoryPropertyResult::DirectoryNotFound;
  }
  return cmGetDirectoryProperty(*mf, propertyName, propertyValue);
}

cmGetDirectoryPropertyResult cmGetDirectoryProperty(
  cmMakefile& mf, std::string const& propertyName, cmValue& propertyValue)
{
  propertyValue = mf.GetProperty(propertyName);
  return cmGetDirectoryPropertyResult::Success;
}
