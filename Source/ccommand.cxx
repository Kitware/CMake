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
#include "time.h"

void CMakeCommandUsage(const char* program)
{
  std::strstream errorStream;

  errorStream 
    << "cmake version " << cmMakefile::GetMajorVersion()
    << "." << cmMakefile::GetMinorVersion() << "\n";

  errorStream 
    << "Usage: " << program << " [command] [arguments ...]\n"
    << "Available commands: \n"
    << "  copy file destination   - copy file to destination (either file or directory)\n"
    << "  remove file1 file2 ...  - remove the file(s)\n"
    << "  time command [args] ... - run command and return elapsed time\n"
#if defined(_WIN32) && !defined(__CYGWIN__)
    << "  write_regv key value    - write registry value\n"
    << "  delete_regv key         - delete registry value\n"
#endif
    << std::ends;

  cmSystemTools::Error(errorStream.str());
}

int main(int ac, char** av)
{
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    args.push_back(av[i]);
    }

  if (args.size() > 1)
    {
    // Copy file
    if (args[1] == "copy" && args.size() == 4)
      {
      cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str());
      return cmSystemTools::GetErrorOccuredFlag();
      }

    // Remove file
    else if (args[1] == "remove" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
	{
        if(args[cc] != "-f")
          {
          if(args[cc] == "\\-f")
            {
            args[cc] = "-f";
            }
          cmSystemTools::RemoveFile(args[cc].c_str());
          }
	}
      return 0;
      }

    // Clock command
    else if (args[1] == "time" && args.size() > 2)
      {
      std::string command = args[2];
      std::string output;
      for (std::string::size_type cc = 3; cc < args.size(); cc ++)
	{
        command += " ";
        command += args[cc];
	}

      clock_t clock_start, clock_finish;
      time_t time_start, time_finish;

      time(&time_start);
      clock_start = clock();
      
      cmSystemTools::RunCommand(command.c_str(), output, 0, true);

      clock_finish = clock();
      time(&time_finish);

      std::cout << output.c_str();

      double clocks_per_sec = (double)CLOCKS_PER_SEC;
      std::cout << "Elapsed time: " 
                << (time_t)(time_finish - time_start) << " s. (time)"
                << ", " 
                << (double)(clock_finish - clock_start) / clocks_per_sec 
                << " s. (clock)"
                << "\n";
      return 0;
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Write registry value
    else if (args[1] == "write_regv" && args.size() > 3)
      {
      return cmSystemTools::WriteRegistryValue(args[2].c_str(), 
                                               args[3].c_str()) ? 0 : 1;
      }

    // Delete registry value
    else if (args[1] == "delete_regv" && args.size() > 2)
      {
      return cmSystemTools::DeleteRegistryValue(args[2].c_str()) ? 0 : 1;
      }
#endif

    }

  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}
