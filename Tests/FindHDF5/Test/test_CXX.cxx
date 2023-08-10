#include <H5Cpp.h>

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

int main(int argc, const char* argv[])
{
  if (argc != 2) {
    return 1;
  }
  H5File f(argv[1], H5F_ACC_TRUNC);
  return 0;
}
