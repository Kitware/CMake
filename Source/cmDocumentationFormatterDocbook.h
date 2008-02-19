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

  virtual void PrintHeader(const char* name, std::ostream& os);
  virtual void PrintFooter(std::ostream& os);
  virtual void PrintSection(std::ostream& os,
                            const cmDocumentationSection& section,
                            const char* name);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
  virtual void PrintParagraph(std::ostream& os, const char* text);
private:
  std::set<std::string> EmittedLinkIds;
};

#endif
