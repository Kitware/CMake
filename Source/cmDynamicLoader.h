/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME cmDynamicLoader - class interface to system dynamic libraries
// .SECTION Description
// cmDynamicLoader provides a portable interface to loading dynamic 
// libraries into a process.  


#ifndef __cmDynamicLoader_h
#define __cmDynamicLoader_h

#include "cmStandardIncludes.h"

// Ugly stuff for library handles
// They are different on several different OS's
#if defined(__hpux)
# include <dl.h>
  typedef shl_t cmLibHandle;
#elif defined(_WIN32)
  #include <windows.h>
  typedef HMODULE cmLibHandle;
#else
  typedef void* cmLibHandle;
#endif

// Return type from cmDynamicLoader::GetSymbolAddress.
typedef void (*cmDynamicLoaderFunction)();

class cmDynamicLoader
{
public:
  // Description:
  // Load a dynamic library into the current process.
  // The returned cmLibHandle can be used to access the symbols in the 
  // library.
  static cmLibHandle OpenLibrary(const char*);

  // Description:
  // Attempt to detach a dynamic library from the
  // process.  A value of true is returned if it is successful.
  static int CloseLibrary(cmLibHandle);
  
  // Description:
  // Find the address of the symbol in the given library
  static cmDynamicLoaderFunction GetSymbolAddress(cmLibHandle, const char*);

  // Description:
  // Return the library prefix for the given architecture
  static const char* LibPrefix();

  // Description:
  // Return the library extension for the given architecture
  static const char* LibExtension();

  // Description:
  // Return the last error produced from a calls made on this class.
  static const char* LastError();
  
protected:
  cmDynamicLoader() {};
  ~cmDynamicLoader() {};

  
private:
  cmDynamicLoader(const cmDynamicLoader&);  // Not implemented.
  void operator=(const cmDynamicLoader&);  // Not implemented.
};

#endif
