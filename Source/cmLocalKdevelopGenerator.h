/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalKdevelopGenerator_h
#define cmLocalKdevelopGenerator_h

#include "cmLocalUnixMakefileGenerator2.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalKdevelopGenerator
 */
class cmLocalKdevelopGenerator : public cmLocalUnixMakefileGenerator2
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalKdevelopGenerator();

  virtual ~cmLocalKdevelopGenerator();

};

#endif
