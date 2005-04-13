#include "kwsysPrivate.h"
#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/iostream)

int main()
{
  kwsys_ios::ostringstream ostr;
  ostr << "Hello, World!";
  kwsys_ios::cout << ostr.str() << kwsys_ios::endl;
  return 0;
}
