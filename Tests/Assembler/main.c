#include <stdio.h>

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
{
  printf("hello assembler world, %d arguments  given\n", argc);
  return 0;
}
