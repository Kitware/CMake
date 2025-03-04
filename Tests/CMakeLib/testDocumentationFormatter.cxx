/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <sstream>
#include <string>
#include <utility>

#include <cmDocumentationFormatter.h>

#include "testCommon.h"

namespace {
using TestCases = std::initializer_list<std::pair<std::string, std::string>>;

bool testPrintFormattedNoIndent()
{
  TestCases const testCases = {
    { "", "" },
    { "\n\n", "\n\n\n\n" },
    { "\n  \n\n", "\n\n  \n\n\n\n" },
    { "One line no EOL text", "One line no EOL text\n" },
    { "One line with trailing spaces and no EOL   ",
      "One line with trailing spaces and no EOL\n" },
    { "Short text. Two sentences.", "Short text.  Two sentences.\n" },
    { "Short text\non\nmultiple\nlines\n",
      "Short text\n\non\n\nmultiple\n\nlines\n\n" },
    { "Just one a very long word: "
      "01234567890123456789012345678901234567890123456789012345"
      "678901234567890123456789",
      "Just one a very long "
      "word:\n01234567890123456789012345678901234567890123456789012345"
      "678901234567890123456789\n" },
    { " Pre-formatted paragraph with the very long word stays the same: "
      "0123456789012345678901234567890123456789012345678901234567890123456789",
      " Pre-formatted paragraph with the very long word stays the same: "
      "0123456789012345678901234567890123456789012345678901234567890123456789"
      "\n" },
    { " Two pre-formatted\n paragraphs w/o EOL",
      " Two pre-formatted\n paragraphs w/o EOL\n" },
    { " Two pre-formatted\n paragraphs w/ EOL\n",
      " Two pre-formatted\n paragraphs w/ EOL\n\n" },
    { "Extra  spaces  are     removed.   However, \n   not in    a "
      "pre-formatted\n  "
      "paragraph",
      "Extra spaces are removed.  However,\n\n   not in    a pre-formatted\n  "
      "paragraph\n" },
    { "This is the text paragraph longer than a pre-defined wrapping position "
      "of the `cmDocumentationFormatter` class. And it's gonna be wrapped "
      "over multiple lines!",
      "This is the text paragraph longer than a pre-defined wrapping position "
      "of the\n`cmDocumentationFormatter` class.  And it's gonna be wrapped "
      "over multiple\nlines!\n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     ",
      "A normal paragraph, followed by ...\n\n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \nAnd another paragraph w/o EOL",
      "A normal paragraph, followed by ...\n\n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \n\nAnd another paragraph w/o EOL\n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \nAnd another paragraph w/ EOL\n",
      "A normal paragraph, followed by ...\n\n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \n\nAnd another paragraph w/ EOL\n\n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \nAnd another two\nparagraphs w/ EOL\n",
      "A normal paragraph, followed by ...\n\n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \n\nAnd another two\n\nparagraphs w/ "
      "EOL\n\n" }
  };

  cmDocumentationFormatter formatter;

  for (auto& test : testCases) {
    std::ostringstream out;
    formatter.PrintFormatted(out, test.first);
    auto actual = out.str();
    ASSERT_EQUAL(actual, test.second);
  }

  return true;
}

bool testPrintFormattedIndent2()
{
  TestCases const testCases = {
    { "", "" },
    // BEGIN NOTE Empty lines are not indented.
    { "\n\n", "\n\n\n\n" },
    { "\n  \n\n", "\n\n    \n\n\n\n" },
    // END NOTE
    { "One line no EOL text", "  One line no EOL text\n" },
    { "Short text. Two sentences.", "  Short text.  Two sentences.\n" },
    { "Short text\non\nmultiple\nlines\n",
      "  Short text\n\n  on\n\n  multiple\n\n  lines\n\n" },
    { "Just one a very long word: "
      "01234567890123456789012345678901234567890123456789012345"
      "678901234567890123456789",
      "  Just one a very long "
      "word:\n  01234567890123456789012345678901234567890123456789012345"
      "678901234567890123456789\n" },
    { " Pre-formatted paragraph with the very long word stays the same: "
      "0123456789012345678901234567890123456789012345678901234567890123456789",
      "   Pre-formatted paragraph with the very long word stays the same: "
      "0123456789012345678901234567890123456789012345678901234567890123456789"
      "\n" },
    { "Extra  spaces  are     removed.   However, \n  not in    a "
      "pre-formatted\n  "
      "paragraph",
      "  Extra spaces are removed.  However,\n\n    not in    a "
      "pre-formatted\n    "
      "paragraph\n" },
    { "This is the text paragraph longer than a pre-defined wrapping position "
      "of the `cmDocumentationFormatter` class. And it's gonna be wrapped "
      "over multiple lines!",
      "  This is the text paragraph longer than a pre-defined wrapping "
      "position of\n"
      "  the `cmDocumentationFormatter` class.  And it's gonna be wrapped "
      "over\n  multiple lines!\n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     ",
      "  A normal paragraph, followed by ...\n\n   Pre-formatted\n   "
      "paragraphs "
      "with trailing whitespaces     \n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \nAnd another paragraph w/o EOL",
      "  A normal paragraph, followed by ...\n\n   Pre-formatted\n   "
      "paragraphs with trailing whitespaces     \n\n  And another paragraph "
      "w/o EOL\n" },
    { "A normal paragraph,  followed by ...   \n Pre-formatted\n paragraphs "
      "with trailing whitespaces     \nAnd another paragraph w/ EOL\n",
      "  A normal paragraph, followed by ...\n\n   Pre-formatted\n   "
      "paragraphs with trailing whitespaces     \n\n  And another paragraph "
      "w/ EOL\n\n" }
  };

  cmDocumentationFormatter formatter;
  formatter.SetIndent(2);

  for (auto& test : testCases) {
    std::ostringstream out;
    formatter.PrintFormatted(out, test.first);
    auto actual = out.str();
    ASSERT_EQUAL(actual, test.second);
  }

  return true;
}

bool testPrintFormattedIndent10()
{
  TestCases const testCases = {
    { "", "" },
    { "One line no EOL text", "          One line no EOL text\n" },
    { "This is the text paragraph longer than a pre-defined wrapping position "
      "of the `cmDocumentationFormatter` class. And it's gonna be wrapped "
      "over multiple lines!",
      "          This is the text paragraph longer than a pre-defined "
      "wrapping\n"
      "          position of the `cmDocumentationFormatter` class.  "
      "And it's gonna\n"
      "          be wrapped over multiple lines!\n" }
  };

  cmDocumentationFormatter formatter;
  formatter.SetIndent(10);

  for (auto& test : testCases) {
    std::ostringstream out;
    formatter.PrintFormatted(out, test.first);
    auto actual = out.str();
    ASSERT_EQUAL(actual, test.second);
  }

  return true;
}
}

int testDocumentationFormatter(int /*unused*/, char* /*unused*/[])
{
  return runTests({ testPrintFormattedNoIndent, testPrintFormattedIndent2,
                    testPrintFormattedIndent10 },
                  false);
}
