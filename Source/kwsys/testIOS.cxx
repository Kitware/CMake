#include "kwsysPrivate.h"
#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "kwsys_stl_string.hxx.in"
# include "kwsys_stl_vector.h.in"
# include "kwsys_ios_sstream.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

int testIOS(int, char*[])
{
  kwsys_ios::ostringstream ostr;
  ostr << "hello";
  if(ostr.str() != "hello")
    {
    kwsys_ios::cerr << "failed to write hello to ostr" << kwsys_ios::endl;
    return 1;
    }
  kwsys_ios::istringstream istr(" 10 20 str ");
  kwsys_stl::string s;
  int x;
  if(istr >> x)
    {
    if(x != 10)
      {
      kwsys_ios::cerr << "x != 10" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 10 from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> x)
    {
    if(x != 20)
      {
      kwsys_ios::cerr << "x != 20" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 20 from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> s)
    {
    if(s != "str")
      {
      kwsys_ios::cerr << "s != \"str\"" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read str from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> s)
    {
    kwsys_ios::cerr << "Able to read past end of stream" << kwsys_ios::endl;
    return 1;
    }
  else
    {
    // Clear the failure.
    istr.clear(istr.rdstate() & ~kwsys_ios::ios::eofbit);
    istr.clear(istr.rdstate() & ~kwsys_ios::ios::failbit);
    }
  istr.str("30");
  if(istr >> x)
    {
    if(x != 30)
      {
      kwsys_ios::cerr << "x != 30" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 30 from istr" << kwsys_ios::endl;
    return 1;
    }

  kwsys_ios::stringstream sstr;
  sstr << "40 str2";
  if(sstr >> x)
    {
    if(x != 40)
      {
      kwsys_ios::cerr << "x != 40" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 40 from sstr" << kwsys_ios::endl;
    return 1;
    }
  if(sstr >> s)
    {
    if(s != "str2")
      {
      kwsys_ios::cerr << "s != \"str2\"" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read str2 from sstr" << kwsys_ios::endl;
    return 1;
    }

  kwsys_ios::cout << "IOS tests passed" << kwsys_ios::endl;
  return 0;
}
