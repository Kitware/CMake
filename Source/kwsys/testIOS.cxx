#include "kwsysPrivate.h"
#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "kwsys_stl_vector.h.in"
# include "kwsys_ios_sstream.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

int main()
{
  kwsys_ios::ostringstream ostr;
  ostr << "Hello, World!";
  kwsys_ios::cout << ostr.str() << kwsys_ios::endl;
  return 0;
}
