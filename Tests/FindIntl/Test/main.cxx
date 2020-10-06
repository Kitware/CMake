extern "C" {
#include <libintl.h>
}

int main()
{
  // Check if we include the directory correctly and have no link errors
  bindtextdomain("", "");
  gettext("");
  return 0;
}
