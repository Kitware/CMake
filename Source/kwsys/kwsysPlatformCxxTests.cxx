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
int main() { return 0; }
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
