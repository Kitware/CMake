/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXSourceWriter_h
#define cmWIXSourceWriter_h

#include <vector>
#include <string>
#include <fstream>

#include <CPack/cmCPackLog.h>

/** \class cmWIXSourceWriter
 * \brief Helper class to generate XML WiX source files
 */
class cmWIXSourceWriter
{
public:
  cmWIXSourceWriter(cmCPackLog* logger,
    const std::string& filename, bool isIncludeFile = false);

  ~cmWIXSourceWriter();

  void BeginElement(const std::string& name);

  void EndElement();

  void AddProcessingInstruction(
    const std::string& target, const std::string& content);

  void AddAttribute(
    const std::string& key, const std::string& value);

  static std::string WindowsCodepageToUtf8(const std::string& value);

private:
  enum State
  {
    DEFAULT,
    BEGIN
  };

  void WriteXMLDeclaration();

  void Indent(size_t count);

  static std::string EscapeAttributeValue(const std::string& value);

  cmCPackLog* Logger;

  std::ofstream file;

  State state;

  std::vector<std::string> elements;
};

#endif
