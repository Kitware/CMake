
#include <interface.h>

int main(int argc, char **argv)
{
  int result = Interface<1, 2, 3, 4>::accumulate();
  return result == 10 ? 0 : 1;
}
