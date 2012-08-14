/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmDocumentationFormatterUsage.h"
#include "cmDocumentationSection.h"

cmDocumentationFormatterUsage::cmDocumentationFormatterUsage()
:cmDocumentationFormatterText()
{
}

void cmDocumentationFormatterUsage
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  if(name)
    {
    os << name << "\n";
    }

  const std::vector<cmDocumentationEntry> &entries =
    section.GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      os << "  " << op->Name;
      this->TextIndent = "                                ";
      int align = static_cast<int>(strlen(this->TextIndent))-4;
      for(int i = static_cast<int>(op->Name.size()); i < align; ++i)
        {
        os << " ";
        }
      if (op->Name.size() > strlen(this->TextIndent)-4 )
        {
        os << "\n";
        os.write(this->TextIndent, strlen(this->TextIndent)-2);
        }
      os << "= ";
      this->PrintColumn(os, op->Brief.c_str());
      os << "\n";
      }
    else
      {
      os << "\n";
      this->TextIndent = "";
      this->PrintFormatted(os, op->Brief.c_str());
      }
    }
  os << "\n";
}

