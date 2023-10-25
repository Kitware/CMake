#include <sqlite3.h>
#include <string.h>

int main(void)
{
  char sqlite3_version[] = SQLITE_VERSION;

  return strcmp(sqlite3_version, CMAKE_EXPECTED_SQLite3_VERSION);
}
