#include <stdio.h>

const char* secondone();

int main()
{
  printf("Hello from subdirectory\n");
  printf("SO: %s\n", secondone());
  return 0;
}
