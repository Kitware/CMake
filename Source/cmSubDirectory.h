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
#ifndef cmSubDirectory_h
#define cmSubDirectory_h

#include "cmStandardIncludes.h"

/** \class cmSubDirectory
 * \brief A class to encapsulate a custom command
 *
 * cmSubDirectory encapsulates the properties of a custom command
 */
class cmSubDirectory
{
public:
  cmSubDirectory() { IncludeTopLevel = true; PreOrder = false; }
  
  std::string SourcePath;
  std::string BinaryPath;
  bool IncludeTopLevel;
  bool PreOrder;
};

#endif
