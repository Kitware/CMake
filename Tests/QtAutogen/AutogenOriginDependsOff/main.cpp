
#include <string>

#include <a_qt.hpp>
#include <b_qt.hpp>

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
