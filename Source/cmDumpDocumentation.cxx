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
// Program extracts documentation describing commands from
// the CMake system.
// 
#include "cmake.h"

int main(int ac, char** av)
{
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
