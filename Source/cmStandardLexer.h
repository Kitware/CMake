/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
#endif

/* Borland system header defines these macros without first undef-ing them. */
#if defined(__BORLANDC__) && __BORLANDC__ >= 0x580
# undef INT8_MIN
# undef INT16_MIN
# undef INT32_MIN
# undef INT8_MAX
# undef INT16_MAX
# undef INT32_MAX
# undef UINT8_MAX
# undef UINT16_MAX
# undef UINT32_MAX
# include <stdint.h>
#endif

/* Make sure SGI termios does not define ECHO differently.  */
#if defined(__sgi) && !defined(__GNUC__)
# include <sys/termios.h>
# undef ECHO
#endif

/* Define isatty on windows.  */
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h>
# if defined( _MSC_VER )
#  define isatty _isatty
# endif
# define YY_NO_UNISTD_H 1
#endif

/* Make sure malloc and free are available on QNX.  */
#ifdef __QNX__
# include <malloc.h>
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1
#define ECHO

#endif
