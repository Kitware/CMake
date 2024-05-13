#include <libexslt/exslt.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>

int main(void)
{
  xsltInit();

  xsltStylesheet* style = xsltNewStylesheet();
  exsltRegisterAll();
  xsltFreeStylesheet(style);

  xsltCleanupGlobals();

  return 0;
}
