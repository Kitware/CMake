/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmNinjaTypes_h
#define cmNinjaTypes_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

typedef std::vector<std::string> cmNinjaDeps;
typedef std::map<std::string, std::string> cmNinjaVars;

#endif // ! cmNinjaTypes_h
