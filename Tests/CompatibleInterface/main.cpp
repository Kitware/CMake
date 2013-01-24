
#ifndef BOOL_PROP1
#error Expected BOOL_PROP1
#endif

#ifndef BOOL_PROP2
#error Expected BOOL_PROP2
#endif

#ifndef BOOL_PROP3
#error Expected BOOL_PROP3
#endif

#include "iface2.h"

int main(int argc, char **argv)
{
  Iface2 if2;
  return if2.foo();
}
