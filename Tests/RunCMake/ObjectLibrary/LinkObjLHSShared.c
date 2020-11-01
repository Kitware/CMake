#ifndef REQUIRED
#  error "REQUIRED not defined"
#endif

#if defined(_WIN32)
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int a(void);
extern int required(void);

int main(void)
{
  return required() + a();
}
