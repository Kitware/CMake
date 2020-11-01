#include <stdio.h>
int main(void)
{
  printf(
    "/tmp/hello.c:3:2: \033[35mwarning:\033[0m Hello, World! [-W#warnings]\n");
  return 0;
}
