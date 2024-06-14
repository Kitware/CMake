#include <assert.h>
#include <spirv-tools/libspirv.h>
#include <stdio.h>

int main(void)
{
  const char* spv_version = spvSoftwareVersionString();
  const char* spv_details = spvSoftwareVersionDetailsString();
  assert(spv_version);
  assert(spv_details);

  printf("SPIRV-Tools version: %s (details: %s)", spv_version, spv_details);

  return 0;
}
