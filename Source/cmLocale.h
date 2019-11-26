/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLocale_h
#define cmLocale_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <clocale>
#include <string>

class cmLocaleRAII
{
public:
  cmLocaleRAII()
    : OldLocale(setlocale(LC_CTYPE, nullptr))
  {
    setlocale(LC_CTYPE, "");
  }

  ~cmLocaleRAII() { setlocale(LC_CTYPE, this->OldLocale.c_str()); }

  cmLocaleRAII(cmLocaleRAII const&) = delete;
  cmLocaleRAII& operator=(cmLocaleRAII const&) = delete;

private:
  std::string OldLocale;
};

#endif
