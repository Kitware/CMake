#include <stdio.h>

extern int own_auto_export_function(int i);

void hello2(void)
{
  printf("hello exec:%i", own_auto_export_function(41));
}
