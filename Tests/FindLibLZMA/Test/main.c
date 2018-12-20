#include <assert.h>
#include <lzma.h>
#include <string.h>

static const uint8_t test_string[9] = "123456789";

int main()
{
  static const uint32_t test_vector = 0xCBF43926;

  uint32_t crc = lzma_crc32(test_string, sizeof(test_string), 0);
  assert(crc == test_vector);

  return 0;
}
