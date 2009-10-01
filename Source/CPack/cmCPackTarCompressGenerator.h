/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTarCompressGenerator_h
#define cmCPackTarCompressGenerator_h

#include "cmCPackTGZGenerator.h"

/** \class cmCPackTarCompressGenerator
 * \brief A generator for TarCompress files
 */
class cmCPackTarCompressGenerator : public cmCPackTGZGenerator
{
public:
  friend class cmCPackTarCompressGeneratorForward;
  cmCPackTypeMacro(cmCPackTarCompressGenerator, cmCPackTGZGenerator);

  /**
   * Construct generator
   */
  cmCPackTarCompressGenerator();
  virtual ~cmCPackTarCompressGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".tar.Z"; }

  int RenameFile(const char* oldname, const char* newname);
  int GenerateHeader(std::ostream* os);
};

#endif
