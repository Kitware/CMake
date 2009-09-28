/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationFormatterDocbook.h"
#include "cmDocumentationSection.h"
//----------------------------------------------------------------------------

// this function is a copy of the one in the HTML formatter
// the three functions below are slightly modified copies
static bool cmDocumentationIsHyperlinkCharDocbook(char c)
{
  // This is not a complete list but works for CMake documentation.
  return ((c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') ||
          c == '-' || c == '.' || c == '/' || c == '~' || c == '@' ||
          c == ':' || c == '_' || c == '&' || c == '?' || c == '=');
}

//----------------------------------------------------------------------------
static void cmDocumentationPrintDocbookChar(std::ostream& os, char c)
{
  // Use an escape sequence if necessary.
  switch(c)
  {
    case '<':
      os << "&lt;";
      break;
    case '>': 
      os << "&gt;";
      break;
    case '&':
      os << "&amp;";
      break;
    default:
      os << c;
  }
}

//----------------------------------------------------------------------------
const char* cmDocumentationPrintDocbookLink(std::ostream& os,const char* begin)
{
  // Look for the end of the link.
  const char* end = begin;
  while(cmDocumentationIsHyperlinkCharDocbook(*end))
    {
    ++end;
    }

  // Print the hyperlink itself.
  os << "<ulink url=\"";
  for(const char* c = begin; c != end; ++c)
    {
    cmDocumentationPrintDocbookChar(os, *c);
    }
  os << "\" />";

  return end;
}

//----------------------------------------------------------------------------
void cmDocumentationPrintDocbookEscapes(std::ostream& os, const char* text)
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
        p = cmDocumentationPrintDocbookLink(os, p);
        found_hyperlink = true;
        }
      }

    // Print other characters normally.
    if(!found_hyperlink)
      {
      cmDocumentationPrintDocbookChar(os, *p++);
      }
    }
}


cmDocumentationFormatterDocbook::cmDocumentationFormatterDocbook()
:cmDocumentationFormatter()
{
}

void cmDocumentationFormatterDocbook
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  if(name)
    {
    std::string id = "section_";
    id += name;
    if (this->EmittedLinkIds.find(id) == this->EmittedLinkIds.end())
      {
      this->EmittedLinkIds.insert(id);
      os << "<sect1 id=\"section_" << name << "\">\n"
            "<title>\n" << name << "</title>\n";
      }
    else
      {
      static unsigned int i=0;
      i++;
      os << "<sect1 id=\"section_" << name << i << "\">\n"
            "<title>\n" << name << "</title>\n";
      }
    }

  std::string prefix = this->ComputeSectionLinkPrefix(name);

  const std::vector<cmDocumentationEntry> &entries = 
    section.GetEntries();

  if (!entries.empty())
    {
    os << "<itemizedlist>\n";
    for(std::vector<cmDocumentationEntry>::const_iterator op 
          = entries.begin(); op != entries.end(); ++ op )
      {
      if(op->Name.size())
        {
        os << "    <listitem><link linkend=\"" << prefix << "_";
        cmDocumentationPrintDocbookEscapes(os, op->Name.c_str());
        os << "\"><emphasis><literal>";
        cmDocumentationPrintDocbookEscapes(os, op->Name.c_str());
        os << "</literal></emphasis></link></listitem>\n";
        }
      }
    os << "</itemizedlist>\n" ;
    }

  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin(); 
      op != entries.end();)
    {
    if(op->Name.size())
      {
      for(;op != entries.end() && op->Name.size(); ++op)
        {
        if(op->Name.size())
          {
          os << "    <para id=\"" << prefix << "_";
          cmDocumentationPrintDocbookEscapes(os, op->Name.c_str());

          // make sure that each id exists only once.  Since it seems
          // not easily possible to determine which link refers to which id, 
          // we have at least to make sure that the duplicated id's get a 
          // different name (by appending an increasing number), Alex
          std::string id = prefix;
          id += "_";
          id += op->Name;
          if (this->EmittedLinkIds.find(id) == this->EmittedLinkIds.end())
            {
            this->EmittedLinkIds.insert(id);
            }
          else
            {
            static unsigned int i=0;
            i++;
            os << i;
            }
          // continue as normal...

          os << "\"><sect2><title>";
          cmDocumentationPrintDocbookEscapes(os, op->Name.c_str());
          os << "</title></sect2> ";
          }
        cmDocumentationPrintDocbookEscapes(os, op->Brief.c_str());
        if(op->Name.size())
          {
          os << "</para>\n";
          }

        if(op->Full.size())
          {
          // a line break seems to be simply a line break with docbook
          os << "\n    ";  
          this->PrintFormatted(os, op->Full.c_str());
          }
        os << "\n";
        }
      }
    else
      {
      this->PrintFormatted(os, op->Brief.c_str());
      os << "\n";
      ++op;
      }
    }
  if(name)
    {
    os << "</sect1>\n";
    }
}

void cmDocumentationFormatterDocbook::PrintPreformatted(std::ostream& os, 
                                                     const char* text)
{
  os << "<literallayout>";
  cmDocumentationPrintDocbookEscapes(os, text);
  os << "</literallayout>\n    ";
}

void cmDocumentationFormatterDocbook::PrintParagraph(std::ostream& os, 
                                                  const char* text)
{
  os << "<para>";
  cmDocumentationPrintDocbookEscapes(os, text);
  os << "</para>";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook::PrintHeader(const char* docname,
                                                  const char* appname,
                                                  std::ostream& os)
{
  // this one is used to ensure that we don't create multiple link targets
  // with the same name. We can clear it here since we are at the 
  // start of a document here.
  this->EmittedLinkIds.clear();

  os << "<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE article PUBLIC \"-//OASIS//DTD DocBook V4.2//EN\" "
        "\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\" [\n"
        "<!ENTITY % addindex \"IGNORE\">\n"
        "<!ENTITY % English \"INCLUDE\"> ]>\n"
        "<article>\n"
        "<articleinfo>\n"
        "<title>" << docname << " - " << appname << "</title>\n"
        "</articleinfo>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook::PrintFooter(std::ostream& os)
{
  os << "</article>\n";
}

