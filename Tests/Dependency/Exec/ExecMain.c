#include <stdio.h>

void NoDepBFunction();
void NoDepCFunction();
void FiveFunction();

int main( )
{
  FiveFunction();
  NoDepBFunction();
  NoDepCFunction();

  printf("Dependency test executable ran successfully.\n");

  return 0;
}
