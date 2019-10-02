#include <iostream>

#include "IncA.hpp"
#include "IncB.hpp"
#include "StyleA.hpp"
#include "StyleB.hpp"

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
