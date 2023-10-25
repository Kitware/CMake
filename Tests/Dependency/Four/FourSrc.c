#include <two-test.h> /* Requires TwoCustom to be built first.  */
void NoDepAFunction(void);
void OneFunction(void);
void TwoFunction(void);

void FourFunction(void)
{
  static int count = 0;
  if (count == 0) {
    ++count;
    TwoFunction();
  }
  OneFunction();
  NoDepAFunction();
}
