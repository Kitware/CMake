#include <iostream>

#include "IncA.hpp"
#include "IncB.hpp"
#include "StyleA.hpp"
#include "StyleB.hpp"

#ifdef HAVE_CFG_DEBUG
#  include "CfgDebug.hpp"
#endif

#ifdef HAVE_CFG_OTHER
#  include "CfgOther.hpp"
#endif

int main(int argv, char** args)
{
  StyleA styleA;
  StyleB styleB;
  IncA incA;
  IncB incB;

  // Test the TOKEN definition passed on the command line
  std::string token(TOKEN);
  std::cout << "std::string(TOKEN): \"" << token << "\"\n";
  return (token == "hello;") ? 0 : -1;
}
