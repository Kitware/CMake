/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackRPMGenerator_h
#define cmCPackRPMGenerator_h


#include "cmCPackGenericGenerator.h"

/** \class cmCPackRPMGenerator
 * \brief A generator for RPM packages
 *
 */
class cmCPackRPMGenerator : public cmCPackGenericGenerator
{
public:
  cmCPackTypeMacro(cmCPackRPMGenerator, cmCPackGenericGenerator);

  /**
   * Construct generator
   */
  cmCPackRPMGenerator();
  virtual ~cmCPackRPMGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".rpm"; }
  virtual const char* GetInstallPrefix() { return "/usr"; }

};

#endif
