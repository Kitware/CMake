/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmProperty_h
#define cmProperty_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmListFileCache.h"

#include <string>

class cmProperty
{
public:
  enum ScopeType
  {
    TARGET,
    SOURCE_FILE,
    DIRECTORY,
    GLOBAL,
    CACHE,
    TEST,
    VARIABLE,
    CACHED_VARIABLE,
    INSTALL
  };

  // set this property
  void Set(const char* value, const cmListFileBacktrace & backtrace);

  // append to this property
  void Append(const char* value, const cmListFileBacktrace & backtrace, bool asString = false);

  // get the value
  const char* GetValue() const;

  // get the backtrace for last set or append
  const cmListFileBacktrace & GetBacktrace() const { return Backtrace; }

  // construct with the value not set
  cmProperty() { this->ValueHasBeenSet = false; }

protected:
  std::string Value;
  bool ValueHasBeenSet;
  cmListFileBacktrace Backtrace;
};

#endif
