/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDependsJava.h"

#include "cmSystemTools.h"

cmDependsJava::cmDependsJava() = default;

cmDependsJava::~cmDependsJava() = default;

bool cmDependsJava::WriteDependencies(std::set<std::string> const& sources,
                                      std::string const& /*obj*/,
                                      std::ostream& /*makeDepends*/,
                                      std::ostream& /*internalDepends*/)
{
  // Make sure this is a scanning instance.
  if (sources.empty() || sources.begin()->empty()) {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
  }

  return true;
}

bool cmDependsJava::CheckDependencies(
  std::istream& /*internalDepends*/,
  std::string const& /*internalDependsFileName*/, DependencyMap& /*validDeps*/)
{
  return true;
}
