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

#ifndef cmCPackNSISGenerator_h
#define cmCPackNSISGenerator_h


#include "cmCPackGenericGenerator.h"
#include "CPack/cmCPackConfigure.h" // for ssize_t

/** \class cmCPackNSISGenerator
 * \brief A generator for NSIS files
 *
 * http://people.freebsd.org/~kientzle/libarchive/
 */
class cmCPackNSISGenerator : public cmCPackGenericGenerator
{
public:
  cmCPackTypeMacro(cmCPackNSISGenerator, cmCPackGenericGenerator);
  /**
   * Do the actual processing. Subclass has to override it.
   * Return < 0 if error.
   */
  virtual int ProcessGenerator();

  /**
   * Initialize generator
   */
  virtual int Initialize(const char* name);

  /**
   * Construct generator
   */
  cmCPackNSISGenerator();
  virtual ~cmCPackNSISGenerator();

protected:
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return "exe"; }
  virtual const char* GetOutputPostfix() { return "win32"; }
};

#endif


