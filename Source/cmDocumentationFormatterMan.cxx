/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmDocumentationFormatterMan.h"
#include "cmDocumentationSection.h"

#include "cmSystemTools.h"
#include "cmVersion.h"


cmDocumentationFormatterMan::cmDocumentationFormatterMan()
:cmDocumentationFormatter()
{
}

void cmDocumentationFormatterMan
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  if(name)
    {
    os << ".SH " << name << "\n";
    }

  const std::vector<cmDocumentationEntry> &entries = 
    section.GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin(); 
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      os << ".TP\n"
         << ".B " << (op->Name.size()?op->Name.c_str():"*") << "\n";
      this->PrintFormatted(os, op->Brief.c_str());
      this->PrintFormatted(os, op->Full.c_str());
      }
    else
      {
      os << ".PP\n";
      this->PrintFormatted(os, op->Brief.c_str());
      }
    }
}

void cmDocumentationFormatterMan::EscapeText(std::string& man_text)
{
  cmSystemTools::ReplaceString(man_text, "\\", "\\\\");
  cmSystemTools::ReplaceString(man_text, "-", "\\-");
}

void cmDocumentationFormatterMan::PrintPreformatted(std::ostream& os, 
                                                    const char* text)
{
  std::string man_text = text;
  this->EscapeText(man_text);
  os << ".nf\n" << man_text;
  if (*text && man_text.at(man_text.length()-1) != '\n')
      os << "\n";
  os << ".fi\n\n";
}

void cmDocumentationFormatterMan::PrintParagraph(std::ostream& os, 
                                                 const char* text)
{
  std::string man_text = text;
  this->EscapeText(man_text);
  os << man_text << "\n\n";
}


//----------------------------------------------------------------------------
void cmDocumentationFormatterMan::PrintHeader(const char* docname,
                                              const char* appname,
                                              std::ostream& os)
{
  std::string s_docname(docname), s_appname(appname);

  this->EscapeText(s_docname);
  this->EscapeText(s_appname);
  os << ".TH " << s_docname << " 1 \""
    << cmSystemTools::GetCurrentDateTime("%B %d, %Y").c_str()
    << "\" \"" << s_appname
    << " " << cmVersion::GetCMakeVersion()
    << "\"\n";
}

