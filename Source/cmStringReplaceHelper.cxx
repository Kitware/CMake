/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmStringReplaceHelper.h"

#include <sstream>
#include <utility>

#include "cmMakefile.h"
#include "cmPolicies.h"

cmStringReplaceHelper::cmStringReplaceHelper(std::string const& regex,
                                             std::string replace_expr,
                                             cmMakefile* makefile)
  : RegExString(regex)
  , RegularExpression(regex)
  , ReplaceExpression(std::move(replace_expr))
  , Makefile(makefile)
{
  this->ParseReplaceExpression();
}

bool cmStringReplaceHelper::Replace(std::string const& input,
                                    std::string& output)
{
  output.clear();

  unsigned optAnchor = 0;
  if (this->Makefile &&
      this->Makefile->GetPolicyStatus(cmPolicies::CMP0186) !=
        cmPolicies::NEW) {
    optAnchor = cmsys::RegularExpression::BOL_AT_OFFSET;
  }

  // Scan through the input for all matches.
  auto& re = this->RegularExpression;
  std::string::size_type base = 0;
  unsigned optNonEmpty = 0;
  while (re.find(input, base, optAnchor | optNonEmpty)) {
    if (this->Makefile) {
      this->Makefile->ClearMatches();
      this->Makefile->StoreMatches(re);
    }

    // Concatenate the part of the input that was not matched.
    output += input.substr(base, re.start() - base);

    // Concatenate the replacement for the match.
    for (auto const& replacement : this->Replacements) {
      if (replacement.Number < 0) {
        // This is just a plain-text part of the replacement.
        output += replacement.Value;
      } else {
        // Replace with part of the match.
        auto n = replacement.Number;
        if (n > re.num_groups()) {
          std::ostringstream error;
          error << "replace expression \"" << this->ReplaceExpression
                << "\" contains an out-of-range escape for regex \""
                << this->RegExString << "\"";
          this->ErrorString = error.str();
          return false;
        }
        output += re.match(n);
      }
    }

    // Move past the match.
    base = re.end();

    if (re.start() == input.length()) {
      break;
    }
    if (re.start() == re.end()) {
      optNonEmpty = cmsys::RegularExpression::NONEMPTY_AT_OFFSET;
    } else {
      optNonEmpty = 0;
    }
  }

  // Concatenate the text after the last match.
  output += input.substr(base);

  return true;
}

void cmStringReplaceHelper::ParseReplaceExpression()
{
  std::string::size_type l = 0;
  while (l < this->ReplaceExpression.length()) {
    auto r = this->ReplaceExpression.find('\\', l);
    if (r == std::string::npos) {
      r = this->ReplaceExpression.length();
      this->Replacements.emplace_back(
        this->ReplaceExpression.substr(l, r - l));
    } else {
      if (r - l > 0) {
        this->Replacements.emplace_back(
          this->ReplaceExpression.substr(l, r - l));
      }
      if (r == (this->ReplaceExpression.length() - 1)) {
        this->ValidReplaceExpression = false;
        this->ErrorString = "replace-expression ends in a backslash";
        return;
      }
      if ((this->ReplaceExpression[r + 1] >= '0') &&
          (this->ReplaceExpression[r + 1] <= '9')) {
        this->Replacements.emplace_back(this->ReplaceExpression[r + 1] - '0');
      } else if (this->ReplaceExpression[r + 1] == 'n') {
        this->Replacements.emplace_back("\n");
      } else if (this->ReplaceExpression[r + 1] == '\\') {
        this->Replacements.emplace_back("\\");
      } else {
        this->ValidReplaceExpression = false;
        std::ostringstream error;
        error << "Unknown escape \"" << this->ReplaceExpression.substr(r, 2)
              << "\" in replace-expression";
        this->ErrorString = error.str();
        return;
      }
      r += 2;
    }
    l = r;
  }
}
