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

#include <CPack/cmCPackLog.h>

#include <cmsys/FStream.hxx>

#include <string>
#include <vector>

/** \class cmWIXSourceWriter
 * \brief Helper class to generate XML WiX source files
 */
class cmWIXSourceWriter
{
public:
  enum GuidType
  {
    WIX_GENERATED_GUID,
    CMAKE_GENERATED_GUID
  };

  enum RootElementType
  {
    WIX_ELEMENT_ROOT,
    INCLUDE_ELEMENT_ROOT
  };

  cmWIXSourceWriter(cmCPackLog* logger, std::string const& filename,
                    GuidType componentGuidType,
                    RootElementType rootElementType = WIX_ELEMENT_ROOT);

  ~cmWIXSourceWriter();

  void BeginElement(std::string const& name);

  void EndElement(std::string const& name);

  void AddTextNode(std::string const& text);

  void AddProcessingInstruction(std::string const& target,
                                std::string const& content);

  void AddAttribute(std::string const& key, std::string const& value);

  void AddAttributeUnlessEmpty(std::string const& key,
                               std::string const& value);

  std::string CreateGuidFromComponentId(std::string const& componentId);

  static std::string CMakeEncodingToUtf8(std::string const& value);

protected:
  cmCPackLog* Logger;

private:
  enum State
  {
    DEFAULT,
    BEGIN
  };

  void WriteXMLDeclaration();

  void Indent(size_t count);

  static std::string EscapeAttributeValue(std::string const& value);

  cmsys::ofstream File;

  State State;

  std::vector<std::string> Elements;

  std::string SourceFilename;

  GuidType ComponentGuidType;
};

#endif
