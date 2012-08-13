/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationFormatterHTML.h"
#include "cmDocumentationSection.h"
#include "cmVersion.h"
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
  switch (c)
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
    case '\n':
      os << "<br />";
      break;
    default:
      os << c;
  }
}

//----------------------------------------------------------------------------
bool cmDocumentationHTMLIsIdChar(char c)
{
  // From the HTML specification:
  //   ID and NAME tokens must begin with a letter ([A-Za-z]) and may
  //   be followed by any number of letters, digits ([0-9]), hyphens
  //   ("-"), underscores ("_"), colons (":"), and periods (".").
  return ((c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') ||
          c == '-' || c == '_' || c == ':' || c == '.');
}

//----------------------------------------------------------------------------
void cmDocumentationPrintHTMLId(std::ostream& os, const char* begin)
{
  for(const char* c = begin; *c; ++c)
    {
    if(cmDocumentationHTMLIsIdChar(*c))
      {
      os << *c;
      }
    }
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

void cmDocumentationFormatterHTML
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  std::string prefix = this->ComputeSectionLinkPrefix(name);

  const std::vector<cmDocumentationEntry> &entries =
    section.GetEntries();

  // skip the index if the help for only a single item (--help-command,
  // --help-policy, --help-property, --help-module) is printed
  bool isSingleItemHelp = ((name!=0) && (strcmp(name, "SingleItem")==0));

  if (!isSingleItemHelp)
    {
    if (name)
      {
      os << "<h2><a name=\"section_";
      cmDocumentationPrintHTMLId(os, name);
      os << "\"></a>" << name << "</h2>\n";
      }

    // Is a list needed?
    for(std::vector<cmDocumentationEntry>::const_iterator op
         = entries.begin(); op != entries.end(); ++ op )
      {
      if (op->Name.size())
        {
        os << "<ul>\n";
        for(;op != entries.end() && op->Name.size(); ++op)
          {
          if(op->Name.size())
            {
            os << "    <li><a href=\"#" << prefix << ":";
            cmDocumentationPrintHTMLId(os, op->Name.c_str());
            os << "\"><b><code>";
            this->PrintHTMLEscapes(os, op->Name.c_str());
            os << "</code></b></a></li>\n";
            }
          }
        os << "</ul>\n" ;
        break; // Skip outer loop termination test
        }
      }
    }

  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end();)
    {
    if(op->Name.size())
      {
      os << "<ul>\n";
      for(;op != entries.end() && op->Name.size(); ++op)
        {
        os << "  <li>\n";
        if(op->Name.size())
          {
          os << "    <a name=\"" << prefix << ":";
          cmDocumentationPrintHTMLId(os, op->Name.c_str());
          os << "\"></a><b><code>";
          this->PrintHTMLEscapes(os, op->Name.c_str());
          os << "</code></b>: ";
          }
        this->PrintHTMLEscapes(os, op->Brief.c_str());
        if(op->Full.size())
          {
          os << "<br />\n    ";
          this->PrintFormatted(os, op->Full.c_str());
          }
        os << "\n";
        os << "  </li>\n";
        }
      os << "</ul>\n";
      }
    else
      {
      this->PrintFormatted(os, op->Brief.c_str());
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
  os << "</p>\n";
}

//----------------------------------------------------------------------------
void cmDocumentationFormatterHTML::PrintHeader(const char* docname,
                                               const char* appname,
                                               std::ostream& os)
{
  os << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
     << " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
  os << "<html xmlns=\"http://www.w3.org/1999/xhtml\""
     << " xml:lang=\"en\" lang=\"en\">\n";
  os << "<head><meta http-equiv=\"Content-Type\" "
     << "content=\"text/html;charset=utf-8\" /><title>";
  os << docname << " - " << appname;
  os << "</title></head><body>\n";
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

void cmDocumentationFormatterHTML
::PrintIndex(std::ostream& os,
             std::vector<const cmDocumentationSection *>& sections)
{
  // skip the index if only the help for a single item is printed
  if ((sections.size() == 1)
       && (sections[0]->GetName(this->GetForm()) != 0 )
       && (std::string(sections[0]->GetName(this->GetForm())) == "SingleItem"))
    {
    return;
    }

  os << "<h2><a name=\"section_Index\"></a>Master Index "
     << "CMake " << cmVersion::GetCMakeVersion()
     << "</h2>\n";

  if (!sections.empty())
    {
    os << "<ul>\n";
    for(unsigned int i=0; i < sections.size(); ++i)
      {
      std::string name = sections[i]->GetName((this->GetForm()));
      os << "  <li><a href=\"#section_";
      cmDocumentationPrintHTMLId(os, name.c_str());
      os << "\"><b>" << name << "</b></a></li>\n";
      }
    os << "</ul>\n";
    }
}
