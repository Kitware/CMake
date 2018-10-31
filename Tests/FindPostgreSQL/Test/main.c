#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>

int main()
{
  int version = PQlibVersion();
  int major = version / 10000;
  int minor = version % 10000;
  char version_string[100];
  snprintf(version_string, sizeof(version_string), "%d.%d", major, minor);
  printf("Found PostgreSQL version %s, expected version %s\n", version_string,
         CMAKE_EXPECTED_POSTGRESQL_VERSION);
  return strcmp(version_string, CMAKE_EXPECTED_POSTGRESQL_VERSION);
}
