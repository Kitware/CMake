#include <IL/ilu.h>

int main(void)
{
  // IL Utilities requires only initialization.
  // Unlike main DevIL there are no shutdown function.
  iluInit();
}
