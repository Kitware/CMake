/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLocale_h
#define cmLocale_h

#include "cmConfigure.h"

#include <locale.h>
#include <string>

class cmLocaleRAII
{
public:
  cmLocaleRAII()
    : OldLocale(setlocale(LC_CTYPE, CM_NULLPTR))
  {
    setlocale(LC_CTYPE, "");
  }
  ~cmLocaleRAII() { setlocale(LC_CTYPE, this->OldLocale.c_str()); }

private:
  cmLocaleRAII(cmLocaleRAII const&);
  cmLocaleRAII& operator=(cmLocaleRAII const&);

  std::string OldLocale;
};

#endif
