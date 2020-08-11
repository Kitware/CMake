#include <stdio.h>

#ifdef __cplusplus_cli
#  include <msclr\marshal_cppstd.h>

void cliFunction()
{
  System::String ^ result = "cliFunction";
  result = result->Trim();
  printf(msclr::interop::marshal_as<std::string>(result).c_str());
}
#else
void cliFunction()
{
  printf("cliFunction (but /cli was not passed to the compiler)");
}
#endif

void nonCliFunction()
{
  printf("nonCliFunction");
}
