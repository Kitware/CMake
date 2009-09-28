/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTarBZip2Generator_h
#define cmCPackTarBZip2Generator_h

#include "cmCPackTGZGenerator.h"

/** \class cmCPackTarBZip2Generator
 * \brief A generator for TarBZip2 files
 */
class cmCPackTarBZip2Generator : public cmCPackTGZGenerator
{
public:
  friend class cmCPackTarBZip2GeneratorForward;
  cmCPackTypeMacro(cmCPackTarBZip2Generator, cmCPackTGZGenerator);

  /**
   * Construct generator
   */
  cmCPackTarBZip2Generator();
  virtual ~cmCPackTarBZip2Generator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".tar.bz2"; }
  int BZip2File(const char* filename);
  int RenameFile(const char* oldname, const char* newname);
};

#endif
