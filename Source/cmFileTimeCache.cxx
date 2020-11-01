/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileTimeCache.h"

#include <string>
#include <unordered_map>
#include <utility>

cmFileTimeCache::cmFileTimeCache() = default;

cmFileTimeCache::~cmFileTimeCache() = default;

bool cmFileTimeCache::Load(std::string const& fileName, cmFileTime& fileTime)
{
  // Use the stored time if available.
  {
    auto fit = this->Cache.find(fileName);
    if (fit != this->Cache.end()) {
      fileTime = fit->second;
      return true;
    }
  }
  // Read file time from OS
  if (!fileTime.Load(fileName)) {
    return false;
  }
  // Store file time in cache
  this->Cache[fileName] = fileTime;
  return true;
}

bool cmFileTimeCache::Remove(std::string const& fileName)
{
  return (this->Cache.erase(fileName) != 0);
}

bool cmFileTimeCache::Compare(std::string const& f1, std::string const& f2,
                              int* result)
{
  // Get the modification time for each file.
  cmFileTime ft1;
  cmFileTime ft2;
  if (this->Load(f1, ft1) && this->Load(f2, ft2)) {
    // Compare the two modification times.
    *result = ft1.Compare(ft2);
    return true;
  }
  // No comparison available.  Default to the same time.
  *result = 0;
  return false;
}

bool cmFileTimeCache::DifferS(std::string const& f1, std::string const& f2)
{
  // Get the modification time for each file.
  cmFileTime ft1;
  cmFileTime ft2;
  if (this->Load(f1, ft1) && this->Load(f2, ft2)) {
    // Compare the two modification times.
    return ft1.DifferS(ft2);
  }
  // No comparison available.  Default to different times.
  return true;
}
