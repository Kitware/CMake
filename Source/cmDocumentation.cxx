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
#include "cmDocumentation.h"

#include "cmSystemTools.h"

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationStandardOptions[] =
{
  {"--copyright", "Print the CMake copyright and exit.", 0},
  {"--usage", "Print usage information and exit.",
   "Usage describes the basic command line interface and its options."},
  {"--help", "Print full help and exit.",
   "Full help displays most of the documentation provided by the UNIX "
   "man page.  It is provided for use on non-UNIX platforms, but is "
   "also convenient if the man page is not installed."},
  {"--help-html", "Print full help in HTML format.",
   "This option is used by CMake authors to help produce web pages."},
  {"--man", "Print a UNIX man page and exit.",
   "This option is used by CMake authors to generate the UNIX man page."},
  {"--version", "Show program name/version banner and exit.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
const cmDocumentationEntry cmDocumentationCopyright[] =
{
  {0,
   "Copyright (c) 2002 Kitware, Inc., Insight Consortium.\n"
   "All rights reserved.\n", 0},
  {0,
   "Redistribution and use in source and binary forms, with or without "
   "modification, are permitted provided that the following conditions are "
   "met:\n", 0},
  {" * ",
   "Redistributions of source code must retain the above copyright notice, "
   "this list of conditions and the following disclaimer.\n", 0},
  {" * ",
   "Redistributions in binary form must reproduce the above copyright "
   "notice, this list of conditions and the following disclaimer in the "
   "documentation and/or other materials provided with the distribution.\n",
   0},
  {" * ",
   "The names of Kitware, Inc., the Insight Consortium, or the names of "
   "any consortium members, or of any contributors, may not be used to "
   "endorse or promote products derived from this software without "
   "specific prior written permission.\n", 0},
  {" * ",
   "Modified source versions must be plainly marked as such, and must "
   "not be misrepresented as being the original software.\n", 0},
  {0,
   "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "
   "``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT "
   "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR "
   "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR "
   "CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, "
   "EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, "
   "PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR "
   "PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF "
   "LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING "
   "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS "
   "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n", 0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
cmDocumentation::cmDocumentation()
{
  this->Commands = 0;
  this->Description = 0;
  this->Name = 0;
  this->UsageHelp = 0;  
  this->SetOptions(0);
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintManSection(std::ostream& os,
                                      const cmDocumentationEntry* section,
                                      const char* name)
{
  if(!section) { return; }
  os << ".SH " << name << "\n";
  for(const cmDocumentationEntry* op = section; op->brief; ++op)
    {
    if(op->name)
      {
      os << ".TP\n"
         << ".B " << op->name << "\n"
         << op->brief << "\n\n";
      if(op->full)
        {
        this->PrintFull(os, op->full, 0, 0);
        }
      }
    else
      {
      os << ".PP\n"
         << op->brief << "\n";
      }
    }  
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHelpSection(std::ostream& os,
                                       const cmDocumentationEntry* section)
{
  if(!section) { return; }
  for(const cmDocumentationEntry* op = section; op->brief; ++op)
    {
    if(op->name)
      {
      os << "  " << op->name << "\n"
         << "       ";
      this->PrintColumn(os, 70, "       ", op->brief);
      if(op->full)
        {
        os << "\n"
           << "\n"
           << "       ";
        this->PrintColumn(os, 70, "       ", op->full);
        }
      os << "\n";
      }
    else
      {
      this->PrintColumn(os, 77, "", op->brief);
      os << "\n";
      }
    os << "\n";
    }  
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHTMLEscapes(std::ostream& os, const char* text)
{
  static cmDocumentationEntry escapes[] =
  {
    {"<", "&lt;", 0},
    {">", "&gt;", 0},
    {"&", "&amp;", 0},
    {"\n", "<br>", 0},
    {0,0,0}
  };
  for(const char* p = text; *p; ++p)
    {
    bool found = false;
    for(const cmDocumentationEntry* op = escapes; !found && op->name; ++op)
      {
      if(op->name[0] == *p)
        {
        os << op->brief;
        found = true;
        }
      }
    if(!found)
      {
      os << *p;
      }
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHTMLPreformatted(std::ostream& os, const char* text)
{
  os << "<pre>";
  cmDocumentation::PrintHTMLEscapes(os, text);
  os << "</pre>";
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHelpHTMLSection(std::ostream& os,
                                           const cmDocumentationEntry* section,
                                           const char* header)
{
  if(!section) { return; }
  if(header)
    {
    os << "<h2>" << header << "</h2>\n";
    }
  for(const cmDocumentationEntry* op = section; op->brief;)
    {
    if(op->name)
      {
      os << "<ul>\n";
      for(;op->name;++op)
        {
        os << "  <li>\n";
        os << "    <b><code>";
        this->PrintHTMLEscapes(os, op->name);
        os << "</code></b>: ";
        this->PrintHTMLEscapes(os, op->brief);
        if(op->full)
          {
          os << "<br>";
          this->PrintFull(os, op->full,
                          &cmDocumentation::PrintHTMLPreformatted,
                          &cmDocumentation::PrintHTMLEscapes);
          }
        os << "\n";
        os << "  </li>\n";
        }
      os << "</ul>\n";
      }
    else
      {
      this->PrintHTMLEscapes(os, op->brief);
      os << "\n";
      ++op;
      }
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintUsageSection(std::ostream& os,
                                        const cmDocumentationEntry* section)
{
  if(!section) { return; }
  for(const cmDocumentationEntry* op = section; op->brief; ++op)
    {
    if(op->name)
      {
      os << "  " << op->name;
      for(int i = static_cast<int>(strlen(op->name)); i < 25; ++i)
        {
        os << " ";
        }
      os << "= " << op->brief << "\n";
      }
    else
      {
      os << "\n";
      this->PrintColumn(os, 74, "", op->brief);
      os << "\n";
      }
    }  
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintUsage(std::ostream& os)
{
  os << "Usage:\n";
  this->PrintUsageSection(os, this->UsageHelp);
  this->PrintUsageSection(os, &this->Options[0]);
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHelp(std::ostream& os)
{
  os << "Usage:\n";
  os << "\n";
  this->PrintHelpSection(os, this->UsageHelp);
  this->PrintHelpSection(os, this->Description);
  os << "--------------------------------------------------------------------------\n";
  this->PrintHelpSection(os, &this->Options[0]);
  os << "--------------------------------------------------------------------------\n";
  this->PrintHelpSection(os, this->Commands);
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintHelpHTML(std::ostream& os)
{
  os << "<html>\n"
     << "<body>\n";
  os << "<h2>Using CMake</h2>\n";
  if(this->UsageHelp)
    {
    os << "<blockquote><code>\n";
    this->PrintHelpHTMLSection(os, this->UsageHelp, 0);
    os << "</code></blockquote>\n";
    }
  this->PrintHelpHTMLSection(os, this->Description, 0);
  this->PrintHelpHTMLSection(os, &this->Options[0], "Command-line Options");
  this->PrintHelpHTMLSection(os, this->Commands, "CMakeLists.txt Commands");
  os << "</body>\n"
     << "</html>\n";
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintManPage(std::ostream& os)
{
  os << ".TH CMake 1 \""
     << cmSystemTools::GetCurrentDateTime("%B %d, %Y").c_str()
     << "\" \"CMake " CMake_VERSION_STRING "\"\n";
  this->PrintManSection(os, this->Name, "NAME");
  this->PrintManSection(os, this->UsageHelp, "SYNOPSIS");
  this->PrintManSection(os, this->Description, "DESCRIPTION");
  this->PrintManSection(os, &this->Options[0], "OPTIONS");
  this->PrintManSection(os, this->Commands, "COMMANDS");
  this->PrintManSection(os, cmDocumentationCopyright, "COPYRIGHT");
  os << ".SH MAILING LIST\n";
  os << "For help and discussion about using cmake, a mailing list is\n"
     << "provided at\n"
     << ".B cmake@www.cmake.org.\n"
     << "Please first read the full documentation at\n"
     << ".B http://www.cmake.org\n"
     << "before posting questions to the list.\n";
  os << ".SH AUTHOR\n"
     << "This manual page was generated by \"cmake --man\".\n";
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintCopyright(std::ostream& os)
{
  os << "CMake version " CMake_VERSION_STRING "\n";
  for(const cmDocumentationEntry* op = cmDocumentationCopyright;
      op->brief; ++op)
    {
    if(op->name)
      {
      os << " * ";
      this->PrintColumn(os, 74, "   ", op->brief);
      }
    else
      {
      this->PrintColumn(os, 77, "", op->brief);
      }
    os << "\n";
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintVersion(std::ostream& os)
{
  os << "CMake version " CMake_VERSION_STRING "\n";
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintColumn(std::ostream& os, int width,
                                  const char* indent, const char* text)
{
  // Print text arranged in a column of fixed witdh indented by the
  // "indent" text.
  const char* l = text;
  int column = 0;
  bool newSentence = false;
  bool firstLine = true;
  bool lastHadBlanks = false;
  
  // Count leading blanks in the text.
  int blanks = 0;
  for(const char* b = l; *b == ' '; ++b) { ++blanks; }
  
  // Loop until the end of the text.
  while(*l)
    {
    // Parse the next word.
    const char* r = l;
    while(*r && (*r != '\n') && (*r != ' ')) { ++r; }
    
    // Does it fit on this line?
    if(r-l < (width-column-(newSentence?1:0)))
      {
      // Word fits on this line.
      if(r > l)
        {
        if(column)
          {
          // Not first word on line.  Separate from the previous word
          // by a space, or two if this is a new sentence.
          if(newSentence)
            {
            os << "  ";
            column += 2;
            }
          else
            {
            os << " ";
            column += 1;
            }
          }
        else
          {
          // If we are switching from a line that has leading blanks
          // to a line that does not, or vice versa, add an extra
          // newline.
          if(blanks)
            {
            if(!lastHadBlanks && !firstLine)
              {
              os << "\n";
              }
            lastHadBlanks = true;
            }
          else
            {
            if(lastHadBlanks && !firstLine)
              {
              os << "\n";
              }
            lastHadBlanks = false;
            }
            
          // First word on line.  Print indentation unless this is the
          // first line.
          os << (firstLine?"":indent);
          
          // Further indent by leading blanks from the text on this
          // line.
          for(int i = 0; i < blanks; ++i)
            {
            os << " ";
            ++column;
            }
          blanks = 0;
          }
        
        // Print the word.
        os.write(l, static_cast<long>(r-l));
        newSentence = (*(r-1) == '.');
        }
      
      if(*r == '\n')
        {
        // Text provided a newline.  Start a new line.
        os << "\n";
        ++r;
        column = 0;
        firstLine = false;
        
        // Count leading blanks in the text.
        for(const char* b = r; *b == ' '; ++b) { ++blanks; }
        }
      else
        {
        // No provided newline.  Continue this line.
        column += static_cast<long>(r-l);
        }
      }
    else
      {
      // Word does not fit on this line.  Start a new line.
      os << "\n";
      firstLine = false;
      if(r > l)
        {
        os << indent;
        os.write(l, static_cast<long>(r-l));
        column = static_cast<long>(r-l);
        newSentence = (*(r-1) == '.');
        }
      }
    
    // Move to beginning of next word.  Skip over whitespace.
    l = r;
    while(*l && (*l == ' ')) { ++l; }    
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintFull(std::ostream& os, const char* text,
                                void (*pPreform)(std::ostream&, const char*),
                                void (*pNormal)(std::ostream&, const char*))
{
  const char* line = text;
  while(*line)
    {
    // Any lines starting in a space are treated as preformatted text.
    std::string preformatted;
    while(*line == ' ')
      {
      for(char ch = *line; ch && ch != '\n'; ++line, ch = *line)
        {
        preformatted.append(1, ch);
        }
      if(*line)
        {
        ++line;
        preformatted.append(1, '\n');
        }
      }
    if(preformatted.length())
      {
      if(pPreform)
        {
        pPreform(os, preformatted.c_str());
        }
      else
        {
        os << preformatted << "\n";
        }
      }
    
    // Other lines are treated as normal text.
    std::string normal;
    for(char ch = *line; ch && ch != '\n'; ++line, ch = *line)
      {
      normal.append(1, ch);
      }
    if(*line)
      {
      ++line;
      normal.append(1, '\n');
      }
    if(normal.length())
      {
      if(pNormal)
        {
        pNormal(os, normal.c_str());
        }
      else
        {
        os << normal << "\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::Print(Type ht, std::ostream& os)
{
  switch (ht)
    {
    case cmDocumentation::Usage:     this->PrintUsage(os); break;
    case cmDocumentation::Help:      this->PrintHelp(os); break;
    case cmDocumentation::HelpHTML:  this->PrintHelpHTML(os); break;
    case cmDocumentation::Man:       this->PrintManPage(os); break;
    case cmDocumentation::Copyright: this->PrintCopyright(os); break;
    case cmDocumentation::Version:   this->PrintVersion(os); break;
    default: break;
    }
}

//----------------------------------------------------------------------------
cmDocumentation::Type cmDocumentation::CheckOptions(int argc, char** argv)
{
  for(int i=1; i < argc; ++i)
    {
    if((strcmp(argv[i], "/?") == 0) ||
       (strcmp(argv[i], "-usage") == 0) ||
       (strcmp(argv[i], "--usage") == 0))
      {
      return cmDocumentation::Usage;
      }
    if((strcmp(argv[i], "-help") == 0) ||
       (strcmp(argv[i], "--help") == 0))
      {
      return cmDocumentation::Help;
      }
    if(strcmp(argv[i], "--help-html") == 0)
      {
      return cmDocumentation::HelpHTML;
      }
    if(strcmp(argv[i], "--man") == 0)
      {
      return cmDocumentation::Man;
      }
    if(strcmp(argv[i], "--copyright") == 0)
      {
      return cmDocumentation::Copyright;
      }
    if(strcmp(argv[i], "--version") == 0)
      {
      return cmDocumentation::Version;
      }
    }
  return cmDocumentation::None;
}

//----------------------------------------------------------------------------
void cmDocumentation::SetOptions(const cmDocumentationEntry* d)
{
  this->Options.erase(this->Options.begin(), this->Options.end());
  if(d)
    {
    for(const cmDocumentationEntry* op = d; op->brief; ++op)
      {
      this->Options.push_back(*op);
      }
    }
  for(const cmDocumentationEntry* op = cmDocumentationStandardOptions;
      op->brief; ++op)
    {
    this->Options.push_back(*op);
    }
  cmDocumentationEntry empty = {0,0,0};
  this->Options.push_back(empty);
}
