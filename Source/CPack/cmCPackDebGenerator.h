/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackDebGenerator_h
#define cmCPackDebGenerator_h


#include "cmCPackGenerator.h"

/** \class cmCPackDebGenerator
 * \brief A generator for Debian packages
 *
 */
class cmCPackDebGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackDebGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackDebGenerator();
  virtual ~cmCPackDebGenerator();

protected:
  virtual int InitializeInternal();
  virtual int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".deb"; }

};

#endif
