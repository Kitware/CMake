/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
/**
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

// include configure generated  header to define
// CMAKE_NO_ANSI_STREAM_HEADERS and CMAKE_NO_STD_NAMESPACE
#if defined(CMAKE_HAS_AUTOCONF) || defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmConfigure.h"
#endif

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#define CMAKE_NO_ANSI_FOR_SCOPE
#endif

#ifdef __ICL
#pragma warning ( disable : 985 )
#endif

#ifndef CMAKE_NO_ANSI_STREAM_HEADERS
#include <fstream>
#include <iostream>
#include <strstream>
#else
#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
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
#   define CM_SGI_CC_720
#  endif
#endif

# ifdef CM_SGI_CC_720
// the 720 sgi compiler has std:: but not for the stream library,
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
using ::strstream;
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

// use this class to shring the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
struct cmStdString : public std::string
{
  typedef std::string Parent;
  cmStdString(const char* s) : Parent(s)
    {
    }
  cmStdString(std::string const&s) : Parent(s)
    {
    }
};
  
#endif
