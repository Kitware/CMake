#include "libattributes.h"

int main()
{
  libattributes::Class l;
  return l.method_exported() + libattributes::function_exported() +
    libattributes::data_exported - 1;
}
