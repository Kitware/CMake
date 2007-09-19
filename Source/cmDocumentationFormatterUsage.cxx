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

#include "cmDocumentationFormatterUsage.h"

cmDocumentationFormatterUsage::cmDocumentationFormatterUsage()
:cmDocumentationFormatterText()
{
}

void cmDocumentationFormatterUsage::PrintSection(std::ostream& os,
                    const cmDocumentationEntry* section,
                    const char* name)
{
  if(name)
    {
    os << name << "\n";
    }
  if(!section) { return; }
  for(const cmDocumentationEntry* op = section; op->brief; ++op)
    {
    if(op->name)
      {
      os << "  " << op->name;
      this->TextIndent = "                                ";
      int align = static_cast<int>(strlen(this->TextIndent))-4;
      for(int i = static_cast<int>(strlen(op->name)); i < align; ++i)
        {
        os << " ";
        }
      if ( strlen(op->name) > strlen(this->TextIndent)-4 )
        {
        os << "\n";
        os.write(this->TextIndent, strlen(this->TextIndent)-2);
        }
      os << "= ";
      this->PrintColumn(os, op->brief);
      os << "\n";
      }
    else
      {
      os << "\n";
      this->TextIndent = "";
      this->PrintFormatted(os, op->brief);
      }
    }
  os << "\n";
}

