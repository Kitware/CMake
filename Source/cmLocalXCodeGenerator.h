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
#ifndef cmLocalXCodeGenerator_h
#define cmLocalXCodeGenerator_h

#include "cmLocalGenerator.h"

/** \class cmLocalXCodeGenerator
 * \brief Write a local Xcode project
 *
 * cmLocalXCodeGenerator produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalXCodeGenerator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalXCodeGenerator();

  virtual ~cmLocalXCodeGenerator();
private:

};

#endif

