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
#include "cmDocumentationFormatter.h"

cmDocumentationFormatter::cmDocumentationFormatter()
{
}

cmDocumentationFormatter::~cmDocumentationFormatter()
{
}

void cmDocumentationFormatter::PrintFormatted(std::ostream& os, 
                                              const char* text)
{
  if(!text)
    {
    return;
    }
  const char* ptr = text;
  while(*ptr)
    {
    // Any ptrs starting in a space are treated as preformatted text.
    std::string preformatted;
    while(*ptr == ' ')
      {
      for(char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr)
        {
        preformatted.append(1, ch);
        }
      if(*ptr)
        {
        ++ptr;
        preformatted.append(1, '\n');
        }
      }
    if(preformatted.length())
      {
      this->PrintPreformatted(os, preformatted.c_str());
      }

    // Other ptrs are treated as paragraphs.
    std::string paragraph;
    for(char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr)
      {
      paragraph.append(1, ch);
      }
    if(*ptr)
      {
      ++ptr;
      paragraph.append(1, '\n');
      }
    if(paragraph.length())
      {
      this->PrintParagraph(os, paragraph.c_str());
      }
    }
}

