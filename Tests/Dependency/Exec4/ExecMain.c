#include <stdio.h>

void FiveFunction();
void TwoFunction();

int main(void)
{
  FiveFunction();
  TwoFunction();

  printf("Dependency test executable ran successfully.\n");

  return 0;
}
