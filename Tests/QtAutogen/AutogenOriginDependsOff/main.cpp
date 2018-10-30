
#include <a_qt.hpp>
#include <b_qt.hpp>
#include <string>

int main()
{
  if (a_qt::mocs_compilation().empty()) {
    return -1;
  }
  if (b_qt::mocs_compilation().empty()) {
    return -1;
  }
  return 0;
}
