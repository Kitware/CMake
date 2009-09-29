extern int __declspec(dllimport) example_dll_function(void);
#ifdef _MSC_VER
extern int __declspec(dllimport) example_dll_2_function(void);
#endif
int example_exe_function(void) { return 0; }
int main(void)
{
  return
    example_dll_function() +
#ifdef _MSC_VER
    example_dll_2_function() +
#endif
    example_exe_function();
}
