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

#ifndef cmCPackCygwinSourceGenerator_h
#define cmCPackCygwinSourceGenerator_h

#include "cmCPackTarBZip2Generator.h"

/** \class cmCPackCygwinSourceGenerator
 * \brief A generator for cygwin source files
 */
class cmCPackCygwinSourceGenerator : public cmCPackTarBZip2Generator
{
public:
  cmCPackTypeMacro(cmCPackCygwinSourceGenerator, cmCPackTGZGenerator);

  /**
   * Construct generator
   */
  cmCPackCygwinSourceGenerator();
  virtual ~cmCPackCygwinSourceGenerator();
protected:
  const char* GetInstallPrefix();
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension();
  std::string InstallPrefix;
  std::string OutputExtension;
};

#endif
