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
#ifndef cmGlobalXCode21Generator_h
#define cmGlobalXCode21Generator_h

#include "cmGlobalXCodeGenerator.h"

/** \class cmGlobalXCode21Generator
 * \brief Write Mac XCode projects
 *
 * cmGlobalXCode21Generator manages UNIX build process for a tree
 */
class cmGlobalXCode21Generator : public cmGlobalXCodeGenerator
{
public:
  cmGlobalXCode21Generator();
  static cmGlobalGenerator* New() { return new cmGlobalXCode21Generator; }
  virtual void  WriteXCodePBXProj(std::ostream& fout,
                                  cmLocalGenerator* root,
                                  std::vector<cmLocalGenerator*>& generators);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGInitDirectory()  { return "$(CONFIGURATION)"; }
};

#endif
