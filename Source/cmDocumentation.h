/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmDocumentationFormatter.h"
#include "cmDocumentationSection.h"

struct cmDocumentationEntry;

/** Class to generate documentation.  */
class cmDocumentation
{
public:
  /** Types of help provided.  */
  enum Type
  {
    None,
    Version,
    Usage,
    Help,
    Full,
    ListManuals,
    ListCommands,
    ListModules,
    ListProperties,
    ListVariables,
    ListPolicies,
    ListGenerators,
    OneManual,
    OneCommand,
    OneModule,
    OneProperty,
    OneVariable,
    OnePolicy,
    OldCustomModules
  };

  cmDocumentation();

  /**
   * Check command line arguments for documentation options.  Returns
   * true if documentation options are found, and false otherwise.
   * When true is returned, PrintRequestedDocumentation should be
   * called.  exitOpt can be used for things like cmake -E, so that
   * all arguments after the -E are ignored and not searched for
   * help arguments.
   */
  bool CheckOptions(int argc, const char* const* argv,
                    const char* exitOpt = nullptr);

  /**
   * Print help requested on the command line.  Call after
   * CheckOptions returns true.  Returns true on success, and false
   * otherwise.  Failure can occur when output files specified on the
   * command line cannot be written.
   */
  bool PrintRequestedDocumentation(std::ostream& os);

  /** Print help of the given type.  */
  bool PrintDocumentation(Type ht, std::ostream& os);

  void SetShowGenerators(bool showGen) { this->ShowGenerators = showGen; }

  /** Set the program name for standard document generation.  */
  void SetName(const std::string& name);

  /** Set a section of the documentation. Typical sections include Name,
      Usage, Description, Options */
  void SetSection(const char* sectionName, cmDocumentationSection section);
  template <typename Iterable>
  void SetSection(const char* sectionName, const Iterable& docs)
  {
    cmDocumentationSection sec{ sectionName };
    sec.Append(docs);
    this->SetSection(sectionName, std::move(sec));
  }

  /** Add the documentation to the beginning/end of the section */
  template <typename Iterable>
  void PrependSection(const char* sectionName, const Iterable& docs)
  {
    this->SectionAtName(sectionName).Prepend(docs);
  }
  void PrependSection(const char* sectionName, cmDocumentationEntry& docs);
  template <typename Iterable>
  void AppendSection(const char* sectionName, const Iterable& docs)
  {
    this->SectionAtName(sectionName).Append(docs);
  }
  void AppendSection(const char* sectionName, cmDocumentationEntry& docs);

  /** Add common (to all tools) documentation section(s) */
  void addCommonStandardDocSections();

  /** Add the CMake standard documentation section(s) */
  void addCMakeStandardDocSections();

  /** Add the CTest standard documentation section(s) */
  void addCTestStandardDocSections();

  /** Add the CPack standard documentation section(s) */
  void addCPackStandardDocSections();

private:
  void GlobHelp(std::vector<std::string>& files, std::string const& pattern);
  void PrintNames(std::ostream& os, std::string const& pattern);
  bool PrintFiles(std::ostream& os, std::string const& pattern);

  bool PrintVersion(std::ostream& os);
  bool PrintUsage(std::ostream& os);
  bool PrintHelp(std::ostream& os);
  bool PrintHelpFull(std::ostream& os);
  bool PrintHelpOneManual(std::ostream& os);
  bool PrintHelpOneCommand(std::ostream& os);
  bool PrintHelpOneModule(std::ostream& os);
  bool PrintHelpOnePolicy(std::ostream& os);
  bool PrintHelpOneProperty(std::ostream& os);
  bool PrintHelpOneVariable(std::ostream& os);
  bool PrintHelpListManuals(std::ostream& os);
  bool PrintHelpListCommands(std::ostream& os);
  bool PrintHelpListModules(std::ostream& os);
  bool PrintHelpListProperties(std::ostream& os);
  bool PrintHelpListVariables(std::ostream& os);
  bool PrintHelpListPolicies(std::ostream& os);
  bool PrintHelpListGenerators(std::ostream& os);
  bool PrintOldCustomModules(std::ostream& os);

  const char* GetNameString() const;

  bool ShowGenerators;

  std::string NameString;
  std::map<std::string, cmDocumentationSection> AllSections;
  cmDocumentationSection& SectionAtName(const char* name);

  std::string CurrentArgument;

  struct RequestedHelpItem
  {
    Type HelpType = None;
    std::string Filename;
    std::string Argument;
  };

  std::vector<RequestedHelpItem> RequestedHelpItems;
  cmDocumentationFormatter Formatter;

  static void WarnFormFromFilename(RequestedHelpItem& request, bool& result);
};
