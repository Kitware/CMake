#include <example.h>

#include <stdio.h>

#if defined(_WIN32)
# define MODULE_EXPORT __declspec(dllexport)
#else
# define MODULE_EXPORT
#endif

MODULE_EXPORT int example_mod_1_function()
{
  int result = example_exe_function() + 456;
  printf("world\n");
  return result;
}
