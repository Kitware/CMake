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

void cmDocumentationFormatter::PrintFormatted(std::ostream& os,
                                              const char* text)
{
  if (!text) {
    return;
  }
  const char* ptr = text;
  while (*ptr) {
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
  const char* l = text;
  long column = 0;
  bool newSentence = false;
  bool firstLine = true;
  int width = this->TextWidth - static_cast<int>(this->TextIndent);

  // Loop until the end of the text.
  while (*l) {
    // Parse the next word.
    const char* r = l;
    while (*r && (*r != '\n') && (*r != ' ')) {
      ++r;
    }

    // Does it fit on this line?
    if (r - l < (width - column - (newSentence ? 1 : 0))) {
      // Word fits on this line.
      if (r > l) {
        if (column) {
          // Not first word on line.  Separate from the previous word
          // by a space, or two if this is a new sentence.
          if (newSentence) {
            os << "  ";
            column += 2;
          } else {
            os << ' ';
            column += 1;
          }
        } else if (!firstLine && this->TextIndent) {
          // First word on line.  Print indentation unless this is the
          // first line.
          os << std::string(this->TextIndent, ' ');
        }

        // Print the word.
        os.write(l, static_cast<long>(r - l));
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
        column += static_cast<long>(r - l);
      }
    } else {
      // Word does not fit on this line.  Start a new line.
      os << '\n';
      firstLine = false;
      if (r > l) {
        os << std::string(this->TextIndent, ' ');
        os.write(l, static_cast<long>(r - l));
        column = static_cast<long>(r - l);
        newSentence = (*(r - 1) == '.');
      } else {
        column = 0;
      }
    }

    // Move to beginning of next word.  Skip over whitespace.
    l = r;
    while (*l == ' ') {
      ++l;
    }
  }
}

void cmDocumentationFormatter::PrintSection(
  std::ostream& os, cmDocumentationSection const& section)
{
  os << section.GetName() << '\n';

  const std::size_t PREFIX_SIZE =
    sizeof(cmDocumentationEntry::CustomNamePrefix) + 1u;
  // length of the "= " literal (see below)
  const std::size_t SUFFIX_SIZE = 2u;
  // legacy magic number ;-)
  const std::size_t NAME_SIZE = 29u;

  const std::size_t PADDING_SIZE = PREFIX_SIZE + SUFFIX_SIZE;
  const std::size_t TITLE_SIZE = NAME_SIZE + PADDING_SIZE;

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
}
