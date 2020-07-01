#include <fstream>

#include <assert.h>
#include <tiffio.hxx>

int main()
{
  /* Without any TIFF file to open, test that the call fails as
     expected.  This tests that linking worked. */
  TIFF* tiff = TIFFOpen("invalid.tiff", "r");
  assert(!tiff);

  std::ifstream s;
  TIFF* tiffxx = TIFFStreamOpen("invalid.tiff", &s);
  return 0;
}
