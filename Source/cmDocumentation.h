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
  
  // High-level interface for standard documents:
  
  /** Types of help provided.  */
  enum Type { None, Usage, Full, HTML, Man, Copyright, Version };
  
  /**
   * Check command line arguments for documentation options.  Returns
   * true if documentation options are found, and false otherwise.
   * When true is returned, PrintRequestedDocumentation should be
   * called.
   */
  bool CheckOptions(int argc, const char* const* argv);
  
  /**
   * Print help requested on the command line.  Call after
   * CheckOptions returns true.  Returns true on success, and false
   * otherwise.  Failure can occur when output files specified on the
   * command line cannot be written.
   */
  bool PrintRequestedDocumentation(std::ostream& os);
  
  /** Print help of the given type.  */
  void PrintDocumentation(Type ht, std::ostream& os);
  
  /** Set the program name for standard document generation.  */
  void SetName(const char* name);

  /** Set the program name section for standard document
   * generation.  */
  void SetNameSection(const cmDocumentationEntry*);

  /** Set the program usage for standard document generation.  */
  void SetUsageSection(const cmDocumentationEntry*);

  /** Set the program description for standard document generation.  */
  void SetDescriptionSection(const cmDocumentationEntry*);

  /** Set the program options for standard document generation.  */
  void SetOptionsSection(const cmDocumentationEntry*);
  
  /** Set the listfile commands for standard document generation.  */
  void SetCommandsSection(const cmDocumentationEntry*);
  
  /** Set the generator descriptions for standard document generation.  */
  void SetGeneratorsSection(const cmDocumentationEntry*);
  
  /** Set the see-also list of references to the other tools.  */
  void SetSeeAlsoList(const cmDocumentationEntry*);
  
  // Low-level interface for custom documents:
  
  /** Forms of documentation output.  */
  enum Form { TextForm, HTMLForm, ManForm, UsageForm };
  
  /**
   * Print documentation in the given form.  All previously added
   * sections will be generated.
   */
  void Print(Form f, std::ostream& os);
  
  /**
   * Add a section of documentation.  The cmDocumentationEntry pointer
   * should point at an array terminated by an all zero ({0,0,0})
   * entry.  This can be used to generate custom help documents.
   */
  void AddSection(const char* name, const cmDocumentationEntry* d);
  
  /** Clear all previously added sections of help.  */
  void ClearSections();  
private:
  void PrintSection(std::ostream& os,
                    const cmDocumentationEntry* section,
                    const char* name);
  void PrintSectionText(std::ostream& os,
                        const cmDocumentationEntry* section,
                        const char* name);
  void PrintSectionHTML(std::ostream& os,
                        const cmDocumentationEntry* section,
                        const char* name);
  void PrintSectionMan(std::ostream& os, const cmDocumentationEntry* section,
                       const char* name);
  void PrintSectionUsage(std::ostream& os,
                         const cmDocumentationEntry* section,
                         const char* name);
  void PrintFormatted(std::ostream& os, const char* text);
  void PrintPreformatted(std::ostream& os, const char* text);
  void PrintPreformattedText(std::ostream& os, const char* text);
  void PrintPreformattedHTML(std::ostream& os, const char* text);
  void PrintPreformattedMan(std::ostream& os, const char* text);
  void PrintParagraph(std::ostream& os, const char* text);
  void PrintParagraphText(std::ostream& os, const char* text);
  void PrintParagraphHTML(std::ostream& os, const char* text);
  void PrintParagraphMan(std::ostream& os, const char* text);
  void PrintColumn(std::ostream& os, const char* text);
  void PrintHTMLEscapes(std::ostream& os, const char* text);

  void PrintCopyright(std::ostream& os);
  void PrintVersion(std::ostream& os);
  void PrintDocumentationUsage(std::ostream& os);
  void PrintDocumentationFull(std::ostream& os);
  void PrintDocumentationHTML(std::ostream& os);
  void PrintDocumentationMan(std::ostream& os);
  void PrintDocumentationCommand(std::ostream& os,
                                 cmDocumentationEntry* entry);
  
  void CreateUsageDocumentation();
  void CreateFullDocumentation();
  void CreateManDocumentation();

  void SetSection(const cmDocumentationEntry* header,
                  const cmDocumentationEntry* section,
                  const cmDocumentationEntry* footer,
                  std::vector<cmDocumentationEntry>&);
  const char* GetNameString();

  std::string NameString;
  std::vector<cmDocumentationEntry> NameSection;
  std::vector<cmDocumentationEntry> UsageSection;
  std::vector<cmDocumentationEntry> DescriptionSection;
  std::vector<cmDocumentationEntry> OptionsSection;
  std::vector<cmDocumentationEntry> CommandsSection;
  std::vector<cmDocumentationEntry> GeneratorsSection;
  std::vector<cmDocumentationEntry> SeeAlsoSection;
  std::string SeeAlsoString;
  
  std::vector< const char* > Names;
  std::vector< const cmDocumentationEntry* > Sections;
  Form CurrentForm;
  const char* TextIndent;
  int TextWidth;
  
  typedef std::map<Type, cmStdString> RequestedMapType;
  RequestedMapType RequestedMap;
};

#endif
