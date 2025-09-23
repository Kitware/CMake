/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <string>

class cmDocumentationSection;

/** Print documentation in a simple text format. */
class cmDocumentationFormatter
{
public:
  std::string Format(std::string text) const;
  void PrintSection(std::ostream& os, cmDocumentationSection const& section);
  void PrintFormatted(std::ostream& os, std::string const& text) const
  {
    os << this->Format(text);
  }
  void SetIndent(std::size_t indent) { this->TextIndent = indent; }

  static constexpr std::size_t TEXT_WIDTH = 77u;

private:
  void PrintPreformatted(std::ostream& os, std::string const&) const;
  void PrintParagraph(std::ostream& os, std::string const&) const;
  void PrintColumn(std::ostream& os, std::string const&) const;

  std::size_t TextIndent = 0u;
};
