/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDocumentationFormatter.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "cmDocumentationEntry.h"
#include "cmDocumentationSection.h"

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
                                              std::string const& text) const
{
  if (text.empty()) {
    return;
  }

  struct Buffer
  {
    // clang-format off
    using PrinterFn = void (cmDocumentationFormatter::*)(
        std::ostream&, std::string const&
      ) const;
    // clang-format on
    std::string collected;
    const PrinterFn printer;
  };
  // const auto NORMAL_IDX = 0u;
  const auto PREFORMATTED_IDX = 1u;
  const auto HANDLERS_SIZE = 2u;
  Buffer buffers[HANDLERS_SIZE] = {
    { {}, &cmDocumentationFormatter::PrintParagraph },
    { {}, &cmDocumentationFormatter::PrintPreformatted }
  };

  const auto padding = std::string(this->TextIndent, ' ');

  for (std::size_t pos = 0u, eol = 0u; pos < text.size(); pos = eol) {
    const auto current_idx = std::size_t(text[pos] == ' ');
    // size_t(!bool(current_idx))
    const auto other_idx = current_idx ^ 1u;

    // Flush the other buffer if anything has been collected
    if (!buffers[other_idx].collected.empty()) {
      // NOTE Whatever the other index is, the current buffered
      // string expected to be empty.
      assert(buffers[current_idx].collected.empty());

      (this->*buffers[other_idx].printer)(os, buffers[other_idx].collected);
      buffers[other_idx].collected.clear();
    }

    // ATTENTION The previous implementation had called `PrintParagraph()`
    // **for every processed (char by char) input line**.
    // The method unconditionally append the `\n' character after the
    // printed text. To keep the backward-compatible behavior it's needed to
    // add the '\n' character to the previously collected line...
    if (!buffers[current_idx].collected.empty() &&
        current_idx != PREFORMATTED_IDX) {
      buffers[current_idx].collected += '\n';
    }

    // Lookup EOL
    eol = text.find('\n', pos);
    if (current_idx == PREFORMATTED_IDX) {
      buffers[current_idx].collected.append(padding);
    }
    buffers[current_idx].collected.append(
      text, pos, eol == std::string::npos ? eol : ++eol - pos);
  }

  for (auto& buf : buffers) {
    if (!buf.collected.empty()) {
      (this->*buf.printer)(os, buf.collected);
    }
  }
}

void cmDocumentationFormatter::PrintPreformatted(std::ostream& os,
                                                 std::string const& text) const
{
  os << text << '\n';
}

void cmDocumentationFormatter::PrintParagraph(std::ostream& os,
                                              std::string const& text) const
{
  if (this->TextIndent) {
    os << std::string(this->TextIndent, ' ');
  }
  this->PrintColumn(os, text);
  os << '\n';
}

void cmDocumentationFormatter::PrintColumn(std::ostream& os,
                                           std::string const& text) const
{
  // Print text arranged in an indented column of fixed width.
  bool newSentence = false;
  bool firstLine = true;

  assert(this->TextIndent < this->TextWidth);
  const std::ptrdiff_t width = this->TextWidth - this->TextIndent;
  std::ptrdiff_t column = 0;

  // Loop until the end of the text.
  for (const char *l = text.c_str(), *r = skipToSpace(text.c_str()); *l;
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
      this->PrintColumn(os, entry.Brief);
      os << '\n';
    } else {
      os << '\n';
      this->TextIndent = 0u;
      this->PrintFormatted(os, entry.Brief);
    }
  }

  os << '\n';

  this->TextIndent = savedIndent;
}
