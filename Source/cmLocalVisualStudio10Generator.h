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
#ifndef cmLocalVisualStudio10Generator_h
#define cmLocalVisualStudio10Generator_h

#include "cmLocalVisualStudio7Generator.h"


/** \class cmLocalVisualStudio10Generator
 * \brief Write Visual Studio 10 project files.
 *
 * cmLocalVisualStudio10Generator produces a Visual Studio 10 project
 * file for each target in its directory.
 */
class cmLocalVisualStudio10Generator : public cmLocalVisualStudio7Generator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio10Generator();

  virtual ~cmLocalVisualStudio10Generator();


  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

private:
};
#endif
