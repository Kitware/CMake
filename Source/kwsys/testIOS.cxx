#include <kwsys/stl/vector>
#include <kwsys/ios/sstream>
#include <kwsys/ios/iostream>

int main()
{
  kwsys_ios::ostringstream ostr;
  ostr << "Hello, World!";
  kwsys_ios::cout << ostr.str() << kwsys_ios::endl;
  return 0;
}
