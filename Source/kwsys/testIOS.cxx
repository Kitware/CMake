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
  const char refstring[] =  "Hello, World!";
  kwsys_ios::ostringstream ostr;
  ostr << refstring;
  kwsys_ios::cout << ostr.str() << kwsys_ios::endl;
  if( ostr.str() != refstring )
    {
    return 1;
    }


  kwsys_ios::istringstream istr;
  istr.str( refstring );
  kwsys_ios::cout << istr.str() << kwsys_ios::endl;
  if( istr.str() != refstring )
    {
    return 1;
    }


  const int val = 12345;
  const char valstr[] = "12345";
  kwsys_ios::stringstream sstr;
  sstr << val;
  int v = 0;
  sstr >> v;
  if(v != val || sstr.str() != valstr)
    {
    return 1;
    }
  kwsys_ios::cout << sstr.str() << kwsys_ios::endl;

  return 0;
}
