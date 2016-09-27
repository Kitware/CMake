/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWIXSourceWriter.h"

#include <CPack/cmCPackGenerator.h>

#include <cmUuid.h>

#include <windows.h>

cmWIXSourceWriter::cmWIXSourceWriter(cmCPackLog* logger,
                                     std::string const& filename,
                                     GuidType componentGuidType,
                                     RootElementType rootElementType)
  : Logger(logger)
  , File(filename.c_str())
  , State(DEFAULT)
  , SourceFilename(filename)
  , ComponentGuidType(componentGuidType)
{
  WriteXMLDeclaration();

  if (rootElementType == INCLUDE_ELEMENT_ROOT) {
    BeginElement("Include");
  } else {
    BeginElement("Wix");
  }

  AddAttribute("xmlns", "http://schemas.microsoft.com/wix/2006/wi");
}

cmWIXSourceWriter::~cmWIXSourceWriter()
{
  if (Elements.size() > 1) {
    cmCPackLogger(cmCPackLog::LOG_ERROR, Elements.size() - 1
                    << " WiX elements were still open when closing '"
                    << SourceFilename << "'" << std::endl);
    return;
  }

  EndElement(Elements.back());
}

void cmWIXSourceWriter::BeginElement(std::string const& name)
{
  if (State == BEGIN) {
    File << ">";
  }

  File << "\n";
  Indent(Elements.size());
  File << "<" << name;

  Elements.push_back(name);
  State = BEGIN;
}

void cmWIXSourceWriter::EndElement(std::string const& name)
{
  if (Elements.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "can not end WiX element with no open elements in '"
                    << SourceFilename << "'" << std::endl);
    return;
  }

  if (Elements.back() != name) {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "WiX element <"
                    << Elements.back() << "> can not be closed by </" << name
                    << "> in '" << SourceFilename << "'" << std::endl);
    return;
  }

  if (State == DEFAULT) {
    File << "\n";
    Indent(Elements.size() - 1);
    File << "</" << Elements.back() << ">";
  } else {
    File << "/>";
  }

  Elements.pop_back();
  State = DEFAULT;
}

void cmWIXSourceWriter::AddTextNode(std::string const& text)
{
  if (State == BEGIN) {
    File << ">";
  }

  if (Elements.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "can not add text without open WiX element in '"
                    << SourceFilename << "'" << std::endl);
    return;
  }

  File << this->EscapeAttributeValue(text);
  State = DEFAULT;
}

void cmWIXSourceWriter::AddProcessingInstruction(std::string const& target,
                                                 std::string const& content)
{
  if (State == BEGIN) {
    File << ">";
  }

  File << "\n";
  Indent(Elements.size());
  File << "<?" << target << " " << content << "?>";

  State = DEFAULT;
}

void cmWIXSourceWriter::AddAttribute(std::string const& key,
                                     std::string const& value)
{
  std::string utf8 = CMakeEncodingToUtf8(value);

  File << " " << key << "=\"" << EscapeAttributeValue(utf8) << '"';
}

void cmWIXSourceWriter::AddAttributeUnlessEmpty(std::string const& key,
                                                std::string const& value)
{
  if (!value.empty()) {
    AddAttribute(key, value);
  }
}

std::string cmWIXSourceWriter::CMakeEncodingToUtf8(std::string const& value)
{
#ifdef CMAKE_ENCODING_UTF8
  return value;
#else
  if (value.empty()) {
    return std::string();
  }

  int characterCount = MultiByteToWideChar(
    CP_ACP, 0, value.c_str(), static_cast<int>(value.size()), 0, 0);

  if (characterCount == 0) {
    return std::string();
  }

  std::vector<wchar_t> utf16(characterCount);

  MultiByteToWideChar(CP_ACP, 0, value.c_str(), static_cast<int>(value.size()),
                      &utf16[0], static_cast<int>(utf16.size()));

  int utf8ByteCount = WideCharToMultiByte(
    CP_UTF8, 0, &utf16[0], static_cast<int>(utf16.size()), 0, 0, 0, 0);

  if (utf8ByteCount == 0) {
    return std::string();
  }

  std::vector<char> utf8(utf8ByteCount);

  WideCharToMultiByte(CP_UTF8, 0, &utf16[0], static_cast<int>(utf16.size()),
                      &utf8[0], static_cast<int>(utf8.size()), 0, 0);

  return std::string(&utf8[0], utf8.size());
#endif
}

std::string cmWIXSourceWriter::CreateGuidFromComponentId(
  std::string const& componentId)
{
  std::string guid = "*";
  if (this->ComponentGuidType == CMAKE_GENERATED_GUID) {
    std::string md5 = cmSystemTools::ComputeStringMD5(componentId);
    cmUuid uuid;
    std::vector<unsigned char> ns;
    guid = uuid.FromMd5(ns, md5);
  }
  return guid;
}

void cmWIXSourceWriter::WriteXMLDeclaration()
{
  File << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
}

void cmWIXSourceWriter::Indent(size_t count)
{
  for (size_t i = 0; i < count; ++i) {
    File << "    ";
  }
}

std::string cmWIXSourceWriter::EscapeAttributeValue(std::string const& value)
{
  std::string result;
  result.reserve(value.size());

  char c = 0;
  for (size_t i = 0; i < value.size(); ++i) {
    c = value[i];
    switch (c) {
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '&':
        result += "&amp;";
        break;
      case '"':
        result += "&quot;";
        break;
      default:
        result += c;
        break;
    }
  }

  return result;
}
