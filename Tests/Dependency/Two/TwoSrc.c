#include <two-test.h>

void TwoFunction(void)
{
  static int count = 0;
  if (count == 0) {
    ++count;
    ThreeFunction();
  }
}
