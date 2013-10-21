/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/**
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

// include configure generated  header to define CMAKE_NO_ANSI_STREAM_HEADERS,
// CMAKE_NO_STD_NAMESPACE, and other macros.
#include <cmConfigure.h>
#include <cmsys/Configure.hxx>

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#pragma warning ( disable : 4512 ) /* operator=() could not be generated */
#define CMAKE_NO_ANSI_FOR_SCOPE
#endif

#ifdef __BORLANDC__
# pragma warn -8030 /* Temporary used for parameter */
# pragma warn -8027 /* 'for' not inlined.  */
# pragma warn -8026 /* 'exception' not inlined.  */
# pragma warn -8004 /* value never used */
#endif

#ifdef __ICL
#pragma warning ( disable : 985 )
#pragma warning ( disable : 1572 ) /* floating-point equality test */
#endif

#include <stdarg.h> // Work-around for SGI MIPSpro 7.4.2m header bug

// This is a hack to prevent warnings about these functions being
// declared but not referenced.
#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 3970 /* conversion from pointer to same-sized */
# include <sys/termios.h>
class cmStandardIncludesHack
{
public:
  enum
  {
    Ref1 = sizeof(cfgetospeed(0)),
    Ref2 = sizeof(cfgetispeed(0)),
    Ref3 = sizeof(tcgetattr(0, 0)),
    Ref4 = sizeof(tcsetattr(0, 0, 0)),
    Ref5 = sizeof(cfsetospeed(0,0)),
    Ref6 = sizeof(cfsetispeed(0,0))
  };
};
#endif

// Include stream compatibility layer from KWSys.
// This is needed to work with large file support
// on some platforms whose stream operators do not
// support the large integer types.
#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/IOStream.hxx>
# undef GetCurrentDirectory // Borland <iosfwd> includes windows.h
#endif

// Avoid warnings in system headers.
#if defined(_MSC_VER)
# pragma warning (push,1)
#endif
#if defined(__BORLANDC__)
# pragma warn -8008 /* condition is always false (RESET BELOW!) */
# pragma warn -8066 /* unreachable code (RESET BELOW!) */
#endif

#ifndef CMAKE_NO_ANSI_STREAM_HEADERS
#  include <fstream>
#  include <iostream>
#  include <iomanip>
#else
#  include <fstream.h>
#  include <iostream.h>
#  include <iomanip.h>
#endif

#if !defined(CMAKE_NO_ANSI_STRING_STREAM)
#  include <sstream>
#elif !defined(CMAKE_NO_ANSI_STREAM_HEADERS)
#  include <strstream>
#else
#  include <strstream.h>
#endif

// we must have stl with the standard include style
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <map>
#include <list>
#include <set>
#include <deque>

#if defined(__BORLANDC__)
# pragma warn .8008 /* condition is always false (disabled above) */
# pragma warn .8066 /* unreachable code (disabled above) */
#endif
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

// include the "c" string header
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Borland C++ defines several of the stdlib.h and string.h symbols in
// sub-headers search.h and mem.h.  These sub-headers have using
// declarations to pull functions from the std namespace to the global
// namespace, but they are defined only if the header was not included
// through the C++-style cstdlib or cstring header.  These outer
// headers are included by the streams library in C++-style and
// include blockers are put in place that prevent including the
// C-style versions from ever including the sub-headers.  Therefore we
// have to include the sub-headers here to get the using declarations.
#if defined(__BORLANDC__)
# include <mem.h>    /* mem... functions from string.h */
# include <search.h> /* search functions from stdlib.h */
#endif


#if !defined(_WIN32) && defined(__COMO__)
// Hack for como strict mode to avoid defining _SVID_SOURCE or _BSD_SOURCE.
extern "C"
{
extern FILE *popen (__const char *__command, __const char *__modes) __THROW;
extern int pclose (FILE *__stream) __THROW;
extern char *realpath (__const char *__restrict __name,
                       char *__restrict __resolved) __THROW;
extern char *strdup (__const char *__s) __THROW;
extern int putenv (char *__string) __THROW;
}
#endif

// if std:: is not supported, then just #define it away
#ifdef CMAKE_NO_STD_NAMESPACE
#define std
#endif

// if the compiler does not support ansi for scoping of vars use a
// #define hack
#ifdef CMAKE_NO_ANSI_FOR_SCOPE
#define for if(false) {} else for
#endif

// Provide std::ios_base on ancient GCC 2.9x
#if defined(__GNUC__) && __GNUC__ < 3
namespace std { typedef ios ios_base; }
#endif

// check for the 720 compiler on the SGI
// which has some strange properties that I don't think are worth
// checking for in a general way in configure
#if defined(__sgi) && !defined(__GNUC__)
#  if   (_COMPILER_VERSION >= 730)
#   define CM_SGI_CC_730
#  elif (_COMPILER_VERSION >= 720)
#   define CM_HAS_STD_BUT_NOT_FOR_IOSTREAM
#  endif
#endif

#ifdef __DECCXX_VER
# if __DECCXX_VER <= 60390008
#  define CM_HAS_STD_BUT_NOT_FOR_IOSTREAM
# endif
#endif

#if defined( _MSC_VER )
typedef unsigned short mode_t;
#endif


#ifdef CM_HAS_STD_BUT_NOT_FOR_IOSTREAM
// some compilers have std:: but not for the stream library,
// so we have to bring it into the std namespace by hand.
namespace std {
using ::ostream;
using ::istream;
using ::ios;
using ::cout;
using ::cerr;
using ::cin;
using ::ifstream;
using ::ofstream;

#if !defined(CMAKE_NO_ANSI_STRING_STREAM)
  using ::ostringstream;
  using ::istringstream;
#else
  using ::ostrstream;
  using ::istrstream;
#endif

using ::endl;
using ::ends;
using ::flush;
using ::dec;
using ::hex;
using ::setw;
using ::setiosflags;
using ::setfill;
using ::setprecision;
}
// The string class is missing these operators so add them
#if !defined(cmsys_STL_STRING_NEQ_CHAR_DEFINED)
# define cmsys_STL_STRING_NO_NEQ_CHAR
inline bool operator!=(std::string const& a, const char* b)
{ return !(a==std::string(b)); }
#endif

inline bool operator==(std::string const& a, const char* b)
{ return (a==std::string(b)); }
# endif  // end CM_SGI_CC_720

#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 1375 /* base class destructor not virtual */
#endif

// use this class to shrink the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
#include <cmsys/String.hxx>
typedef cmsys::String cmStdString;

// Define cmOStringStream and cmIStringStream wrappers to hide
// differences between std::stringstream and the old strstream.
#if !defined(CMAKE_NO_ANSI_STRING_STREAM)
class cmOStringStream: public std::ostringstream
{
public:
  cmOStringStream();
  ~cmOStringStream();
private:
  cmOStringStream(const cmOStringStream&);
  void operator=(const cmOStringStream&);
};
class cmIStringStream: public std::istringstream
{
public:
  typedef std::istringstream Superclass;
  cmIStringStream() {}
  cmIStringStream(const std::string& s): Superclass(s) {}
private:
  cmIStringStream(const cmIStringStream&);
  void operator=(const cmIStringStream&);
};
#else
class cmOStrStreamCleanup
{
public:
  cmOStrStreamCleanup(std::ostrstream& ostr): OStrStream(ostr) {}
  ~cmOStrStreamCleanup() { this->OStrStream.rdbuf()->freeze(0); }
  static void IgnoreUnusedVariable(const cmOStrStreamCleanup&) {}
protected:
  std::ostrstream& OStrStream;
};

class cmOStringStream: public std::ostrstream
{
public:
  typedef std::ostrstream Superclass;
  cmOStringStream() {}
  std::string str()
    {
    cmOStrStreamCleanup cleanup(*this);
    cmOStrStreamCleanup::IgnoreUnusedVariable(cleanup);
    int pcount = this->pcount();
    const char* ptr = this->Superclass::str();
    return std::string(ptr?ptr:"", pcount);
    }
private:
  cmOStringStream(const cmOStringStream&);
  void operator=(const cmOStringStream&);
};

class cmIStringStream: private std::string, public std::istrstream
{
public:
  typedef std::string StdString;
  typedef std::istrstream IStrStream;
  cmIStringStream(): StdString(), IStrStream(StdString::c_str()) {}
  cmIStringStream(const std::string& s):
    StdString(s), IStrStream(StdString::c_str()) {}
  std::string str() const { return *this; }
  void str(const std::string& s)
    {
    // Very dangerous.  If this throws, the object is hosed.  When the
    // destructor is later called, the program is hosed too.
    this->~cmIStringStream();
    new (this) cmIStringStream(s);
    }
private:
  cmIStringStream(const cmIStringStream&);
  void operator=(const cmIStringStream&);
};
#endif

/* Poison this operator to avoid common mistakes.  */
extern void operator << (std::ostream&, const cmOStringStream&);

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  std::string Name;
  std::string Brief;
  cmDocumentationEntry(){};
  cmDocumentationEntry(const char *doc[2])
  { if (doc[0]) this->Name = doc[0];
  if (doc[1]) this->Brief = doc[1];};
  cmDocumentationEntry(const char *n, const char *b)
  { if (n) this->Name = n; if (b) this->Brief = b; };
};

/** Data structure to represent a single command line.  */
class cmCustomCommandLine: public std::vector<std::string>
{
public:
  typedef std::vector<std::string> Superclass;
  typedef Superclass::iterator iterator;
  typedef Superclass::const_iterator const_iterator;
};

/** Data structure to represent a list of command lines.  */
class cmCustomCommandLines: public std::vector<cmCustomCommandLine>
{
public:
  typedef std::vector<cmCustomCommandLine> Superclass;
  typedef Superclass::iterator iterator;
  typedef Superclass::const_iterator const_iterator;
};

#if defined(__sgi) && !defined(__GNUC__)
# pragma reset woff 1375 /* base class destructor not virtual */
#endif

// All subclasses of cmCommand or cmCTestGenericHandler should
// invoke this macro.
#define cmTypeMacro(thisClass,superclass) \
virtual const char* GetNameOfClass() { return #thisClass; } \
typedef superclass Superclass; \
static bool IsTypeOf(const char *type) \
{ \
  if ( !strcmp(#thisClass,type) ) \
    { \
    return true; \
    } \
  return Superclass::IsTypeOf(type); \
} \
virtual bool IsA(const char *type) \
{ \
  return thisClass::IsTypeOf(type); \
} \
static thisClass* SafeDownCast(cmObject *c) \
{ \
  if ( c && c->IsA(#thisClass) ) \
    { \
    return static_cast<thisClass *>(c); \
    } \
  return 0;\
}

template<typename T, size_t N>
const T* cmArrayBegin(const T (&a)[N]) { return a; }
template<typename T, size_t N>
const T* cmArrayEnd(const T (&a)[N]) { return a + N; }
template<typename T, size_t N>
size_t cmArraySize(const T (&)[N]) { return N; }

struct cmStrCmp {
  cmStrCmp(const char *test) : m_test(test) {}
  cmStrCmp(std::string &test) : m_test(test.c_str()) {}

  bool operator()(const char * input)
  {
    return strcmp(input, m_test) == 0;
  }

private:
  const char *m_test;
};

#endif
