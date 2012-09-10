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
#include <algorithm>
#include <ctype.h> // for isalnum

static int cmIsAlnum(int c)
{
  return isalnum(c);
}

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

//----------------------------------------------------------------------------
cmDocumentationFormatterDocbook::cmDocumentationFormatterDocbook()
:cmDocumentationFormatter()
{
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  os << "<sect1 id=\"";
  this->PrintId(os, 0, name);
  os << "\">\n<title>" << name << "</title>\n";

  std::string prefix = this->ComputeSectionLinkPrefix(name);
  const std::vector<cmDocumentationEntry> &entries = section.GetEntries();

  bool hasSubSections = false;
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      hasSubSections = true;
      break;
      }
    }

  bool inAbstract = false;
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      if(inAbstract)
        {
        os << "</abstract>\n";
        inAbstract = false;
        }
      os << "<sect2 id=\"";
      this->PrintId(os, prefix.c_str(), op->Name);
      os << "\">\n<title>";
      cmDocumentationPrintDocbookEscapes(os, op->Name.c_str());
      os << "</title>\n";
      if(op->Full.size())
        {
        os << "<abstract>\n<para>";
        cmDocumentationPrintDocbookEscapes(os, op->Brief.c_str());
        os << "</para>\n</abstract>\n";
        this->PrintFormatted(os, op->Full.c_str());
        }
      else
        {
        this->PrintFormatted(os, op->Brief.c_str());
        }
      os << "</sect2>\n";
      }
    else
      {
      if(hasSubSections && op == entries.begin())
        {
        os << "<abstract>\n";
        inAbstract = true;
        }
      this->PrintFormatted(os, op->Brief.c_str());
      }
    }

  // empty sections are not allowed in docbook.
  if(entries.empty())
    {
    os << "<para/>\n";
    }

  os << "</sect1>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook
::PrintPreformatted(std::ostream& os, const char* text)
{
  os << "<para>\n<programlisting>";
  cmDocumentationPrintDocbookEscapes(os, text);
  os << "</programlisting>\n</para>\n";
}

void cmDocumentationFormatterDocbook
::PrintParagraph(std::ostream& os, const char* text)
{
  os << "<para>";
  cmDocumentationPrintDocbookEscapes(os, text);
  os << "</para>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook
::PrintHeader(const char* docname, const char* appname, std::ostream& os)
{
  this->Docname = docname;

  // this one is used to ensure that we don't create multiple link targets
  // with the same name. We can clear it here since we are at the
  // start of a document here.
  this->EmittedLinkIds.clear();

  os << "<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE article PUBLIC \"-//OASIS//DTD DocBook V4.5//EN\" "
        "\"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd\" [\n"
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

//----------------------------------------------------------------------------
void cmDocumentationFormatterDocbook
::PrintId(std::ostream& os, const char* prefix, std::string id)
{
  std::replace_if(id.begin(), id.end(),
                  std::not1(std::ptr_fun(cmIsAlnum)), '_');
  if(prefix)
    {
    id = std::string(prefix) + "." + id;
    }
  os << this->Docname << '.' << id;

  // make sure that each id exists only once.  Since it seems
  // not easily possible to determine which link refers to which id,
  // we have at least to make sure that the duplicated id's get a
  // different name (by appending an increasing number), Alex
  if (this->EmittedLinkIds.find(id) == this->EmittedLinkIds.end())
    {
    this->EmittedLinkIds.insert(id);
    }
  else
    {
    static unsigned int i=0;
    os << i++;
    }
}
