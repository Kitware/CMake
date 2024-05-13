#include <stdio.h>

void FiveFunction();
void EightFunction();

int main(void)
{
  FiveFunction();
  EightFunction();

  printf("Dependency test executable ran successfully.\n");

  return 0;
}
