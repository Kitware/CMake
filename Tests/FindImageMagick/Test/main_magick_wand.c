#include <wand/MagickWand.h>

int main()
{
  MagickWand* wand = NewMagickWand();
  wand = DestroyMagickWand(wand);
  return 0;
}
