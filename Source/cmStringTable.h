/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStringTable_h
#define cmStringTable_h

#include <string>

/** \class cmStringTable
 * \brief A lookup table for strings used to keep duplicate strings out of memory
 */
class cmStringTable
{
public:
  static size_t GetStringId(const std::string & str);
  static size_t GetStringId(const char * str);
  static const std::string & GetString(size_t id);
};

#endif
