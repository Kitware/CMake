#include <jasper/jasper.h>
#include <string.h>

int main(void)
{
  jas_conf_clear();
  jas_conf_set_max_mem_usage(0x100000);
  jas_init_library();
  jas_cleanup_library();
  return strcmp(JAS_VERSION, CMAKE_EXPECTED_JASPER_VERSION);
}
