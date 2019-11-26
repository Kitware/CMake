/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTimestamp_h
#define cmTimestamp_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <ctime>
#include <string>

/** \class cmTimestamp
 * \brief Utility class to generate string representation of a timestamp
 *
 */
class cmTimestamp
{
public:
  std::string CurrentTime(const std::string& formatString, bool utcFlag);

  std::string FileModificationTime(const char* path,
                                   const std::string& formatString,
                                   bool utcFlag);

  std::string CreateTimestampFromTimeT(time_t timeT, std::string formatString,
                                       bool utcFlag) const;

private:
  time_t CreateUtcTimeTFromTm(struct tm& timeStruct) const;

  std::string AddTimestampComponent(char flag, struct tm& timeStruct,
                                    time_t timeT) const;
};

#endif
