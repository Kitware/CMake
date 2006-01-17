// Setup for tests that use result of stl namespace test.
#if defined(KWSYS_STL_HAVE_STD)
# if KWSYS_STL_HAVE_STD
#  define kwsys_stl std
# else
#  define kwsys_stl
# endif
#endif

#ifdef TEST_KWSYS_STL_HAVE_STD
#include <list>
void f(std::list<int>*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_ANSI
#include <iosfwd>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_STD
#include <iosfwd>
void f(std::ostream*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_SSTREAM
#include <sstream>
int main()
{ 
  std::ostringstream ostr;
  ostr << "hello";
  if(ostr.str().size() == 5)
    {
    return 0;
    }
  return -1;
}
#endif

#ifdef TEST_KWSYS_IOS_USE_STRSTREAM_H
#include <strstream.h>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_STRSTREA_H
#include <strstrea.h>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_OSTREAM
# include <iostream.h>
# include <string>
void f(ostream& os, const kwsys_stl::string& s) { os << s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_ISTREAM
# include <iostream.h>
# include <string>
void f(istream& is, kwsys_stl::string& s) { is >> s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_NEQ_CHAR
# include <string>
bool f(const kwsys_stl::string& s) { return s != ""; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_CXX_HAS_CSTDDEF
#include <cstddef>
void f(size_t) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_CXX_HAS_NULL_TEMPLATE_ARGS
template <class T> class A;
template <class T> int f(A<T>&);
template <class T> class A
{
public:
  // "friend int f<>(A<T>&)" would conform
  friend int f(A<T>&);
private:
  int x;
};

template <class T> int f(A<T>& a) { return a.x = 0; }
template int f(A<int>&);

int main()
{
  A<int> a;
  return f(a);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_MEMBER_TEMPLATES
template <class U>
class A
{
public:
  U u;
  A(): u(0) {}
  template <class V> V m(V* p) { return *p = u; }
};

int main()
{
  A<short> a;
  int s = 1;
  return a.m(&s);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_FULL_SPECIALIZATION
template <class T> struct A {};
template <> struct A<int*>
{
  static int f() { return 0; }
};
int main() { return A<int*>::f(); }
#endif

#ifdef TEST_KWSYS_CXX_HAS_ARGUMENT_DEPENDENT_LOOKUP
namespace N
{
  class A {};
  int f(A*) { return 0; }
}
void f(void*);
int main()
{
  N::A* a = 0;
  return f(a);
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ITERATOR_TRAITS
#include <iterator>
#include <list>
void f(kwsys_stl::iterator_traits<kwsys_stl::list<int>::iterator>::iterator_category const&) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ITERATOR_CATEGORY
#include <iterator>
#include <list>
void f(kwsys_stl::list<int>::iterator x) { kwsys_stl::iterator_category(x); }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS___ITERATOR_CATEGORY
#include <iterator>
#include <list>
void f(kwsys_stl::list<int>::iterator x) { kwsys_stl::__iterator_category(x); }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_TEMPLATE
#include <memory>
template <class Alloc>
void f(const Alloc&)
{
  typedef typename Alloc::size_type alloc_size_type;
}
int main()
{
  f(kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE
#include <memory>
void f(kwsys_stl::allocator::size_type const&) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_REBIND
#include <memory>
template <class T, class Alloc>
void f(const T&, const Alloc&)
{
  typedef typename Alloc::template rebind<T>::other alloc_type;
}
int main()
{
  f(0, kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT
#include <memory>
void f(kwsys_stl::allocator<char> const& a)
{
  a.max_size(sizeof(int));
}
int main()
{
  f(kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_OBJECTS
#include <vector>
void f(kwsys_stl::vector<int> const& v1)
{
  kwsys_stl::vector<int>(1, 1, v1.get_allocator());
}
int main()
{
  f(kwsys_stl::vector<int>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STAT_HAS_ST_MTIM
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int main()
{
  struct stat stat1;
  (void)stat1.st_mtim.tv_sec;
  (void)stat1.st_mtim.tv_nsec;
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_SAME_LONG_AND___INT64
void function(long**) {}
int main()
{
  __int64** p = 0;
  function(p);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_SAME_LONG_LONG_AND___INT64
void function(long long**) {}
int main()
{
  __int64** p = 0;
  function(p);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CAN_CONVERT_UI64_TO_DOUBLE
void function(double& l, unsigned __int64 const& r)
{
  l = static_cast<double>(r);
}

int main()
{
  double tTo = 0.0;
  unsigned __int64 tFrom = 0;
  function(tTo, tFrom);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CHAR_IS_SIGNED
/* Return 1 for char signed and 0 for char unsigned.  */
int main()
{
  unsigned char uc = 255;
  return (*reinterpret_cast<char*>(&uc) < 0)?1:0;
}
#endif

#ifdef TEST_KWSYS_CXX_TYPE_INFO
/* Collect fundamental type information and save it to a CMake script.  */

/* Include limits.h to get macros indicating long long and __int64.
   Note that certain compilers need special macros to define these
   macros in limits.h.  */
#if defined(_MSC_VER) && !defined(_MSC_EXTENSIONS)
# define _MSC_EXTENSIONS
#endif
#if defined(__GNUC__) && __GNUC__ < 3
# define _GNU_SOURCE
#endif
#include <limits.h>

#include <stdio.h>
#include <string.h>

/* Due to shell differences and limitations of ADD_DEFINITIONS the
   KWSYS_CXX_TYPE_INFO_FILE macro will sometimes have double quotes
   and sometimes not.  This macro will make sure the value is treated
   as a double-quoted string.  */
#define TO_STRING(x) TO_STRING0(x)
#define TO_STRING0(x) TO_STRING1(x)
#define TO_STRING1(x) #x

void f() {}

int main()
{
  /* Construct the output file name.  Some preprocessors will add an
     extra level of double quotes, so strip them.  */
  char fbuf[] = TO_STRING(KWSYS_CXX_TYPE_INFO_FILE);
  char* fname = fbuf;
  if(fname[0] == '"')
    {
    ++fname;
    int len = static_cast<int>(strlen(fname));
    if(len > 0 && fname[len-1] == '"')
      {
      fname[len-1] = 0;
      }
    }

  /* Try to open the output file.  */
  if(FILE* fout = fopen(fname, "w"))
    {
    /* Set the size of standard types.  */
    fprintf(fout, "SET(KWSYS_SIZEOF_CHAR %d)\n", static_cast<int>(sizeof(char)));
    fprintf(fout, "SET(KWSYS_SIZEOF_SHORT %d)\n", static_cast<int>(sizeof(short)));
    fprintf(fout, "SET(KWSYS_SIZEOF_INT %d)\n", static_cast<int>(sizeof(int)));
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG %d)\n", static_cast<int>(sizeof(long)));

    /* Set the size of some non-standard but common types.  */
    /* Check for a limits.h macro for long long to see if the type exists.  */
#if defined(LLONG_MAX) || defined(LONG_LONG_MAX) || defined(LONGLONG_MAX)
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG_LONG %d)\n", static_cast<int>(sizeof(long long)));
#else
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG_LONG 0) # No long long available.\n");
#endif
    /* Check for a limits.h macro for __int64 to see if the type exists.  */
#if defined(_I64_MIN)
    fprintf(fout, "SET(KWSYS_SIZEOF___INT64 %d)\n", static_cast<int>(sizeof(__int64)));
#else
    fprintf(fout, "SET(KWSYS_SIZEOF___INT64 0) # No __int64 available.\n");
#endif

    /* Set the size of some pointer types.  */
    fprintf(fout, "SET(KWSYS_SIZEOF_PDATA %d)\n", static_cast<int>(sizeof(void*)));
    fprintf(fout, "SET(KWSYS_SIZEOF_PFUNC %d)\n", static_cast<int>(sizeof(&f)));

    /* Set whether the native type "char" is signed or unsigned.  */
    unsigned char uc = 255;
    fprintf(fout, "SET(KWSYS_CHAR_IS_SIGNED %d)\n",
            (*reinterpret_cast<char*>(&uc) < 0)?1:0);

    fclose(fout);
    return 0;
    }
  else
    {
    fprintf(stderr, "Failed to write fundamental type info to \"%s\".\n",
            fname);
    return 1;
    }
}
#endif
