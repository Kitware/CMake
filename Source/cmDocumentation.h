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
  
  ~cmDocumentation();
  // High-level interface for standard documents:
  
  /** Types of help provided.  */
  enum Type { None, Usage, Single, SingleModule, SingleProperty,
              List, ModuleList, PropertyList,
              Full, HTML, Man, Copyright, Version };
  
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
  bool PrintDocumentation(Type ht, std::ostream& os);
  
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
  
  /** Set the listfile compat. commands for standard document generation.  */
  void SetCompatCommandsSection(const cmDocumentationEntry*);
  
  /** Set the properties for standard document generation.  */
  void SetPropertiesSection(const cmDocumentationEntry*);

  /** Set the generator descriptions for standard document generation.  */
  void SetGeneratorsSection(const cmDocumentationEntry*);
  
  /** Set the see-also list of references to the other tools.  */
  void SetSeeAlsoList(const cmDocumentationEntry*);
  
  // Low-level interface for custom documents:
  
  /** Forms of documentation output.  */
  enum Form { TextForm, HTMLForm, ManForm, UsageForm };
  
  
  /** Internal class representing a section of the documentation.
   * Cares e.g. for the different section titles in the different
   * output formats.
   */
  class cmSection
  {
    public:
      /** Create a cmSection, with a special name for man-output mode. */
      cmSection(const char* name, const char* manName)
                :Name(name), ManName(manName)       {}

      /** Has any content been added to this section or is it empty ? */
      bool IsEmpty() const
        { return this->Entries.empty(); }

      /** Clear contents. */
      void Clear()
        { this->Entries.clear(); }

      /** Return the name of this section for the given output form. */
      const char* GetName(Form form) const
        { return (form==ManForm?this->ManName.c_str():this->Name.c_str()); }

      /** Return a pointer to the first entry of this section. */
      cmDocumentationEntry *GetEntries()
        { return &this->Entries[0]; }

      /** Return a pointer to the first entry of this section. */
      const cmDocumentationEntry *GetEntries() const
        { return &this->Entries[0]; }

      /** Append an entry to this section. */
      void Append(const cmDocumentationEntry& entry)
        { this->Entries.push_back(entry); }

      /** Set the contents of this section. */
      void Set(const cmDocumentationEntry* header,
               const cmDocumentationEntry* section,
               const cmDocumentationEntry* footer);

    private:
      std::string Name;
      std::string ManName;
      std::vector<cmDocumentationEntry> Entries;
  };

  /**
   * Print documentation in the given form.  All previously added
   * sections will be generated.
   */
  void Print(Form f, std::ostream& os);
  
  /**
   * Print documentation in the current form.  All previously added
   * sections will be generated.
   */
  void Print(std::ostream& os);

  /**
   * Add a section of documentation.  The cmDocumentationEntry pointer
   * should point at an array terminated by an all zero ({0,0,0})
   * entry.  This can be used to generate custom help documents.
   */
  void AddSection(const char* name, const cmDocumentationEntry* d);
  
  /** Convenience function, does the same as above */
  void AddSection(const cmSection& section);

  /** Clear all previously added sections of help.  */
  void ClearSections();  
  
  /** Set cmake root so we can find installed files */
  void SetCMakeRoot(const char* root)  { this->CMakeRoot = root;}
private:
  void PrintHeader(const char* title, std::ostream& os);
  void PrintFooter(std::ostream& os);

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

  bool CreateSingleModule(const char* fname, const char* moduleName);
  bool CreateModulesSection();
  bool PrintCopyright(std::ostream& os);
  bool PrintVersion(std::ostream& os);
  bool PrintDocumentationList(std::ostream& os);
  bool PrintModuleList(std::ostream& os);
  bool PrintPropertyList(std::ostream& os);
  bool PrintDocumentationSingle(std::ostream& os);
  bool PrintDocumentationSingleModule(std::ostream& os);
  bool PrintDocumentationSingleProperty(std::ostream& os);
  bool PrintDocumentationUsage(std::ostream& os);
  bool PrintDocumentationFull(std::ostream& os);
  void PrintDocumentationCommand(std::ostream& os,
                                 cmDocumentationEntry* entry);
  
  void CreateUsageDocumentation();
  void CreateFullDocumentation();
  void CreateCurrentCommandDocumentation();
  void CreateCompatCommandDocumentation();
  void CreateModulesDocumentation();
  void CreatePropertiesDocumentation();

  void SetSection(const cmDocumentationEntry* header,
                  const cmDocumentationEntry* section,
                  const cmDocumentationEntry* footer,
                  std::vector<cmDocumentationEntry>&);
  const char* GetNameString() const;
  bool IsOption(const char* arg) const;

  std::string NameString;
  cmSection NameSection;
  cmSection UsageSection;
  cmSection DescriptionSection;
  cmSection OptionsSection;
  cmSection CommandsSection;
  cmSection CompatCommandsSection;
  cmSection ModulesSection;
  cmSection PropertiesSection;
  cmSection GeneratorsSection;
  cmSection SeeAlsoSection;
  cmSection CopyrightSection;
  cmSection AuthorSection;

  std::string SeeAlsoString;
  std::string SingleCommand;
  std::string SingleModuleName;
  std::string SinglePropertyName;
  std::string CMakeRoot;
  std::vector< char* > ModuleStrings;
  std::vector< const char* > Names;
  std::vector< const cmDocumentationEntry* > Sections;
  Form CurrentForm;
  const char* TextIndent;
  int TextWidth;
  
  typedef std::map<Type, cmStdString> RequestedMapType;
  RequestedMapType RequestedMap;
};

#endif
