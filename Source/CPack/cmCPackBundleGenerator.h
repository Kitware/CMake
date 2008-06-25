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

#ifndef cmCPackBundleGenerator_h
#define cmCPackBundleGenerator_h

#include "cmCPackGenerator.h"

/** \class cmCPackBundleGenerator
 * \brief A generator for OSX bundles
 *
 * Based on Gimp.app
 */
class cmCPackBundleGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackBundleGenerator, cmCPackGenerator);

  cmCPackBundleGenerator();
  virtual ~cmCPackBundleGenerator();

protected:
  virtual int InitializeInternal();
  virtual const char* GetOutputExtension();
  virtual const char* GetPackagingInstallPrefix();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);

  bool CopyFile(cmOStringStream& source, cmOStringStream& target);
  bool RunCommand(cmOStringStream& command);

  std::string InstallPrefix;
};

#endif
