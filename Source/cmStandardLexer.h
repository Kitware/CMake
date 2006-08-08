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
#ifndef cmStandardLexer_h
#define cmStandardLexer_h

/* Disable some warnings.  */
#if defined(_MSC_VER)
# pragma warning ( disable : 4127 )
# pragma warning ( disable : 4131 )
# pragma warning ( disable : 4244 )
# pragma warning ( disable : 4251 )
# pragma warning ( disable : 4267 )
# pragma warning ( disable : 4305 )
# pragma warning ( disable : 4309 )
# pragma warning ( disable : 4706 )
# pragma warning ( disable : 4786 )
#endif

#if defined(__BORLANDC__)
# pragma warn -8008 /* condition always returns true */
# pragma warn -8066 /* unreachable code */
/* Borland system header defines these macros without first undef-ing them.  */
# if __BORLANDC__ >= 0x580
#  undef INT8_MIN
#  undef INT16_MIN
#  undef INT32_MIN
#  undef INT8_MAX
#  undef INT16_MAX
#  undef INT32_MAX
#  undef UINT8_MAX
#  undef UINT16_MAX
#  undef UINT32_MAX
#  include <stdint.h>
# endif
#endif

/* Define isatty on windows.  */
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h>
# if defined( _MSC_VER )
#  define isatty _isatty
# endif
# define YY_NO_UNISTD_H 1
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1

/* Avoid display of input matches to standard output.  */
#undef ECHO /* SGI termios defines this differently. */
#define ECHO

#endif
