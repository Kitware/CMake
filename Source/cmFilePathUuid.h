/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFilePathUuid_h
#define cmFilePathUuid_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <stddef.h>
#include <string>
#include <utility>

class cmMakefile;

/** \class cmFilePathUuid
 * @brief Generates a unique pathless file name with a checksum component
 *        calculated from the file path.
 *
 * The checksum is calculated from the relative file path to the
 * closest known project directory. This guarantees reproducibility
 * when source and build directory differ e.g. for different project
 * build directories.
 */
class cmFilePathUuid
{
public:
  /// Maximum number of characters to use from the file name
  static const size_t partLengthName = 14;
  /// Maximum number of characters to use from the path checksum
  static const size_t partLengthCheckSum = 14;

  /// @brief Initilizes the parent directories from a makefile
  cmFilePathUuid(cmMakefile* makefile);

  /// @brief Initilizes the parent directories manually
  cmFilePathUuid(const std::string& currentSrcDir,
                 const std::string& currentBinDir,
                 const std::string& projectSrcDir,
                 const std::string& projectBinDir);

  /* @brief Calculates and returns the uuid for a file path
   *
   * @arg outputPrefix optional string to prepend to the result
   * @arg outputSuffix optional string to append to the result
   */
  std::string get(const std::string& filePath,
                  const char* outputPrefix = CM_NULLPTR,
                  const char* outputSuffix = CM_NULLPTR);

private:
  void initParentDirs(const std::string& currentSrcDir,
                      const std::string& currentBinDir,
                      const std::string& projectSrcDir,
                      const std::string& projectBinDir);

  /// Returns the relative path and the parent directory key string (seed)
  void GetRelPathSeed(const std::string& filePath, std::string& sourceRelPath,
                      std::string& sourceRelSeed);

  std::string GetChecksumString(const std::string& sourceFilename,
                                const std::string& sourceRelPath,
                                const std::string& sourceRelSeed);

  /// Size of the parent directory list
  static const size_t numParentDirs = 4;
  /// List of (directory name, seed name) pairs
  std::pair<std::string, std::string> parentDirs[numParentDirs];
};

#endif
