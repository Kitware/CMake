#include <aspell.h>
#include <assert.h>
#include <string.h>

int main(void)
{
  char const* aspell_version = aspell_version_string();
  assert(strcmp(aspell_version, CMAKE_EXPECTED_ASPELL_VERSION) == 0);
  return 0;
}
