#ifdef TEST_IMAGE_MAGICK_7
#  include <MagickWand/MagickWand.h>
#else
#  include <wand/MagickWand.h>
#endif

int main(void)
{
  MagickWand* wand = NewMagickWand();
  wand = DestroyMagickWand(wand);
  return 0;
}
