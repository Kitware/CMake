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
#include "cmSystemTools.h"
#include "cmWin32ProcessExecution.h"

// this is a test driver program for cmake.
int main (int argc, char *argv[])
{
  if ( argc <= 1 )
    {
    std::cerr << "Usage: " << argv[0] << " executable" << std::endl;
    return 1;
    }
  std::string arg = argv[1];
  if ( (arg.find_first_of(" ") != arg.npos) && 
       (arg.find_first_of("\"") == arg.npos) )
    {
    arg = "\"" + arg + "\"";
    }
  std::string command = arg;
  int cc;
  for ( cc = 2; cc < argc; cc ++ )
    {
    std::string arg = argv[cc];
    if ( (arg.find_first_of(" ") != arg.npos) && 
	 (arg.find_first_of("\"") == arg.npos) )
      {
      arg = "\"" + arg + "\"";
      }
    command += " ";
    command += arg;
    }

  return cmWin32ProcessExecution::Windows9xHack(command.c_str());
}
