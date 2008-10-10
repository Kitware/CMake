/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  os << ".fi\n";
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

