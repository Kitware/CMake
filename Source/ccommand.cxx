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

#include "cmMakefile.h"
#include "cmSystemTools.h"

void CMakeCommandUsage(const char* program)
{
  std::strstream errorStream;

  errorStream << "cmake version " << cmMakefile::GetMajorVersion()
	      << "." << cmMakefile::GetMinorVersion() << "\n";
  errorStream << "Usage: " << program << " [command] [arguments ...]\n"
	      << "  Available commands: \n"
	      << "    copy file1 file2  - copy first file to the second one\n"
	      << "    remove file1 file2 ... - remove the file(s)\n";
  errorStream << std::ends;
  cmSystemTools::Error(errorStream.str());
}

int main(int ac, char** av)
{
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    args.push_back(av[i]);
    }

  if ( args.size() > 1 )
    {
    if ( args[1] == "copy" && args.size() == 4 )
      {
      cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str());
      return cmSystemTools::GetErrorOccuredFlag();
      }
    if ( args[1] == "remove" && args.size() > 2 )
      {
      for ( std::string::size_type cc = 2; cc < args.size(); cc ++ )
	{
	cmSystemTools::RemoveFile(args[cc].c_str());
	}
      return 0;
      }
    }
  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}
