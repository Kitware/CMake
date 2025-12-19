#include <string.h>

unsigned char const long_unsigned[] = {
#include "long.c.txt"
};

char const long_signed[] = {
#include "long_signed.c.txt"
};

unsigned char const long_decimal[] = {
#include "long_decimal.c.txt"
};

char const long_signed_decimal[] = {
#include "long_signed_decimal.c.txt"
};

#define VERIFY(contents)                                                      \
  do {                                                                        \
    if (sizeof(long_unsigned) != sizeof(contents)) {                          \
      return 1;                                                               \
    }                                                                         \
    if (memcmp(long_unsigned, contents, sizeof(contents))) {                  \
      return 1;                                                               \
    }                                                                         \
  } while (0)

int main(void)
{
  VERIFY(long_signed);
  VERIFY(long_decimal);
  VERIFY(long_signed_decimal);
  return 0;
}
