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


#include "cmCPackGenericGenerator.h"
#include "CPack/cmCPackConfigure.h" // for ssize_t

/** \class cmCPackTGZGenerator
 * \brief A generator for TGZ files
 *
 * http://people.freebsd.org/~kientzle/libarchive/
 */
class cmCPackTGZGenerator : public cmCPackGenericGenerator
{
public:
  cmCPackTypeMacro(cmCPackTGZGenerator, cmCPackGenericGenerator);

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
  cmCPackTGZGenerator();
  virtual ~cmCPackTGZGenerator();

protected:
  static int TGZ_Open(void *client_data, const char* name, int oflags, mode_t mode);
  static ssize_t TGZ_Write(void *client_data, void *buff, size_t n);
  static int TGZ_Close(void *client_data);

  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return "tar.gz"; }
};

#endif


