#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
  char* s = malloc(100);
  free(s);
  /* Bug: this writes to memory that was already freed. */
  strcpy(s, "Hello world!");
  printf("string is: %s\n", s);
  return 0;
}
