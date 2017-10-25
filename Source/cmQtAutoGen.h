/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGen_h
#define cmQtAutoGen_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

/** \class cmQtAutoGen
 * \brief Class used as namespace for QtAutogen related types  and functions
 */
class cmQtAutoGen
{
public:
  static std::string const listSep;

  enum Generator
  {
    GEN, // General
    MOC,
    UIC,
    RCC
  };

  enum MultiConfig
  {
    SINGLE, // Single configuration
    WRAP,   // Multi configuration using wrapper files
    FULL    // Full multi configuration using per config sources
  };

public:
  /// @brief Returns the generator name
  static std::string const& GeneratorName(Generator genType);
  /// @brief Returns the generator name in upper case
  static std::string GeneratorNameUpper(Generator genType);

  /// @brief Returns the multi configuration name string
  static std::string const& MultiConfigName(MultiConfig config);
  /// @brief Returns the multi configuration type
  static MultiConfig MultiConfigType(std::string const& name);

  /// @brief Returns a the string escaped and enclosed in quotes
  static std::string Quoted(std::string const& text);

  /// @brief Appends the suffix to the filename before the last dot
  static std::string AppendFilenameSuffix(std::string const& filename,
                                          std::string const& suffix);

  /// @brief Merges newOpts into baseOpts
  static void UicMergeOptions(std::vector<std::string>& baseOpts,
                              std::vector<std::string> const& newOpts,
                              bool isQt5);

  /// @brief Merges newOpts into baseOpts
  static void RccMergeOptions(std::vector<std::string>& baseOpts,
                              std::vector<std::string> const& newOpts,
                              bool isQt5);

  /// @brief Reads the resource files list from from a .qrc file
  /// @arg fileName Must be the absolute path of the .qrc file
  /// @return True if the rcc file was successfully parsed
  static bool RccListInputs(std::string const& qtMajorVersion,
                            std::string const& rccCommand,
                            std::string const& fileName,
                            std::vector<std::string>& files,
                            std::string* errorMessage = nullptr);
};

#endif
