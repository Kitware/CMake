/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmPathLabel
 * \brief Helper class for text based labels
 *
 * cmPathLabel is extended in different classes to act as an inheritable
 * enum.  Comparisons are done on a precomputed Jenkins hash of the string
 * label for indexing and searchig.
 */
class cmPathLabel
{
public:
  cmPathLabel(std::string label);

  // The comparison operators are only for quick sorting and searching and
  // in no way imply any lexicographical order of the label
  bool operator<(cmPathLabel const& l) const;
  bool operator==(cmPathLabel const& l) const;

  std::string const& GetLabel() const { return this->Label; }
  unsigned int const& GetHash() const { return this->Hash; }

protected:
  cmPathLabel();

  std::string Label;
  unsigned int Hash;
};
