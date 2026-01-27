#include <assert.h>
#include <lzma.h>
#include <stdio.h>
#include <string.h>

static uint8_t const test_string[9] = "123456789";

int main(void)
{
  static uint32_t const test_vector = 0xCBF43926;

  uint32_t crc = lzma_crc32(test_string, sizeof(test_string), 0);
  assert(crc == test_vector);

  printf("Found LibLZMA version %s, expected version %s\n",
         lzma_version_string(), CMAKE_EXPECTED_LIBLZMA_VERSION);

  return strcmp(lzma_version_string(), CMAKE_EXPECTED_LIBLZMA_VERSION);
}
