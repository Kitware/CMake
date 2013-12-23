/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

#include KWSYS_HEADER(FStream.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "FStream.hxx.in"
#endif


//----------------------------------------------------------------------------
static int testNoFile()
{
  kwsys::ifstream in_file("NoSuchFile.txt");
  if(in_file)
    {
    return 1;
    }

  return 0;
}


//----------------------------------------------------------------------------
int testFStream(int, char*[])
{
  int ret = 0;

  ret |= testNoFile();

  return ret;
}
