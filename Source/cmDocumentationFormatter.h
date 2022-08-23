/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>

class cmDocumentationSection;

/** Print documentation in a simple text format. */
class cmDocumentationFormatter
{
public:
  void PrintFormatted(std::ostream& os, const char* text);
  void PrintSection(std::ostream& os, cmDocumentationSection const& section);
  void PrintPreformatted(std::ostream& os, const char* text);
  void PrintParagraph(std::ostream& os, const char* text);
  void PrintColumn(std::ostream& os, const char* text);
  void SetIndent(const char* indent);

private:
  int TextWidth = 77;
  const char* TextIndent = "";
};
