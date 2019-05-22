/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGen_h
#define cmQtAutoGen_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory> // IWYU pragma: keep
#include <string>
#include <vector>

/** \class cmQtAutoGen
 * \brief Common base class for QtAutoGen classes
 */
class cmQtAutoGen
{
public:
  /// @brief Integer version
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

    bool operator>(IntegerVersion const version)
    {
      return (this->Major > version.Major) ||
        ((this->Major == version.Major) && (this->Minor > version.Minor));
    }

    bool operator>=(IntegerVersion const version)
    {
      return (this->Major > version.Major) ||
        ((this->Major == version.Major) && (this->Minor >= version.Minor));
    }
  };

  class CompilerFeatures
  {
  public:
    bool Evaluated = false;
    std::string HelpOutput;
    std::vector<std::string> ListOptions;
  };
  typedef std::shared_ptr<CompilerFeatures> CompilerFeaturesHandle;

  /// @brief AutoGen generator type
  enum class GenT
  {
    GEN, // AUTOGEN
    MOC, // AUTOMOC
    UIC, // AUTOUIC
    RCC  // AUTORCC
  };

  /// @brief Nested lists separator
  static std::string const ListSep;
  /// @brief Maximum number of parallel threads/processes in a generator
  static unsigned int const ParallelMax;

public:
  /// @brief Returns the generator name
  static std::string const& GeneratorName(GenT genType);
  /// @brief Returns the generator name in upper case
  static std::string const& GeneratorNameUpper(GenT genType);

  /// @brief Returns a string with the requested tool names
  static std::string Tools(bool moc, bool uic, bool rcc);

  /// @brief Returns the string escaped and enclosed in quotes
  static std::string Quoted(std::string const& text);

  static std::string QuotedCommand(std::vector<std::string> const& command);

  /// @brief Returns the parent directory of the file with a "/" suffix
  static std::string SubDirPrefix(std::string const& filename);

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

  /** @class RccLister
   * @brief Lists files in qrc resource files
   */
  class RccLister
  {
  public:
    RccLister();
    RccLister(std::string rccExecutable, std::vector<std::string> listOptions);

    //! The rcc executable
    std::string const& RccExcutable() const { return RccExcutable_; }
    void SetRccExecutable(std::string const& rccExecutable)
    {
      RccExcutable_ = rccExecutable;
    }

    //! The rcc executable list options
    std::vector<std::string> const& ListOptions() const
    {
      return ListOptions_;
    }
    void SetListOptions(std::vector<std::string> const& listOptions)
    {
      ListOptions_ = listOptions;
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

#endif
