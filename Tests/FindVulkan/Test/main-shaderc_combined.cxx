#include <assert.h>
#include <shaderc/shaderc.h>
#include <stdio.h>

int main()
{
  unsigned int shaderc_version, shaderc_revision;
  shaderc_get_spv_version(&shaderc_version, &shaderc_revision);

  printf("shaderc version: %u (revision: %u)", shaderc_version,
         shaderc_revision);

  return 0;
}
