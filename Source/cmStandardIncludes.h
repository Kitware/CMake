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
#endif


#ifdef __ICL
#pragma warning ( disable : 985 )
#pragma warning ( disable : 1572 ) /* floating-point equality test */
#endif

// Provide fixed-size integer types.
#include <cmIML/INT.h>

// Include stream compatibility layer from KWSys.
// This is needed to work with large file support
// on some platforms whose stream operators do not
// support the large integer types.
#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/IOStream.hxx>
#endif

// Avoid warnings in system headers.
#if defined(_MSC_VER)
# pragma warning (push,1)
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

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

// include the "c" string header
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( _MSC_VER )
typedef unsigned short mode_t;
#endif

// use this class to shrink the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
#include <cmsys/String.hxx>
//typedef cmsys::String std::string;

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
#endif

/* Poison this operator to avoid common mistakes.  */
extern void operator << (std::ostream&, const std::ostringstream&);

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  std::string Name;
  std::string Brief;
  cmDocumentationEntry(){}
  cmDocumentationEntry(const char *doc[2])
  { if (doc[0]) this->Name = doc[0];
  if (doc[1]) this->Brief = doc[1];}
  cmDocumentationEntry(const char *n, const char *b)
  { if (n) this->Name = n; if (b) this->Brief = b; }
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
} \
class cmTypeMacro_UseTrailingSemicolon

inline bool cmHasLiteralPrefixImpl(const std::string &str1,
                                 const char *str2,
                                 size_t N)
{
  return strncmp(str1.c_str(), str2, N) == 0;
}

inline bool cmHasLiteralPrefixImpl(const char* str1,
                                 const char *str2,
                                 size_t N)
{
  return strncmp(str1, str2, N) == 0;
}

inline bool cmHasLiteralSuffixImpl(const std::string &str1,
                                   const char *str2,
                                   size_t N)
{
  size_t len = str1.size();
  return len >= N && strcmp(str1.c_str() + len - N, str2) == 0;
}

inline bool cmHasLiteralSuffixImpl(const char* str1,
                                   const char* str2,
                                   size_t N)
{
  size_t len = strlen(str1);
  return len >= N && strcmp(str1 + len - N, str2) == 0;
}

template<typename T, size_t N>
const T* cmArrayBegin(const T (&a)[N]) { return a; }
template<typename T, size_t N>
const T* cmArrayEnd(const T (&a)[N]) { return a + N; }
template<typename T, size_t N>
size_t cmArraySize(const T (&)[N]) { return N; }

template<typename T, size_t N>
bool cmHasLiteralPrefix(T str1, const char (&str2)[N])
{
  return cmHasLiteralPrefixImpl(str1, str2, N - 1);
}

template<typename T, size_t N>
bool cmHasLiteralSuffix(T str1, const char (&str2)[N])
{
  return cmHasLiteralSuffixImpl(str1, str2, N - 1);
}

struct cmStrCmp {
  cmStrCmp(const char *test) : m_test(test) {}
  cmStrCmp(const std::string &test) : m_test(test) {}

  bool operator()(const std::string& input) const
  {
    return m_test == input;
  }

  bool operator()(const char * input) const
  {
    return strcmp(input, m_test.c_str()) == 0;
  }

private:
  const std::string m_test;
};

#endif
