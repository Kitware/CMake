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
#ifndef _cmDocumentation_h
#define _cmDocumentation_h

#include "cmStandardIncludes.h"

/** Class to generate documentation.  */
class cmDocumentation
{
public:
  cmDocumentation();
  
  enum Type { None, Usage, Help, HelpHTML, Man, Copyright, Version };
  
  void Print(Type ht, std::ostream& os);
  void PrintUsage(std::ostream& os);
  void PrintHelp(std::ostream& os);
  void PrintHelpHTML(std::ostream& os);
  void PrintManPage(std::ostream& os);
  void PrintCopyright(std::ostream& os);
  void PrintVersion(std::ostream& os);
  
  void SetCommands(const cmDocumentationEntry* d);
  void SetDescription(const cmDocumentationEntry* d) {this->Description = d;}
  void SetName(const cmDocumentationEntry* d)        {this->Name = d;}
  void SetOptions(const cmDocumentationEntry* d);
  void SetUsage(const cmDocumentationEntry* d)       {this->UsageHelp = d;}
  
  Type CheckOptions(int argc, char** argv);
private:
  void PrintColumn(std::ostream& os, int width,
                   const char* indent, const char* text);
  void PrintManSection(std::ostream& os, const cmDocumentationEntry* section,
                       const char* name);
  void PrintHelpSection(std::ostream& os, const cmDocumentationEntry* section);
  static void PrintHTMLEscapes(std::ostream& os, const char* text);
  static void PrintHTMLPreformatted(std::ostream& os, const char* text);
  void PrintFull(std::ostream& os, const char* text,
                 void (*pPreform)(std::ostream&, const char*),
                 void (*pNormal)(std::ostream&, const char*));
  void PrintHelpHTMLSection(std::ostream& os,
                            const cmDocumentationEntry* section,
                            const char* header);
  void PrintUsageSection(std::ostream& os,
                         const cmDocumentationEntry* section);
  
  std::vector<cmDocumentationEntry> Commands;
  const cmDocumentationEntry* Description;
  const cmDocumentationEntry* Name;
  std::vector<cmDocumentationEntry> Options;
  const cmDocumentationEntry* UsageHelp;
};

#endif
