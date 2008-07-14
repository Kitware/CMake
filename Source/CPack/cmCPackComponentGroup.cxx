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

#include "cmCPackComponentGroup.h"
#include "cmSystemTools.h"
#include <vector>
#include <string>

//----------------------------------------------------------------------
unsigned long cmCPackComponent::GetInstalledSize(const char* installDir) const
{
  if (this->TotalSize != 0)
    {
    return this->TotalSize;
    }

  std::vector<std::string>::const_iterator fileIt;
  for (fileIt = this->Files.begin(); fileIt != this->Files.end(); ++fileIt)
    {
    std::string path = installDir;
    path += '/';
    path += *fileIt;
    this->TotalSize += cmSystemTools::FileLength(path.c_str());
    }

  return this->TotalSize;
}

//----------------------------------------------------------------------
unsigned long 
cmCPackComponent::GetInstalledSizeInKbytes(const char* installDir) const
{
  unsigned long result = (GetInstalledSize(installDir) + 512) / 1024;
  return result? result : 1;
}
