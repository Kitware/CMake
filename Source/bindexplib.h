/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdio>
#include <set>
#include <string>

class bindexplib
{
public:
  bindexplib() { NmPath = "nm"; }
  bool AddDefinitionFile(const char* filename);
  bool AddObjectFile(const char* filename);
  void WriteFile(FILE* file);

  void SetNmPath(std::string const& nm);

private:
  std::set<std::string> Symbols;
  std::set<std::string> DataSymbols;
  std::string NmPath;
};
