#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltconfig.h>
#include <stdio.h>
#include <string.h>

int main()
{
  xsltInit();

  xsltStylesheet* style = xsltNewStylesheet();
  xsltFreeStylesheet(style);

  if (0 != strcmp(CMAKE_EXPECTED_LibXslt_VERSION, LIBXSLT_DOTTED_VERSION)) {
    printf("CMAKE_EXPECTED_LibXslt_VERSION: '%s'\n",
           CMAKE_EXPECTED_LibXslt_VERSION);
    printf("LIBXSLT_DOTTED_VERSION: '%s'\n", LIBXSLT_DOTTED_VERSION);
    return 1;
  }

  xsltCleanupGlobals();

  return 0;
}
