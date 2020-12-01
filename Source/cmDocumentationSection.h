/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cmext/algorithm>

#include "cmDocumentationEntry.h"

// Low-level interface for custom documents:
/** Internal class representing a section of the documentation.
 * Cares e.g. for the different section titles in the different
 * output formats.
 */
class cmDocumentationSection
{
public:
  /** Create a cmSection, with a special name for man-output mode. */
  explicit cmDocumentationSection(const char* name)
    : Name(name)
  {
  }

  /** Has any content been added to this section or is it empty ? */
  bool IsEmpty() const { return this->Entries.empty(); }

  /** Clear contents. */
  void Clear() { this->Entries.clear(); }

  /** Return the name of this section. */
  std::string GetName() const { return this->Name; }

  /** Return a pointer to the first entry of this section. */
  const std::vector<cmDocumentationEntry>& GetEntries() const
  {
    return this->Entries;
  }

  /** Append an entry to this section. */
  void Append(const cmDocumentationEntry& entry)
  {
    this->Entries.push_back(entry);
  }
  void Append(const std::vector<cmDocumentationEntry>& entries)
  {
    cm::append(this->Entries, entries);
  }

  /** Append an entry to this section using NULL terminated chars */
  void Append(const char* [][2]);
  void Append(const char* n, const char* b);

  /** prepend some documentation to this section */
  void Prepend(const char* [][2]);
  void Prepend(const std::vector<cmDocumentationEntry>& entries)
  {
    this->Entries.insert(this->Entries.begin(), entries.begin(),
                         entries.end());
  }

private:
  std::string Name;
  std::vector<cmDocumentationEntry> Entries;
};
