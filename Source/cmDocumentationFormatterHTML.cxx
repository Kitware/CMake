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
#include "cmDocumentationFormatterHTML.h"

//----------------------------------------------------------------------------
static bool cmDocumentationIsHyperlinkChar(char c)
{
  // This is not a complete list but works for CMake documentation.
  return ((c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') ||
          c == '-' || c == '.' || c == '/' || c == '~' || c == '@' ||
          c == ':' || c == '_' || c == '&' || c == '?' || c == '=');
}

//----------------------------------------------------------------------------
static void cmDocumentationPrintHTMLChar(std::ostream& os, char c)
{
  // Use an escape sequence if necessary.
  static cmDocumentationEntry escapes[] =
  {
    {"<", "&lt;", 0},
    {">", "&gt;", 0},
    {"&", "&amp;", 0},
    {"\n", "<br>", 0},
    {0,0,0}
  };
  for(const cmDocumentationEntry* op = escapes; op->name; ++op)
    {
    if(op->name[0] == c)
      {
      os << op->brief;
      return;
      }
    }

  // No escape sequence is needed.
  os << c;
}

//----------------------------------------------------------------------------
const char* cmDocumentationPrintHTMLLink(std::ostream& os, const char* begin)
{
  // Look for the end of the link.
  const char* end = begin;
  while(cmDocumentationIsHyperlinkChar(*end))
    {
    ++end;
    }

  // Print the hyperlink itself.
  os << "<a href=\"";
  for(const char* c = begin; c != end; ++c)
    {
    cmDocumentationPrintHTMLChar(os, *c);
    }
  os << "\">";

  // The name of the hyperlink is the text itself.
  for(const char* c = begin; c != end; ++c)
    {
    cmDocumentationPrintHTMLChar(os, *c);
    }
  os << "</a>";

  // Return the position at which to continue scanning the input
  // string.
  return end;
}


cmDocumentationFormatterHTML::cmDocumentationFormatterHTML()
:cmDocumentationFormatter()
{
}

void cmDocumentationFormatterHTML::PrintSection(std::ostream& os,
                                           const cmDocumentationEntry* section,
                                           const char* name)
{
  if(name)
    {
    os << "<h2>" << name << "</h2>\n";
    }
  if(!section) { return; }
  for(const cmDocumentationEntry* op = section; op->brief;)
    {
    if(op->name)
      {
      os << "<ul>\n";
      for(;op->name;++op)
        {
        os << "  <li>\n";
        if(op->name[0])
          {
          os << "    <b><code>";
          this->PrintHTMLEscapes(os, op->name);
          os << "</code></b>: ";
          }
        this->PrintHTMLEscapes(os, op->brief);
        if(op->full)
          {
          os << "<br>\n    ";
          this->PrintFormatted(os, op->full);
          }
        os << "\n";
        os << "  </li>\n";
        }
      os << "</ul>\n";
      }
    else
      {
      this->PrintFormatted(os, op->brief);
      os << "\n";
      ++op;
      }
    }
}

void cmDocumentationFormatterHTML::PrintPreformatted(std::ostream& os, 
                                                     const char* text)
{
  os << "<pre>";
  this->PrintHTMLEscapes(os, text);
  os << "</pre>\n    ";
}

void cmDocumentationFormatterHTML::PrintParagraph(std::ostream& os, 
                                                  const char* text)
{
  os << "<p>";
  this->PrintHTMLEscapes(os, text);
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterHTML::PrintHeader(const char* /*name*/, 
                                               std::ostream& os)
{
  os << "<html><body>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterHTML::PrintFooter(std::ostream& os)
{
  os << "</body></html>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterHTML::PrintHTMLEscapes(std::ostream& os, 
                                                    const char* text)
{
  // Hyperlink prefixes.
  static const char* hyperlinks[] = {"http://", "ftp://", "mailto:", 0};

  // Print each character.
  for(const char* p = text; *p;)
    {
    // Handle hyperlinks specially to make them active.
    bool found_hyperlink = false;
    for(const char** h = hyperlinks; !found_hyperlink && *h; ++h)
      {
      if(strncmp(p, *h, strlen(*h)) == 0)
        {
        p = cmDocumentationPrintHTMLLink(os, p);
        found_hyperlink = true;
        }
      }

    // Print other characters normally.
    if(!found_hyperlink)
      {
      cmDocumentationPrintHTMLChar(os, *p++);
      }
    }
}
