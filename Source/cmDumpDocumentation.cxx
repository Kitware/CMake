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
// Program extracts documentation describing commands from
// the CMake system.
// 
#include "cmake.h"

int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  cmake cmi;
  const char* outname = "cmake.html";
  if(ac > 1)
    {
    outname = av[1];
    }
  std::ofstream fout(outname);
  if(!fout)
    {
    std::cerr << "failed to open output file: " << outname << "\n";
    return -1;
    }
  cmi.DumpDocumentationToFile(fout);
  return 0;
}
