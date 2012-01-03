/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentation_h
#define _cmDocumentation_h

#include "cmStandardIncludes.h"
#include "cmProperty.h"
#include "cmDocumentationFormatter.h"
#include "cmDocumentationFormatterHTML.h"
#include "cmDocumentationFormatterDocbook.h"
#include "cmDocumentationFormatterMan.h"
#include "cmDocumentationFormatterText.h"
#include "cmDocumentationFormatterUsage.h"
#include "cmDocumentationSection.h"

namespace cmsys
{
  class Directory;
}

/** Class to generate documentation.  */
class cmDocumentation: public cmDocumentationEnums
{
public:
  cmDocumentation();
  
  ~cmDocumentation();
  // High-level interface for standard documents:
  
  /**
   * Check command line arguments for documentation options.  Returns
   * true if documentation options are found, and false otherwise.
   * When true is returned, PrintRequestedDocumentation should be
   * called.  exitOpt can be used for things like cmake -E, so that 
   * all arguments after the -E are ignored and not searched for
   * help arguments.
   */
  bool CheckOptions(int argc, const char* const* argv, 
                    const char* exitOpt =0);
  
  /**
   * Print help requested on the command line.  Call after
   * CheckOptions returns true.  Returns true on success, and false
   * otherwise.  Failure can occur when output files specified on the
   * command line cannot be written.
   */
  bool PrintRequestedDocumentation(std::ostream& os);
  
  /** Print help of the given type.  */
  bool PrintDocumentation(Type ht, std::ostream& os, const char* docname=0);

  void SetShowGenerators(bool showGen) { this->ShowGenerators = showGen; }
  
  /** Set the program name for standard document generation.  */
  void SetName(const char* name);

  /** Set a section of the documentation. Typical sections include Name,
      Usage, Description, Options, SeeAlso */
  void SetSection(const char *sectionName,
                  cmDocumentationSection *section);
  void SetSection(const char *sectionName,
                  std::vector<cmDocumentationEntry> &docs);
  void SetSection(const char *sectionName,
                  const char *docs[][3]);
  void SetSections(std::map<std::string,cmDocumentationSection *>
                   &sections);

  /** Add the documentation to the beginning/end of the section */
  void PrependSection(const char *sectionName,
                      const char *docs[][3]);
  void PrependSection(const char *sectionName,
                      std::vector<cmDocumentationEntry> &docs);
  void PrependSection(const char *sectionName,
                      cmDocumentationEntry &docs);
  void AppendSection(const char *sectionName,
                     const char *docs[][3]);
  void AppendSection(const char *sectionName,
                     std::vector<cmDocumentationEntry> &docs);
  void AppendSection(const char *sectionName,
                     cmDocumentationEntry &docs);

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
   * Add a section of documentation. This can be used to generate custom help
   * documents.
   */
  void AddSectionToPrint(const char *section);

  void SetSeeAlsoList(const char *data[][3]);

  /** Clear all previously added sections of help.  */
  void ClearSections();  
  
  /** Set cmake root so we can find installed files */
  void SetCMakeRoot(const char* root)  { this->CMakeRoot = root;}

  /** Set CMAKE_MODULE_PATH so we can find additional cmake modules */
  void SetCMakeModulePath(const char* path)  { this->CMakeModulePath = path;}
  
  static Form GetFormFromFilename(const std::string& filename);

private:
  void SetForm(Form f);
  void SetDocName(const char* docname);

  bool CreateSingleModule(const char* fname, 
                          const char* moduleName,
                          cmDocumentationSection &sec);
  void CreateModuleDocsForDir(cmsys::Directory& dir, 
                              cmDocumentationSection &moduleSection);
  bool CreateModulesSection();
  bool CreateCustomModulesSection();
  void CreateFullDocumentation();

  void AddDocumentIntroToPrint(const char* intro[2]);

  bool PrintCopyright(std::ostream& os);
  bool PrintVersion(std::ostream& os);
  bool PrintDocumentationGeneric(std::ostream& os, const char *section);
  bool PrintDocumentationList(std::ostream& os, const char *section);
  bool PrintDocumentationSingle(std::ostream& os);
  bool PrintDocumentationSingleModule(std::ostream& os);
  bool PrintDocumentationSingleProperty(std::ostream& os);
  bool PrintDocumentationSinglePolicy(std::ostream& os);
  bool PrintDocumentationSingleVariable(std::ostream& os);
  bool PrintDocumentationUsage(std::ostream& os);
  bool PrintDocumentationFull(std::ostream& os);
  bool PrintDocumentationModules(std::ostream& os);
  bool PrintDocumentationCustomModules(std::ostream& os);
  bool PrintDocumentationPolicies(std::ostream& os);
  bool PrintDocumentationProperties(std::ostream& os);
  bool PrintDocumentationVariables(std::ostream& os);
  bool PrintDocumentationCurrentCommands(std::ostream& os);
  bool PrintDocumentationCompatCommands(std::ostream& os);
  void PrintDocumentationCommand(std::ostream& os,
                                 const cmDocumentationEntry &entry);


  const char* GetNameString() const;
  const char* GetDocName(bool fallbackToNameString = true) const;
  const char* GetDefaultDocName(Type ht) const;
  bool IsOption(const char* arg) const;

  bool ShowGenerators;

  std::string NameString;
  std::string DocName;
  std::map<std::string,cmDocumentationSection*> AllSections;
  
  std::string SeeAlsoString;
  std::string CMakeRoot;
  std::string CMakeModulePath;
  std::set<std::string> ModulesFound;
  std::vector< char* > ModuleStrings;
  std::vector<const cmDocumentationSection *> PrintSections;
  std::string CurrentArgument;

  struct RequestedHelpItem
  {
    RequestedHelpItem():HelpForm(TextForm), HelpType(None) {}
    cmDocumentationEnums::Form HelpForm;
    cmDocumentationEnums::Type HelpType;
    std::string Filename;
    std::string Argument;
  };

  std::vector<RequestedHelpItem> RequestedHelpItems;
  cmDocumentationFormatter* CurrentFormatter;
  cmDocumentationFormatterHTML HTMLFormatter;
  cmDocumentationFormatterDocbook DocbookFormatter;
  cmDocumentationFormatterMan ManFormatter;
  cmDocumentationFormatterText TextFormatter;
  cmDocumentationFormatterUsage UsageFormatter;

  std::vector<std::string> PropertySections;
  std::vector<std::string> VariableSections;
};

#endif
