/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentationFormatterMan_h
#define _cmDocumentationFormatterMan_h

#include "cmStandardIncludes.h"

#include "cmDocumentationFormatter.h"

/** Class to print the documentation as man page.  */
class cmDocumentationFormatterMan : public cmDocumentationFormatter
{
public:
  cmDocumentationFormatterMan();

  virtual cmDocumentationEnums::Form GetForm() const
                                      { return cmDocumentationEnums::ManForm;}

  virtual void PrintHeader(const char* docname, const char* appname,
                           std::ostream& os);
  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
  virtual void PrintParagraph(std::ostream& os, const char* text);

private:
  void EscapeText(std::string& man_text);
};

#endif
