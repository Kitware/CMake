#include <wand/MagickWand.h>

int main(void)
{
  MagickWand* wand = NewMagickWand();
  wand = DestroyMagickWand(wand);
  return 0;
}
