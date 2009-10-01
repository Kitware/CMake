/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackZIPGenerator_h
#define cmCPackZIPGenerator_h

#include "cmCPackGenerator.h"

class cmCPackZIPGeneratorForward;

/** \class cmCPackZIPGenerator
 * \brief A generator for ZIP files
 */
class cmCPackZIPGenerator : public cmCPackGenerator
{
public:
  friend class cmCPackZIPGeneratorForward;
  cmCPackTypeMacro(cmCPackZIPGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackZIPGenerator();
  virtual ~cmCPackZIPGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".zip"; }

  int ZipStyle;
};

#endif
