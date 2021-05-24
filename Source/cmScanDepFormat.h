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

struct cmScanDepInfo
{
  std::string PrimaryOutput;

  // Set of provided and required modules.
  std::vector<cmSourceReqInfo> Provides;
  std::vector<cmSourceReqInfo> Requires;
};

bool cmScanDepFormat_P1689_Parse(std::string const& arg_pp,
                                 cmScanDepInfo* info);
bool cmScanDepFormat_P1689_Write(std::string const& path,
                                 cmScanDepInfo const& info);
