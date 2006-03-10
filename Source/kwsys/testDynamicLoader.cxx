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
#include KWSYS_HEADER(stl/string)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "DynamicLoader.hxx.in"
# include "kwsys_ios_iostream.h.in"
# include "kwsys_stl_string.hxx.in"
#endif

#include "testSystemTools.h"

kwsys_stl::string GetLibName(const char* lname)
{
  // Construct proper name of lib
  kwsys_stl::string slname;
  slname = kwsys::DynamicLoader::LibPrefix();
  slname += lname;
  slname += kwsys::DynamicLoader::LibExtension();

  return slname;
}

/* libname = Library name (proper prefix, proper extension)
 * System  = symbol to lookup in libname
 * r1: should OpenLibrary succeed ?
 * r2: should GetSymbolAddress succeed ?
 * r3: should CloseLibrary succeed ?
 */
int TestDynamicLoader(const char* libname, const char* symbol, int r1, int r2, int r3)
{
  //kwsys_ios::cerr << "Testing: " << libname << kwsys_ios::endl;
  kwsys::LibHandle l = kwsys::DynamicLoader::OpenLibrary(libname);
  // If result is incompatible with expectation just fails (xor):
  if( (r1 && !l) || (!r1 && l) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  kwsys::DynamicLoaderFunction f = kwsys::DynamicLoader::GetSymbolAddress(l, symbol);
  if( (r2 && !f) || (!r2 && f) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  int s = kwsys::DynamicLoader::CloseLibrary(l);
  if( (r3 && !s) || (!r3 && s) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  return 0;
}

int main(int , char *[])
{
  int res;
  // Make sure that inexistant lib is giving correct result
  res = TestDynamicLoader("azerty_", "foo_bar",0,0,0);
  // Make sure that random binary file cannnot be assimilated as dylib
  res += TestDynamicLoader(TEST_SYSTEMTOOLS_BIN_FILE, "wp",0,0,0);
#ifdef __linux__
  // This one is actually fun to test, since dlopen is by default loaded...wonder why :)
  res += TestDynamicLoader("foobar.lib", "dlopen",0,1,0);
  res += TestDynamicLoader("libdl.so", "dlopen",1,1,1);
  res += TestDynamicLoader("libdl.so", "TestDynamicLoader",1,0,1);
#endif
  // Now try on the generated library
  kwsys_stl::string libname = GetLibName("testDynload");
  res += TestDynamicLoader(libname.c_str(), "dummy",1,0,1);
  res += TestDynamicLoader(libname.c_str(), "TestDynamicLoaderFunction",1,1,1);
  res += TestDynamicLoader(libname.c_str(), "_TestDynamicLoaderFunction",1,0,1);
  res += TestDynamicLoader(libname.c_str(), "TestDynamicLoaderData",1,1,1);
  res += TestDynamicLoader(libname.c_str(), "_TestDynamicLoaderData",1,0,1);

  return res;
}
