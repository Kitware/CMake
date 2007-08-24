#include <stdio.h>

extern int foo(void);
extern int bar(void);
extern int foobar(void);
extern int barbar(void);

int main()
{
   printf("foo: %d bar: %d foobar: %d barbar: %d\n", foo(), bar(), foobar(), barbar());
   return 0;
}
