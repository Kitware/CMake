#include <stdio.h>

int foo();
int bar();
int foobar();
int barbar();
int baz();

int main()
{
   printf("foo: %d bar: %d foobar: %d barbar: %d baz: %d\n", foo(), bar(), foobar(), barbar(), baz());
   return 0;
}
