/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmDocumentationFormatterText.h"
#include "cmDocumentationSection.h"

cmDocumentationFormatterText::cmDocumentationFormatterText()
:cmDocumentationFormatter()
,TextWidth(77)
,TextIndent("")
{
}

void cmDocumentationFormatterText
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  if(name && (strcmp(name, "SingleItem")!=0))
    {
    os <<
      "---------------------------------------"
      "---------------------------------------\n";
    os << name << "\n\n";
    }

  const std::vector<cmDocumentationEntry> &entries =
    section.GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      os << "  " << op->Name << "\n";
      this->TextIndent = "       ";
      this->PrintFormatted(os, op->Brief.c_str());
      if(op->Full.size())
        {
        os << "\n";
        this->PrintFormatted(os, op->Full.c_str());
        }
      }
    else
      {
      this->TextIndent = "";
      this->PrintFormatted(os, op->Brief.c_str());
      }
    os << "\n";
    }
}

void cmDocumentationFormatterText::PrintPreformatted(std::ostream& os,
                                                     const char* text)
{
  bool newline = true;
  for(const char* ptr = text; *ptr; ++ptr)
    {
    if(newline && *ptr != '\n')
      {
      os << this->TextIndent;
      newline = false;
      }
    os << *ptr;
    if(*ptr == '\n')
      {
      newline = true;
      }
    }
  os << "\n";
}

void cmDocumentationFormatterText::PrintParagraph(std::ostream& os,
                                                  const char* text)
{
  os << this->TextIndent;
  this->PrintColumn(os, text);
  os << "\n";
}

void cmDocumentationFormatterText::SetIndent(const char* indent)
{
  this->TextIndent = indent;
}

void cmDocumentationFormatterText::PrintColumn(std::ostream& os,
                                               const char* text)
{
  // Print text arranged in an indented column of fixed witdh.
  const char* l = text;
  long column = 0;
  bool newSentence = false;
  bool firstLine = true;
  int width = this->TextWidth - static_cast<int>(strlen(this->TextIndent));

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
          // First word on line.  Print indentation unless this is the
          // first line.
          os << (firstLine?"":this->TextIndent);
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
        os << this->TextIndent;
        os.write(l, static_cast<long>(r-l));
        column = static_cast<long>(r-l);
        newSentence = (*(r-1) == '.');
        }
      else
        {
        column = 0;
        }
      }

    // Move to beginning of next word.  Skip over whitespace.
    l = r;
    while(*l && (*l == ' ')) { ++l; }
    }
}
