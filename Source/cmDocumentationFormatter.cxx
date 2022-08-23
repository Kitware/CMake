/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDocumentationFormatter.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "cmDocumentationEntry.h"
#include "cmDocumentationSection.h"
#include "cmSystemTools.h"

namespace {
const char* skipSpaces(const char* ptr)
{
  assert(ptr);
  for (; *ptr == ' '; ++ptr) {
    ;
  }
  return ptr;
}
const char* skipToSpace(const char* ptr)
{
  assert(ptr);
  for (; *ptr && (*ptr != '\n') && (*ptr != ' '); ++ptr) {
    ;
  }
  return ptr;
}
}

void cmDocumentationFormatter::PrintFormatted(std::ostream& os,
                                              const char* text)
{
  if (!text) {
    return;
  }

  for (const char* ptr = text; *ptr;) {
    // Any ptrs starting in a space are treated as preformatted text.
    std::string preformatted;
    while (*ptr == ' ') {
      for (char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr) {
        preformatted.append(1, ch);
      }
      if (*ptr) {
        ++ptr;
        preformatted.append(1, '\n');
      }
    }
    if (!preformatted.empty()) {
      this->PrintPreformatted(os, preformatted);
    }

    // Other ptrs are treated as paragraphs.
    std::string paragraph;
    for (char ch = *ptr; ch && ch != '\n'; ++ptr, ch = *ptr) {
      paragraph.append(1, ch);
    }
    if (*ptr) {
      ++ptr;
      paragraph.append(1, '\n');
    }
    if (!paragraph.empty()) {
      this->PrintParagraph(os, paragraph.c_str());
    }
  }
}

void cmDocumentationFormatter::PrintPreformatted(std::ostream& os,
                                                 std::string const& text) const
{
  if (this->TextIndent) {
    auto indented = text;
    auto padding = std::string(this->TextIndent, ' ');
    cmSystemTools::ReplaceString(indented, "\n", "\n" + padding);
    indented = std::move(padding) + indented;
    os << indented << '\n';
  } else {
    os << text << '\n';
  }
}

void cmDocumentationFormatter::PrintParagraph(std::ostream& os,
                                              const char* text)
{
  if (this->TextIndent) {
    os << std::string(this->TextIndent, ' ');
  }
  this->PrintColumn(os, text);
  os << '\n';
}

void cmDocumentationFormatter::PrintColumn(std::ostream& os, const char* text)
{
  // Print text arranged in an indented column of fixed width.
  bool newSentence = false;
  bool firstLine = true;

  assert(this->TextIndent < this->TextWidth);
  const std::ptrdiff_t width = this->TextWidth - this->TextIndent;
  std::ptrdiff_t column = 0;

  // Loop until the end of the text.
  for (const char *l = text, *r = skipToSpace(text); *l;
       l = skipSpaces(r), r = skipToSpace(l)) {
    // Does it fit on this line?
    if (r - l < width - column - std::ptrdiff_t(newSentence)) {
      // Word fits on this line.
      if (r > l) {
        if (column) {
          // Not first word on line.  Separate from the previous word
          // by a space, or two if this is a new sentence.
          os << &("  "[std::size_t(!newSentence)]);
          column += 1u + std::ptrdiff_t(newSentence);
        } else if (!firstLine && this->TextIndent) {
          // First word on line.  Print indentation unless this is the
          // first line.
          os << std::string(this->TextIndent, ' ');
        }

        // Print the word.
        os.write(l, r - l);
        newSentence = (*(r - 1) == '.');
      }

      if (*r == '\n') {
        // Text provided a newline.  Start a new line.
        os << '\n';
        ++r;
        column = 0;
        firstLine = false;
      } else {
        // No provided newline.  Continue this line.
        column += r - l;
      }
    } else {
      // Word does not fit on this line.  Start a new line.
      os << '\n';
      firstLine = false;
      if (r > l) {
        os << std::string(this->TextIndent, ' ');
        os.write(l, r - l);
        column = r - l;
        newSentence = (*(r - 1) == '.');
      } else {
        column = 0;
      }
    }
    // Move to beginning of next word.  Skip over whitespace.
  }
}

void cmDocumentationFormatter::PrintSection(
  std::ostream& os, cmDocumentationSection const& section)
{
  const std::size_t PREFIX_SIZE =
    sizeof(cmDocumentationEntry::CustomNamePrefix) + 1u;
  // length of the "= " literal (see below)
  const std::size_t SUFFIX_SIZE = 2u;
  // legacy magic number ;-)
  const std::size_t NAME_SIZE = 29u;

  const std::size_t PADDING_SIZE = PREFIX_SIZE + SUFFIX_SIZE;
  const std::size_t TITLE_SIZE = NAME_SIZE + PADDING_SIZE;

  const auto savedIndent = this->TextIndent;

  os << section.GetName() << '\n';

  for (cmDocumentationEntry const& entry : section.GetEntries()) {
    if (!entry.Name.empty()) {
      this->TextIndent = TITLE_SIZE;
      os << std::setw(PREFIX_SIZE) << std::left << entry.CustomNamePrefix
         << std::setw(int(std::max(NAME_SIZE, entry.Name.size())))
         << entry.Name;
      if (entry.Name.size() > NAME_SIZE) {
        os << '\n' << std::setw(int(this->TextIndent - PREFIX_SIZE)) << ' ';
      }
      os << "= ";
      this->PrintColumn(os, entry.Brief.c_str());
      os << '\n';
    } else {
      os << '\n';
      this->TextIndent = 0u;
      this->PrintFormatted(os, entry.Brief.c_str());
    }
  }

  os << '\n';

  this->TextIndent = savedIndent;
}
