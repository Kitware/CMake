#include <assert.h>
// clang-format off
#include <stdio.h>
#include <jasper/jasper.h>
// clang-format on

int main(void)
{
  /* Without any JPEG file to open, test that the call fails as
     expected.  This tests that linking worked. */
  jas_init();
  jas_image_t* img = jas_image_create0();
  jas_image_destroy(img);
  jas_cleanup();

  return (JAS_VERSION != CMAKE_EXPECTED_JASPER_VERSION);
}
