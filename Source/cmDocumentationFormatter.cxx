/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDocumentationFormatter.h"

#include <algorithm> // IWYU pragma: keep
#include <cassert>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmDocumentationEntry.h"
#include "cmDocumentationSection.h"
#include "cmStringAlgorithms.h"

namespace {
auto const EOL = "\n"_s;
auto const SPACE = " "_s;
auto const TWO_SPACES = "  "_s;
auto const MAX_WIDTH_PADDING =
  std::string(cmDocumentationFormatter::TEXT_WIDTH, ' ');

void FormatLine(std::back_insert_iterator<std::vector<cm::string_view>> outIt,
                cm::string_view const text, cm::string_view const padding)
{
  auto tokens = cmTokenizedView(text, ' ', cmTokenizerMode::New);
  if (tokens.empty()) {
    return;
  }

  // Push padding in front of a first line
  if (!padding.empty()) {
    outIt = padding;
  }

  auto currentWidth = padding.size();
  auto newSentence = false;

  for (auto token : tokens) {
    // It's no need to add a space if this is a very first
    // word on a line.
    auto const needSpace = currentWidth > padding.size();
    // Evaluate the size of a current token + possibly spaces before it.
    auto const tokenWithSpaceSize = token.size() + std::size_t(needSpace) +
      std::size_t(needSpace && newSentence);
    // Check if a current word fits on a line.
    // Also, take in account:
    //  - extra space if not a first word on a line
    //  - extra space if last token ends w/ a period
    if (currentWidth + tokenWithSpaceSize <=
        cmDocumentationFormatter::TEXT_WIDTH) {
      // If not a first word on a line...
      if (needSpace) {
        // ... add a space after the last token +
        // possibly one more space if the last token
        // ends with a period (means, end of a sentence).
        outIt = newSentence ? TWO_SPACES : SPACE;
      }
      outIt = token;
      currentWidth += tokenWithSpaceSize;
    } else {
      // Start a new line!
      outIt = EOL;
      if (!padding.empty()) {
        outIt = padding;
      }
      outIt = token;
      currentWidth = padding.size() + token.size();
    }

    // Start a new sentence if the current word ends with period
    newSentence = token.back() == '.';
  }
  // Always add EOL at the end of formatted text
  outIt = EOL;
}
} // anonymous namespace

std::string cmDocumentationFormatter::Format(std::string text) const
{
  // Exit early on empty text
  if (text.empty()) {
    return {};
  }

  assert(this->TextIndent < this->TEXT_WIDTH);

  auto const padding =
    cm::string_view(MAX_WIDTH_PADDING.c_str(), this->TextIndent);

  std::vector<cm::string_view> tokens;
  auto outIt = std::back_inserter(tokens);
  auto prevWasPreFormatted = false;

  // NOTE Can't use `cmTokenizedView()` cuz every sequential EOL does matter
  // (and `cmTokenizedView()` will squeeze 'em)
  for ( // clang-format off
      std::string::size_type start = 0
    , end = text.find('\n')
    ; start < text.size()
    ; start = end + ((end != std::string::npos) ? 1 : 0)
    , end = text.find('\n', start)
    ) // clang-format on
  {
    auto const isLastLine = end == std::string::npos;
    auto const line = isLastLine
      ? cm::string_view{ text.c_str() + start }
      : cm::string_view{ text.c_str() + start, end - start };

    if (!line.empty() && line.front() == ' ') {
      // Preformatted lines go as is w/ a leading padding
      if (!padding.empty()) {
        outIt = padding;
      }
      outIt = line;
      prevWasPreFormatted = true;
    } else {
      // Separate a normal paragraph from a pre-formatted
      // w/ an extra EOL
      if (prevWasPreFormatted) {
        outIt = EOL;
      }
      if (line.empty()) {
        if (!isLastLine) {
          outIt = EOL;
        }
      } else {
        FormatLine(outIt, line, padding);
      }
      prevWasPreFormatted = false;
    }
    if (!isLastLine) {
      outIt = EOL;
    }
  }

  if (prevWasPreFormatted) {
    outIt = EOL;
  }

  return cmJoinStrings(tokens, {}, {});
}

void cmDocumentationFormatter::PrintSection(
  std::ostream& os, cmDocumentationSection const& section)
{
  std::size_t const PREFIX_SIZE =
    sizeof(cmDocumentationEntry::CustomNamePrefix) + 1u;
  // length of the "= " literal (see below)
  std::size_t const SUFFIX_SIZE = 2u;
  // legacy magic number ;-)
  std::size_t const NAME_SIZE = 29u;

  std::size_t const PADDING_SIZE = PREFIX_SIZE + SUFFIX_SIZE;
  std::size_t const TITLE_SIZE = NAME_SIZE + PADDING_SIZE;

  auto const savedIndent = this->TextIndent;

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
      os << "= " << this->Format(entry.Brief).substr(this->TextIndent);
    } else {
      this->TextIndent = 0u;
      os << '\n' << this->Format(entry.Brief);
    }
  }

  os << '\n';

  this->TextIndent = savedIndent;
}
