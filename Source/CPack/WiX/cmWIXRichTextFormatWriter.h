/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXRichTextFormatWriter_h
#define cmWIXRichTextFormatWriter_h

#include <fstream>

/** \class cmWIXRichtTextFormatWriter
 * \brief Helper class to generate Rich Text Format (RTF) documents
 * from plain text (e.g. for license and welcome text)
 */
class cmWIXRichTextFormatWriter
{
public:
  cmWIXRichTextFormatWriter(const std::string& filename);
  ~cmWIXRichTextFormatWriter();

  void AddText(const std::string& text);

private:
  void WriteHeader();
  void WriteFontTable();
  void WriteGenerator();

  void WriteDocumentPrefix();

  void ControlWord(const std::string& keyword);
  void NewControlWord(const std::string& keyword);

  void StartGroup();
  void EndGroup();

  std::ofstream file;
};

#endif
