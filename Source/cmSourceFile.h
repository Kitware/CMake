/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSourceFile_h
#define cmSourceFile_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include "cmCustomCommand.h"
#include "cmListFileCache.h"
#include "cmPropertyMap.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"

class cmMakefile;

/** \class cmSourceFile
 * \brief Represent a class loaded from a makefile.
 *
 * cmSourceFile is represents a class loaded from
 * a makefile.
 */
class cmSourceFile
{
public:
  /**
   * Construct with the makefile storing the source and the initial
   * name referencing it.
   */
  cmSourceFile(
    cmMakefile* mf, const std::string& name,
    cmSourceFileLocationKind kind = cmSourceFileLocationKind::Ambiguous);

  /**
   * Get the custom command for this source file
   */
  cmCustomCommand* GetCustomCommand() const;
  void SetCustomCommand(std::unique_ptr<cmCustomCommand> cc);

  //! Set/Get a property of this source file
  void SetProperty(const std::string& prop, const char* value);
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  //! Might return a nullptr if the property is not set or invalid
  const char* GetProperty(const std::string& prop) const;
  //! Always returns a valid pointer
  const char* GetSafeProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;

  /** Implement getting a property when called from a CMake language
      command like get_property or get_source_file_property.  */
  const char* GetPropertyForUser(const std::string& prop);

  //! Checks is the GENERATED property is set and true
  /// @return Equivalent to GetPropertyAsBool("GENERATED")
  bool GetIsGenerated() const { return this->IsGenerated; }

  const std::vector<BT<std::string>>& GetCompileOptions() const
  {
    return this->CompileOptions;
  }

  const std::vector<BT<std::string>>& GetCompileDefinitions() const
  {
    return this->CompileDefinitions;
  }

  const std::vector<BT<std::string>>& GetIncludeDirectories() const
  {
    return this->IncludeDirectories;
  }

  /**
   * Resolves the full path to the file.  Attempts to locate the file on disk
   * and finalizes its location.
   */
  std::string const& ResolveFullPath(std::string* error = nullptr);

  /**
   * The resolved full path to the file.  The returned file name might be empty
   * if the path has not yet been resolved.
   */
  std::string const& GetFullPath() const;

  /**
   * Get the information currently known about the source file
   * location without attempting to locate the file as GetFullPath
   * would.  See cmSourceFileLocation documentation.
   */
  cmSourceFileLocation const& GetLocation() const;

  /**
   * Get the file extension of this source file.
   */
  std::string const& GetExtension() const;

  /**
   * Get the language of the compiler to use for this source file.
   */
  std::string const& GetOrDetermineLanguage();
  std::string GetLanguage() const;

  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string>& GetDepends() const { return this->Depends; }
  void AddDepend(const std::string& d) { this->Depends.push_back(d); }

  // Get the properties
  const cmPropertyMap& GetProperties() const { return this->Properties; }
  // Set the properties
  void SetProperties(cmPropertyMap properties);

  /**
   * Check whether the given source file location could refer to this
   * source.
   */
  bool Matches(cmSourceFileLocation const&);

  void SetObjectLibrary(std::string const& objlib);
  std::string GetObjectLibrary() const;

private:
  cmSourceFileLocation Location;
  cmPropertyMap Properties;
  std::unique_ptr<cmCustomCommand> CustomCommand;
  std::string Extension;
  std::string Language;
  std::string FullPath;
  std::string ObjectLibrary;
  std::vector<std::string> Depends;
  std::vector<BT<std::string>> CompileOptions;
  std::vector<BT<std::string>> CompileDefinitions;
  std::vector<BT<std::string>> IncludeDirectories;
  bool FindFullPathFailed = false;
  bool IsGenerated = false;

  bool FindFullPath(std::string* error);
  void CheckExtension();
  void CheckLanguage(std::string const& ext);

  static const std::string propLANGUAGE;
  static const std::string propLOCATION;
  static const std::string propGENERATED;
  static const std::string propCOMPILE_DEFINITIONS;
  static const std::string propCOMPILE_OPTIONS;
  static const std::string propINCLUDE_DIRECTORIES;
};

// TODO: Factor out into platform information modules.
#define CM_HEADER_REGEX "\\.(h|hh|h\\+\\+|hm|hpp|hxx|in|txx|inl)$"

#define CM_SOURCE_REGEX                                                       \
  "\\.(C|F|M|c|c\\+\\+|cc|cpp|cxx|cu|f|f90|for|fpp|ftn|m|mm|"                 \
  "rc|def|r|odl|idl|hpj|bat)$"

#define CM_PCH_REGEX "cmake_pch\\.(h|hxx)$"

#define CM_RESOURCE_REGEX "\\.(pdf|plist|png|jpeg|jpg|storyboard|xcassets)$"

#endif
