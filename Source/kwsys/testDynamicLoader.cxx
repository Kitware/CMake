/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"

#include KWSYS_HEADER(DynamicLoader.hxx)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "DynamicLoader.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

int TestDynamicLoader(const char* libname, const char* symbol)
{
  kwsys::LibHandle l = kwsys::DynamicLoader::OpenLibrary(libname);
  if( l )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  kwsys::DynamicLoaderFunction f =
    kwsys::DynamicLoader::GetSymbolAddress(l, symbol);
  if( f )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  int success = kwsys::DynamicLoader::CloseLibrary(l);
  if( success )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  return 0;
}

int main(int , char *[])
{
  int res = TestDynamicLoader("foobar.lib", "foobar");

  return res;
}
