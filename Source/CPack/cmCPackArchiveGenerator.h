/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackArchiveGenerator_h
#define cmCPackArchiveGenerator_h

#include "cmArchiveWrite.h"
#include "cmCPackGenerator.h"


/** \class cmCPackArchiveGenerator
 * \brief A generator base for libarchive generation.
 * The generator itself uses the libarchive wrapper
 * \ref cmArchiveWrite.
 *
 */
class cmCPackArchiveGenerator : public cmCPackGenerator
{
public:
  cmTypeMacro(cmCPackArchiveGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackArchiveGenerator(cmArchiveWrite::Compress, cmArchiveWrite::Type);
  virtual ~cmCPackArchiveGenerator();
  // Used to add a header to the archive 
  virtual int GenerateHeader(std::ostream* os);

protected:
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension() = 0;
  cmArchiveWrite::Compress Compress;
  cmArchiveWrite::Type Archive;
};

#endif
