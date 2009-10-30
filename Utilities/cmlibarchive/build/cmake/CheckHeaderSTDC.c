#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define ISLOWER(c) ('a' <= (c) && (c) <= 'z')
#define TOUPPER(c) (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))
#define XOR(e, f) (((e) && !(f)) || (!(e) && (f)))

int
main()
{
  int i;

  for (i = 0; i < 256; i++) {
    if (XOR(islower(i), ISLOWER(i)) || toupper(i) != TOUPPER(i))
      return 2;
  }
  return 0;
}
