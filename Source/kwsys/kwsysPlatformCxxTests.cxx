#ifdef TEST_KWSYS_STL_HAVE_STD
#include <list>
void f(std::list<int>*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_ANSI
#include <iosfwd>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_STD
#include <iosfwd>
void f(std::ostream*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_SSTREAM
#include <sstream>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_STRSTREAM_H
#include <strstream.h>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_STRSTREA_H
#include <strstrea.h>
int main() { return 0; }
#endif
