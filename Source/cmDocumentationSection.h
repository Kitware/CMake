/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iterator>
#include <string>
#include <vector>

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
  explicit cmDocumentationSection(char const* name)
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
  std::vector<cmDocumentationEntry> const& GetEntries() const
  {
    return this->Entries;
  }

  /** Append an entry to this section. */
  void Append(cmDocumentationEntry const& entry)
  {
    this->Entries.push_back(entry);
  }

  template <typename Iterable>
  void Append(Iterable const& entries)
  {
    this->Entries.insert(std::end(this->Entries), std::begin(entries),
                         std::end(entries));
  }

  /** prepend some documentation to this section */
  template <typename Iterable>
  void Prepend(Iterable const& entries)
  {
    this->Entries.insert(std::begin(this->Entries), std::begin(entries),
                         std::end(entries));
  }

private:
  std::string Name;
  std::vector<cmDocumentationEntry> Entries;
};
