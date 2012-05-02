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

#include KWSYS_HEADER(Registry.hxx)
#include KWSYS_HEADER(ios/iostream)
#include <string.h>

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Registry.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

#define IFT(x,res) if ( !x )                    \
  {                                             \
  res = 1;                                      \
  kwsys_ios::cout << "Error in: " << #x << kwsys_ios::endl;           \
  }
#define IFNT(x,res) if ( x )                    \
  {                                             \
  res = 1;                                      \
  kwsys_ios::cout << "Error in: " << #x << kwsys_ios::endl;           \
  }

#define CHE(x,y,res) if ( x && strcmp(x,y) )                 \
  {                                                     \
  res = 1;                                              \
  kwsys_ios::cout << "Error, " << x << " != " << y << kwsys_ios::endl;        \
  }

int testRegistry(int, char*[])
{
  int res = 0;
  
  kwsys::Registry reg;
  reg.SetTopLevel("TestRegistry");
  
  IFT(reg.SetValue("TestSubkey",  "TestKey1", "Test Value 1"), res);
  IFT(reg.SetValue("TestSubkey1", "TestKey2", "Test Value 2"), res);
  IFT(reg.SetValue("TestSubkey",  "TestKey3", "Test Value 3"), res);
  IFT(reg.SetValue("TestSubkey2", "TestKey4", "Test Value 4"), res);

  const char *buffer;
  IFT(reg.ReadValue("TestSubkey",  "TestKey1", &buffer), res);
  CHE(buffer, "Test Value 1", res);
  IFT(reg.ReadValue("TestSubkey1", "TestKey2", &buffer), res);
  CHE(buffer, "Test Value 2", res);
  IFT(reg.ReadValue("TestSubkey",  "TestKey3", &buffer), res);
  CHE(buffer, "Test Value 3", res);
  IFT(reg.ReadValue("TestSubkey2", "TestKey4", &buffer), res);
  CHE(buffer, "Test Value 4", res);
 
  IFT(reg.SetValue("TestSubkey",  "TestKey1", "New Test Value 1"), res);
  IFT(reg.SetValue("TestSubkey1", "TestKey2", "New Test Value 2"), res);
  IFT(reg.SetValue("TestSubkey",  "TestKey3", "New Test Value 3"), res);
  IFT(reg.SetValue("TestSubkey2", "TestKey4", "New Test Value 4"), res);

  IFT(reg.ReadValue("TestSubkey",  "TestKey1", &buffer), res);
  CHE(buffer, "New Test Value 1", res);
  IFT(reg.ReadValue("TestSubkey1", "TestKey2", &buffer), res);
  CHE(buffer, "New Test Value 2", res);
  IFT(reg.ReadValue("TestSubkey",  "TestKey3", &buffer), res);
  CHE(buffer, "New Test Value 3", res);
  IFT(reg.ReadValue("TestSubkey2", "TestKey4", &buffer), res);
  CHE(buffer, "New Test Value 4", res);

  IFT( reg.DeleteValue("TestSubkey",  "TestKey1"), res);
  IFNT(reg.ReadValue(  "TestSubkey",  "TestKey1", &buffer), res);
  IFT( reg.DeleteValue("TestSubkey1", "TestKey2"), res);
  IFNT(reg.ReadValue(  "TestSubkey1", "TestKey2", &buffer), res);
  IFT( reg.DeleteValue("TestSubkey",  "TestKey3"), res);
  IFNT(reg.ReadValue(  "TestSubkey",  "TestKey3", &buffer), res);
  IFT( reg.DeleteValue("TestSubkey2", "TestKey4"), res);
  IFNT(reg.ReadValue(  "TestSubkey2", "TestKey5", &buffer), res);  

  const char* longStringWithNewLines = "Value with embedded CR and LF characters CR='\015' LF='\012' CRLF='\015\012'";
  IFT(reg.SetValue("TestSubkeyWithVeryLongInFactSoLongItsHardToImagineAnybodyWouldReallyDoItLongName",  "TestKey1", longStringWithNewLines), res);
  IFT(reg.ReadValue("TestSubkeyWithVeryLongInFactSoLongItsHardToImagineAnybodyWouldReallyDoItLongName", "TestKey1", &buffer), res);
  CHE(buffer, longStringWithNewLines, res);
  IFT(reg.DeleteValue("TestSubkeyWithVeryLongInFactSoLongItsHardToImagineAnybodyWouldReallyDoItLongName",  "TestKey1"), res);
  IFNT(reg.ReadValue("TestSubkeyWithVeryLongInFactSoLongItsHardToImagineAnybodyWouldReallyDoItLongName", "TestKey1", &buffer), res);

  IFT(reg.SetValue("TestSubkeyWith = EqualSignChar",  "TestKey = 1", "Some value"), res);
  IFT(reg.ReadValue("TestSubkeyWith = EqualSignChar",  "TestKey = 1", &buffer), res);
  CHE(buffer, "Some value", res);
  IFT(reg.DeleteValue("TestSubkeyWith = EqualSignChar",  "TestKey = 1"), res);
  IFNT(reg.ReadValue("TestSubkeyWith = EqualSignChar",  "TestKey = 1", &buffer), res);

  if ( res )
    {
    kwsys_ios::cout << "Test failed" << kwsys_ios::endl;
    }
  else
    {
    kwsys_ios::cout << "Test passed" << kwsys_ios::endl;
    }
  return res;
}
