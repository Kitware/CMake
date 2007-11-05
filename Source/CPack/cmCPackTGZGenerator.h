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

#ifndef cmCPackTGZGenerator_h
#define cmCPackTGZGenerator_h

#include "cmCPackGenerator.h"

class cmCPackTGZGeneratorForward;

/** \class cmCPackTGZGenerator
 * \brief A generator for TGZ files
 *
 * http://people.freebsd.org/~kientzle/libarchive/
 */
class cmCPackTGZGenerator : public cmCPackGenerator
{
public:
  friend class cmCPackTGZGeneratorForward;
  cmCPackTypeMacro(cmCPackTGZGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackTGZGenerator();
  virtual ~cmCPackTGZGenerator();

protected:
  virtual int InitializeInternal();
  virtual int GenerateHeader(std::ostream* os);
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".tar.gz"; }

  bool Compress;
};

#endif
