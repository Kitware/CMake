#include <bzlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
  int chunksize = 1024;
  FILE* file = fopen("test.bzip2", "wb");
  char* buf = malloc(sizeof(char) * chunksize);
  int error, rsize;
  unsigned int in, out;
  BZFILE* bzfile = BZ2_bzWriteOpen(&error, file, 64, 1, 10);

  /* Don't actually write anything for the purposes of the test */

  BZ2_bzWriteClose(&error, bzfile, 1, &in, &out);
  free(buf);
  fclose(file);

  remove("test.bzip2");

  printf("Found BZip2 version %s, expected version %s\n", BZ2_bzlibVersion(),
         CMAKE_EXPECTED_BZip2_VERSION);

  return strncmp(BZ2_bzlibVersion(), CMAKE_EXPECTED_BZip2_VERSION,
                 strlen(CMAKE_EXPECTED_BZip2_VERSION));
}
