/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationFormatterRST.h"
#include "cmDocumentationSection.h"
#include "cmVersion.h"

#include "cmSystemTools.h"

cmDocumentationFormatterRST::cmDocumentationFormatterRST()
:cmDocumentationFormatterText()
{
}

static std::string rstFileName(std::string fn)
{
  cmSystemTools::ReplaceString(fn, "<", "");
  cmSystemTools::ReplaceString(fn, ">", "");
  return fn;
}

void cmDocumentationFormatterRST
::PrintSection(std::ostream& os,
               const cmDocumentationSection &section,
               const char* name)
{
  std::string prefix = this->ComputeSectionLinkPrefix(name);
  std::vector<cmDocumentationEntry> const& entries = section.GetEntries();
  this->TextWidth = 70;

  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end();)
    {
    if(op->Name.size())
      {
      for(;op != entries.end() && op->Name.size(); ++op)
        {
        if(prefix == "opt" || prefix == "see")
          {
          os << "\n";
          os << "* ``" << op->Name << "``: " << op->Brief << "\n";
          this->TextIndent = "  ";
          if(op->Full.size())
            {
            os << "\n";
            this->PrintFormatted(os, op->Full.c_str());
            }
          this->TextIndent = "";
          }
        else
          {
          cmSystemTools::MakeDirectory(prefix.c_str());
          std::string fname = prefix + "/" + rstFileName(op->Name) + ".rst";
          if(cmSystemTools::FileExists(fname.c_str()))
            {
            cmSystemTools::Error("Duplicate file name: ", fname.c_str());
            continue;
            }
          std::ofstream of(fname.c_str());
          of << op->Name << "\n";
          for(size_t i = 0; i < op->Name.size(); ++i)
            {
            of << "-";
            }
          of << "\n\n" << op->Brief << "\n";
          if(op->Full.size())
            {
            of << "\n";
            this->PrintFormatted(of, op->Full.c_str());
            }
          }
        }
      }
    else
      {
      this->PrintFormatted(os, op->Brief.c_str());
      os << "\n";
      ++op;
      }
    }
}

void cmDocumentationFormatterRST::PrintPreformatted(std::ostream& os,
                                                    const char* text)
{
  os << this->TextIndent << "::\n\n";
  bool newline = true;
  for(const char* c = text; *c; ++c)
    {
    if (newline)
      {
      os << this->TextIndent;
      newline = false;
      }
    os << *c;
    newline = (*c == '\n');
    }
  os << "\n";
}
