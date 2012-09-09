/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentationFormatterDocbook_h
#define _cmDocumentationFormatterDocbook_h

#include "cmStandardIncludes.h"

#include "cmDocumentationFormatter.h"

/** Class to print the documentation as Docbook.
 http://www.oasis-open.org/docbook/xml/4.2/   */
class cmDocumentationFormatterDocbook : public cmDocumentationFormatter
{
public:
  cmDocumentationFormatterDocbook();

  virtual cmDocumentationEnums::Form GetForm() const
                                  { return cmDocumentationEnums::DocbookForm;}

  virtual void PrintHeader(const char* docname, const char* appname,
                           std::ostream& os);
  virtual void PrintFooter(std::ostream& os);
  virtual void PrintSection(std::ostream& os,
                            const cmDocumentationSection& section,
                            const char* name);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
  virtual void PrintParagraph(std::ostream& os, const char* text);
private:
  void PrintId(std::ostream& os, const char* prefix, std::string id);
  std::set<std::string> EmittedLinkIds;
  std::string Docname;
};

#endif
