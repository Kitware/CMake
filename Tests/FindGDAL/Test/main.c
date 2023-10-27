#include <gdal.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  printf("Found GDAL version %s, expected version %s\n", GDAL_RELEASE_NAME,
         CMAKE_EXPECTED_GDAL_VERSION);
  GDALAllRegister();
  return strcmp(GDAL_RELEASE_NAME, CMAKE_EXPECTED_GDAL_VERSION);
}
