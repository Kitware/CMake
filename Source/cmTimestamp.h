/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdint>
#include <ctime>
#include <string>

#include <cm/string_view>

/** \class cmTimestamp
 * \brief Utility class to generate string representation of a timestamp
 *
 */
class cmTimestamp
{
public:
  std::string CurrentTime(cm::string_view formatString, bool utcFlag) const;

  std::string FileModificationTime(char const* path,
                                   cm::string_view formatString,
                                   bool utcFlag) const;

  std::string CreateTimestampFromTimeT(time_t timeT, std::string formatString,
                                       bool utcFlag) const;

  std::string CreateTimestampFromTimeT(time_t timeT, uint32_t microseconds,
                                       std::string formatString,
                                       bool utcFlag) const;

private:
  time_t CreateUtcTimeTFromTm(struct tm& timeStruct) const;

  std::string AddTimestampComponent(char flag, struct tm& timeStruct,
                                    time_t timeT, bool utcFlag,
                                    uint32_t microseconds) const;
};
