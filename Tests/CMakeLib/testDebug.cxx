#include <iostream>
#include <string>

#include "cmDebugTools.h"

#define check(expr, value)                                                    \
  do {                                                                        \
    if (expr != value) {                                                      \
      std::cerr << "Failed to return " #value " for " #expr << std::endl;     \
      retval = 1;                                                             \
    }                                                                         \
  } while (false)

int testDebug(int argc, char** const /*argv*/)
{
  if (argc != 1) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  int retval = 0;
  check(CM_DBG(true), true);
  check(CM_DBG(4), 4);
  check(CM_DBG(1.), 1.);
  check(CM_DBG('c'), 'c');
  check(CM_DBG("literal string"), std::string("literal string"));

  std::string str = "std string";
  check(CM_DBG(str), "std string");
  check(CM_DBG(str.empty()), false);

  return retval;
}
