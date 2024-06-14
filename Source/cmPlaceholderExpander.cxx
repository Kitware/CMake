/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPlaceholderExpander.h"

#include <cctype>

std::string& cmPlaceholderExpander::ExpandVariables(std::string& s)
{
  std::string::size_type start = s.find('<');
  // no variables to expand
  if (start == std::string::npos) {
    return s;
  }
  std::string::size_type pos = 0;
  std::string expandedInput;
  while (start != std::string::npos && start < s.size() - 2) {
    std::string::size_type end = s.find('>', start);
    // if we find a < with no > we are done
    if (end == std::string::npos) {
      s = expandedInput;
      return s;
    }
    char c = s[start + 1];
    // if the next char after the < is not A-Za-z then
    // skip it and try to find the next < in the string
    if (!isalpha(c)) {
      start = s.find('<', start + 1);
    } else {
      // extract the var
      std::string var = s.substr(start + 1, end - start - 1);
      std::string replace = this->ExpandVariable(var);
      expandedInput += s.substr(pos, start - pos);

      // Prevent consecutive whitespace in the output if the rule variable
      // expands to an empty string.
      bool consecutive = replace.empty() && start > 0 && s[start - 1] == ' ' &&
        end + 1 < s.size() && s[end + 1] == ' ';
      if (consecutive) {
        expandedInput.pop_back();
      }

      expandedInput += replace;

      // move to next one
      start = s.find('<', start + var.size() + 2);
      pos = end + 1;
    }
  }
  // add the rest of the input
  expandedInput += s.substr(pos, s.size() - pos);
  // remove trailing whitespace
  if (!expandedInput.empty() && expandedInput.back() == ' ') {
    expandedInput.pop_back();
  }
  s = expandedInput;

  return s;
}
