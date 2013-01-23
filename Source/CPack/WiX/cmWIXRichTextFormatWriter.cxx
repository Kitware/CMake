/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXRichTextFormatWriter.h"

#include <cmVersion.h>

cmWIXRichTextFormatWriter::cmWIXRichTextFormatWriter(
  const std::string& filename):
    file(filename.c_str(), std::ios::binary)
{
  StartGroup();
  WriteHeader();
  WriteDocumentPrefix();
}

cmWIXRichTextFormatWriter::~cmWIXRichTextFormatWriter()
{
  EndGroup();

  /* I haven't seen this in the RTF spec but
   *  wordpad terminates its RTF like this */
  file << "\r\n";
  file.put(0);
}

void cmWIXRichTextFormatWriter::AddText(const std::string& text)
{
  typedef unsigned char rtf_byte_t;

  for(size_t i = 0; i < text.size(); ++i)
    {
    rtf_byte_t c = rtf_byte_t(text[i]);

    switch(c)
      {
    case '\\':
      file << "\\\\";
      break;
    case '{':
      file << "\\{";
      break;
    case '}':
      file << "\\}";
      break;
    case '\n':
      file << "\\par\r\n";
      break;
    case '\r':
      continue;
    default:
        {
        if(c <= 0x7F)
          {
          file << c;
          }
        else
          {
          file << "[NON-ASCII-" << int(c) << "]";
          }
        }
      break;
      }
    }
}

void cmWIXRichTextFormatWriter::WriteHeader()
{
  ControlWord("rtf1");
  ControlWord("ansi");
  ControlWord("ansicpg1252");
  ControlWord("deff0");
  ControlWord("deflang1031");

  WriteFontTable();
  WriteGenerator();
}

void cmWIXRichTextFormatWriter::WriteFontTable()
{
  StartGroup();
  ControlWord("fonttbl");

  StartGroup();
  ControlWord("f0");
  ControlWord("fswiss");
  ControlWord("fcharset0 Arial;");
  EndGroup();

  EndGroup();
}

void cmWIXRichTextFormatWriter::WriteGenerator()
{
  StartGroup();
  NewControlWord("generator");
  file << " CPack WiX Generator (" << cmVersion::GetCMakeVersion() << ");";
  EndGroup();
}

void cmWIXRichTextFormatWriter::WriteDocumentPrefix()
{
  ControlWord("viewkind4");
  ControlWord("uc1");
  ControlWord("pard");
  ControlWord("f0");
  ControlWord("fs20");
}

void cmWIXRichTextFormatWriter::ControlWord(const std::string& keyword)
{
  file << "\\" << keyword;
}

void cmWIXRichTextFormatWriter::NewControlWord(const std::string& keyword)
{
  file << "\\*\\" << keyword;
}

void cmWIXRichTextFormatWriter::StartGroup()
{
  file.put('{');
}

void cmWIXRichTextFormatWriter::EndGroup()
{
  file.put('}');
}
