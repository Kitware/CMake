#ifndef _MSC_VER
#  define winexport
#else
#  ifdef autoexport_EXPORTS
#    define winexport
#  else
#    define winexport __declspec(dllimport)
#  endif
#endif

class Hello
{
public:
  static winexport int Data;
  void real();
  static void operator delete[](void*);
  static void operator delete(void*);
};

// In the MSVC ABI, a delegating constructor references the vftable.
#if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
#  define HELLO_VFTABLE
#endif
#ifdef HELLO_VFTABLE
class HelloVFTable
{
public:
  HelloVFTable();
  HelloVFTable(int)
    : HelloVFTable()
  {
  }
  virtual ~HelloVFTable();
};
#endif
