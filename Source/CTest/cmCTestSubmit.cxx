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

#include "cmCTestSubmit.h"
#include "cmSystemTools.h"

bool cmCTestSubmit::SubmitUsingFTP(const std::vector<std::string>& files,
                                   const std::string& prefix, 
                                   const std::string& url)
{
  std::string::size_type cc;
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    std::cout << "upload file: " << files[cc].c_str() << " to " << url.c_str()
              << " / " << prefix.c_str() << " " << files[cc].c_str() << std::endl;
    }
}

bool cmCTestSubmit::SubmitUsingSCP(const std::vector<std::string>& files,
                                   const std::string& prefix, 
                                   const std::string& url)
{
  std::cout << "SubmitUsingSCP is not yet implemented" << std::endl;
}
