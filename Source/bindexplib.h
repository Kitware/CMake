/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

class bindexplib
{
public:
  bool AddDefinitionFile(std::string const& definitionFile);
  bool AddDefinitionFile(std::vector<std::string> const& definitionFiles);
  bool AddObjectFile(std::string const& objectFile);
  bool AddObjectFile(std::vector<std::string> const& objectFiles,
                     std::string const& responseFile = "exports.def.objs.rsp");
  bool WriteFile(std::ostream& output);

  void SetNmPath(std::string const& nm);

private:
  std::set<std::string> Symbols;
  std::set<std::string> DataSymbols;
  std::string NmPath;
};
