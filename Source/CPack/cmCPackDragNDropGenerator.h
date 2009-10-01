/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackDragNDropGenerator_h
#define cmCPackDragNDropGenerator_h

#include "cmCPackGenerator.h"

/** \class cmCPackDragNDropGenerator
 * \brief A generator for OSX drag-n-drop installs
 */
class cmCPackDragNDropGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackDragNDropGenerator, cmCPackGenerator);

  cmCPackDragNDropGenerator();
  virtual ~cmCPackDragNDropGenerator();

protected:
  virtual int InitializeInternal();
  virtual const char* GetOutputExtension();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);

  bool CopyFile(cmOStringStream& source, cmOStringStream& target);
  bool RunCommand(cmOStringStream& command, std::string* output = 0);

  virtual int CreateDMG(const std::string& installdir,
    const std::string& outdmg);

  std::string InstallPrefix;
};

#endif
