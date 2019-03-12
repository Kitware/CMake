/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

struct cmSourceReqInfo
{
  std::string LogicalName;
  std::string CompiledModulePath;
};

struct cmSourceInfo
{
  std::string PrimaryOutput;

  // Set of provided and required modules.
  std::vector<cmSourceReqInfo> Provides;
  std::vector<cmSourceReqInfo> Requires;

  // Set of files included in the translation unit.
  std::vector<std::string> Includes;
};

bool cmScanDepFormat_P1689_Parse(std::string const& arg_pp,
                                 cmSourceInfo* info);
bool cmScanDepFormat_P1689_Write(std::string const& path,
                                 std::string const& input,
                                 cmSourceInfo const& info);
