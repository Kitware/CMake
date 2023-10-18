/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <cm/string_view>

/** \class cmQtAutoGen
 * \brief Common base class for QtAutoGen classes
 */
class cmQtAutoGen
{
public:
  /** String value with per configuration variants.  */
  class ConfigString
  {
  public:
    std::string Default;
    std::unordered_map<std::string, std::string> Config;
  };

  /** String values with per configuration variants.  */
  template <typename C>
  class ConfigStrings
  {
  public:
    C Default;
    std::unordered_map<std::string, C> Config;
  };
  /** Integer version.  */
  struct IntegerVersion
  {
    unsigned int Major = 0;
    unsigned int Minor = 0;

    IntegerVersion() = default;
    IntegerVersion(unsigned int major, unsigned int minor)
      : Major(major)
      , Minor(minor)
    {
    }

    bool operator>(IntegerVersion const version) const
    {
      return (this->Major > version.Major) ||
        ((this->Major == version.Major) && (this->Minor > version.Minor));
    }

    bool operator>=(IntegerVersion const version) const
    {
      return (this->Major > version.Major) ||
        ((this->Major == version.Major) && (this->Minor >= version.Minor));
    }
  };

  /** Compiler features.  */
  class CompilerFeatures
  {
  public:
    bool Evaluated = false;
    std::string HelpOutput;
    std::vector<std::string> ListOptions;
  };
  using CompilerFeaturesHandle = std::shared_ptr<CompilerFeatures>;

  /** AutoGen generator type.  */
  enum class GenT
  {
    GEN, // AUTOGEN
    MOC, // AUTOMOC
    UIC, // AUTOUIC
    RCC  // AUTORCC
  };

  /// @brief Maximum number of parallel threads/processes in a generator
  static unsigned int const ParallelMax;

  /// @brief Returns the generator name
  static cm::string_view GeneratorName(GenT genType);
  /// @brief Returns the generator name in upper case
  static cm::string_view GeneratorNameUpper(GenT genType);

  /// @brief Returns a string with the requested tool names
  static std::string Tools(bool moc, bool uic, bool rcc);

  /// @brief Returns the string escaped and enclosed in quotes
  static std::string Quoted(cm::string_view text);

  static std::string QuotedCommand(std::vector<std::string> const& command);

  /// @brief Returns the file name without path and extension (thread safe)
  static std::string FileNameWithoutLastExtension(cm::string_view filename);

  /// @brief Returns the parent directory of the file (thread safe)
  static std::string ParentDir(cm::string_view filename);

  /// @brief Returns the parent directory of the file with a "/" suffix
  static std::string SubDirPrefix(cm::string_view filename);

  /// @brief Appends the suffix to the filename before the last dot
  static std::string AppendFilenameSuffix(cm::string_view filename,
                                          cm::string_view suffix);

  /// @brief Merges newOpts into baseOpts
  static void UicMergeOptions(std::vector<std::string>& baseOpts,
                              std::vector<std::string> const& newOpts,
                              bool isQt5OrLater);

  /// @brief Merges newOpts into baseOpts
  static void RccMergeOptions(std::vector<std::string>& baseOpts,
                              std::vector<std::string> const& newOpts,
                              bool isQt5OrLater);

  /** @class RccLister
   * @brief Lists files in qrc resource files
   */
  class RccLister
  {
  public:
    RccLister();
    RccLister(std::string rccExecutable, std::vector<std::string> listOptions);

    //! The rcc executable
    std::string const& RccExcutable() const { return this->RccExcutable_; }
    void SetRccExecutable(std::string const& rccExecutable)
    {
      this->RccExcutable_ = rccExecutable;
    }

    //! The rcc executable list options
    std::vector<std::string> const& ListOptions() const
    {
      return this->ListOptions_;
    }
    void SetListOptions(std::vector<std::string> const& listOptions)
    {
      this->ListOptions_ = listOptions;
    }

    /**
     * @brief Lists a files in the qrcFile
     * @arg files The file names are appended to this list
     * @arg error contains the error message when the function fails
     */
    bool list(std::string const& qrcFile, std::vector<std::string>& files,
              std::string& error, bool verbose = false) const;

  private:
    std::string RccExcutable_;
    std::vector<std::string> ListOptions_;
  };
};
