#include <cstdlib>

#include "binary.h"

int main()
{
  if (sizeof(imageBytes) != 8) {
    return 1;
  }

  for (size_t i = 0; i < 8; i++) {
    if (imageBytes[i] != i) {
      return 1;
    }
  }

  return 0;
}
