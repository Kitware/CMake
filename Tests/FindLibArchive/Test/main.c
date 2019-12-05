#include <archive.h>

int main(void)
{
  archive_read_free(archive_read_new());
  return 0;
}
