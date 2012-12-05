/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXSourceWriter.h"

#include <CPack/cmCPackGenerator.h>

#include <windows.h>

cmWIXSourceWriter::cmWIXSourceWriter(cmCPackLog* logger,
  const std::string& filename,
  bool isIncludeFile):
    Logger(logger),
    file(filename.c_str()),
    state(DEFAULT)
{
  WriteXMLDeclaration();

  if(isIncludeFile)
    {
    BeginElement("Include");
    }
  else
    {
    BeginElement("Wix");
    }

  AddAttribute("xmlns", "http://schemas.microsoft.com/wix/2006/wi");
}

cmWIXSourceWriter::~cmWIXSourceWriter()
{
  while(elements.size())
    {
    EndElement();
    }
}

void cmWIXSourceWriter::BeginElement(const std::string& name)
{
  if(state == BEGIN)
    {
    file << ">";
    }

  file << "\n";
  Indent(elements.size());
  file << "<" << name;

  elements.push_back(name);
  state = BEGIN;
}

void cmWIXSourceWriter::EndElement()
{
  if(elements.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "can not end WiX element with no open elements" << std::endl);
    return;
    }

  if(state == DEFAULT)
    {
    file << "\n";
    Indent(elements.size()-1);
    file << "</" << elements.back() << ">";
    }
  else
    {
    file << "/>";
    }

  elements.pop_back();
  state = DEFAULT;
}

void cmWIXSourceWriter::AddProcessingInstruction(
  const std::string& target, const std::string& content)
{
  if(state == BEGIN)
    {
    file << ">";
    }

  file << "\n";
  Indent(elements.size());
  file << "<?" << target << " " << content << "?>";

  state = DEFAULT;
}

void cmWIXSourceWriter::AddAttribute(
  const std::string& key, const std::string& value)
{
  std::string utf8 = WindowsCodepageToUtf8(value);

  file << " " << key << "=\"" << EscapeAttributeValue(utf8) << '"';
}

std::string cmWIXSourceWriter::WindowsCodepageToUtf8(const std::string& value)
{
  if(value.empty())
    {
    return std::string();
    }

  int characterCount = MultiByteToWideChar(
    CP_ACP, 0, value.c_str(), static_cast<int>(value.size()), 0, 0);

  if(characterCount == 0)
    {
    return std::string();
    }

  std::vector<wchar_t> utf16(characterCount);

  MultiByteToWideChar(
    CP_ACP, 0, value.c_str(), static_cast<int>(value.size()),
    &utf16[0], static_cast<int>(utf16.size()));

  int utf8ByteCount = WideCharToMultiByte(
    CP_UTF8, 0, &utf16[0], static_cast<int>(utf16.size()), 0, 0, 0, 0);

  if(utf8ByteCount == 0)
    {
    return std::string();
    }

  std::vector<char> utf8(utf8ByteCount);

  WideCharToMultiByte(CP_UTF8, 0, &utf16[0], static_cast<int>(utf16.size()),
    &utf8[0], static_cast<int>(utf8.size()), 0, 0);

  return std::string(&utf8[0], utf8.size());
}


void cmWIXSourceWriter::WriteXMLDeclaration()
{
  file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
}

void cmWIXSourceWriter::Indent(size_t count)
{
  for(size_t i = 0; i < count; ++i)
    {
    file << "    ";
    }
}

std::string cmWIXSourceWriter::EscapeAttributeValue(
  const std::string& value)
{
  std::string result;
  result.reserve(value.size());

  char c = 0;
  for(size_t i = 0 ; i < value.size(); ++i)
    {
    c = value[i];
    switch(c)
      {
    case '<':
      result += "&lt;";
      break;
    case '&':
      result +="&amp;";
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
