#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  int version = PQlibVersion();
  char version_string[100];
  // 9.x and older encoding.
  if (version < 100000) {
    int major = version / 10000;
    int minor = version % 10000 / 100;
    int patch = version % 100;
    snprintf(version_string, sizeof(version_string), "%d.%d.%d", major, minor,
             patch);
  } else {
    int major = version / 10000;
    int minor = version % 10000;
    snprintf(version_string, sizeof(version_string), "%d.%d", major, minor);
  }
  printf("Found PostgreSQL version %s, expected version %s\n", version_string,
         CMAKE_EXPECTED_POSTGRESQL_VERSION);
  return strcmp(version_string, CMAKE_EXPECTED_POSTGRESQL_VERSION);
}
