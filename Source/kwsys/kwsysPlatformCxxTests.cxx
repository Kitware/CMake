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
# if KWSYS_STL_HAVE_STD
#  define kwsys_stl std
# else
#  define kwsys_stl
# endif
# include <iostream.h>
# include <string>
void f(ostream& os, const kwsys_stl::string& s) { os << s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_ISTREAM
# if KWSYS_STL_HAVE_STD
#  define kwsys_stl std
# else
#  define kwsys_stl
# endif
# include <iostream.h>
# include <string>
void f(istream& is, kwsys_stl::string& s) { is >> s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_NEQ_CHAR
# if KWSYS_STL_HAVE_STD
#  define kwsys_stl std
# else
#  define kwsys_stl
# endif
# include <string>
bool f(const kwsys_stl::string& s) { return s != ""; }
int main() { return 0; }
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
