/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/**
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

#define CMAKE_TO_STRING(x) CMAKE_TO_STRING0(x)
#define CMAKE_TO_STRING0(x) #x

// include configure generated  header to define CMAKE_NO_ANSI_STREAM_HEADERS,
// CMAKE_NO_STD_NAMESPACE, and other macros.
#include "cmConfigure.h"

#define CMake_VERSION_STRING \
  CMAKE_TO_STRING(CMake_VERSION_MAJOR) "." \
  CMAKE_TO_STRING(CMake_VERSION_MINOR) "." \
  CMAKE_TO_STRING(CMake_VERSION_PATCH)

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#define CMAKE_NO_ANSI_FOR_SCOPE
#endif

#ifdef __ICL
#pragma warning ( disable : 985 )
#endif

#ifndef CMAKE_NO_ANSI_STREAM_HEADERS
#  include <fstream>
#  include <iostream>
#else
#  include <fstream.h>
#  include <iostream.h>
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

// include the "c" string header
#include <string.h>

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
}
// The string class is missing these operators so add them
inline bool operator!=(std::string const& a, const char* b)
{ return !(a==std::string(b)); }

inline bool operator==(std::string const& a, const char* b)
{ return (a==std::string(b)); }
# endif  // end CM_SGI_CC_720

// use this class to shrink the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
struct cmStdString : public std::string
{
  typedef std::string StdString;
  typedef StdString::value_type             value_type;
  typedef StdString::pointer                pointer;
  typedef StdString::reference              reference;
  typedef StdString::const_reference        const_reference;
  typedef StdString::size_type              size_type;
  typedef StdString::difference_type        difference_type;
  typedef StdString::iterator               iterator;
  typedef StdString::const_iterator         const_iterator;
  typedef StdString::reverse_iterator       reverse_iterator;
  typedef StdString::const_reverse_iterator const_reverse_iterator;
  
  cmStdString(): StdString() {}
  cmStdString(const value_type* s): StdString(s) {}
  cmStdString(const value_type* s, size_type n): StdString(s, n) {}
  cmStdString(const StdString& s, size_type pos=0, size_type n=npos):
    StdString(s, pos, n) {}
};

// Define cmOStringStream and cmIStringStream wrappers to hide
// differences between std::stringstream and the old strstream.
#if !defined(CMAKE_NO_ANSI_STRING_STREAM)
class cmOStringStream: public std::ostringstream
{
public:
  cmOStringStream() {}
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
  cmOStrStreamCleanup(std::ostrstream& ostr): m_OStrStream(ostr) {}
  ~cmOStrStreamCleanup() { m_OStrStream.rdbuf()->freeze(0); }
  static void IgnoreUnusedVariable(const cmOStrStreamCleanup&) {}
protected:
  std::ostrstream& m_OStrStream;
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

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  const char* name;
  const char* brief;
  const char* full;
};

#endif
