
#include <string.h>

#define xstr(s) str(s)
#define str(s) #s

int main(void)
{
  return strcmp(xstr(VAR), xstr(GENEX)) == 0 ? 0 : 1;
}
