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
  static const std::string listSep;

  enum GeneratorType
  {
    GEN, // General
    MOC,
    UIC,
    RCC
  };

public:
  /// @brief Returns the generator name
  static const std::string& GeneratorName(GeneratorType genType);
  /// @brief Returns the generator name in upper case
  static std::string GeneratorNameUpper(GeneratorType genType);

  /// @brief Returns a the string escaped and enclosed in quotes
  ///
  static std::string Quoted(const std::string& text);

  /// @brief Merges newOpts into baseOpts
  static void UicMergeOptions(std::vector<std::string>& baseOpts,
                              const std::vector<std::string>& newOpts,
                              bool isQt5);

  /// @brief Merges newOpts into baseOpts
  static void RccMergeOptions(std::vector<std::string>& baseOpts,
                              const std::vector<std::string>& newOpts,
                              bool isQt5);

  /// @brief Reads the resource files list from from a .qrc file
  /// @arg fileName Must be the absolute path of the .qrc file
  /// @return True if the rcc file was successfully parsed
  static bool RccListInputs(const std::string& qtMajorVersion,
                            const std::string& rccCommand,
                            const std::string& fileName,
                            std::vector<std::string>& files,
                            std::string* errorMessage = nullptr);
};

#endif
