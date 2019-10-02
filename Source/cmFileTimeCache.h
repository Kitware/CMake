/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileTimeCache_h
#define cmFileTimeCache_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>

#include "cmFileTime.h" // IWYU pragma: keep

/** \class cmFileTimeCache
 * \brief Caches file modification times in an internal map for fast lookups.
 */
class cmFileTimeCache
{
public:
  cmFileTimeCache();
  ~cmFileTimeCache();

  cmFileTimeCache(const cmFileTimeCache&) = delete;
  cmFileTimeCache& operator=(const cmFileTimeCache&) = delete;

  /**
   * @brief Loads the file time from the cache or the file system.
   * @return true on success
   */
  bool Load(std::string const& fileName, cmFileTime& fileTime);

  /**
   * @brief Removes a file time from the cache
   * @return true if the file was found in the cache and removed
   */
  bool Remove(std::string const& fileName);

  /**
   * @brief Compare file modification times.
   * @return true for successful comparison and false for error.
   *
   * When true is returned, result has -1, 0, +1 for
   * f1 older, same, or newer than f2.
   */
  bool Compare(std::string const& f1, std::string const& f2, int* result);

  /**
   * @brief Compare file modification times.
   * @return true unless both files exist and have modification times less
   *         than 1 second apart.
   */
  bool DifferS(std::string const& f1, std::string const& f2);

private:
  std::unordered_map<std::string, cmFileTime> Cache;
};

#endif
