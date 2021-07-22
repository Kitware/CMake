/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoUicHelpers.h"

cmQtAutoUicHelpers::cmQtAutoUicHelpers()
{
  RegExpInclude.compile("(^|\n)[ \t]*#[ \t]*include[ \t]+"
                        "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

void cmQtAutoUicHelpers::CollectUicIncludes(std::set<std::string>& includes,
                                            const std::string& content) const
{
  if (content.find("ui_") == std::string::npos) {
    return;
  }

  const char* contentChars = content.c_str();
  cmsys::RegularExpressionMatch match;
  while (this->RegExpInclude.find(contentChars, match)) {
    includes.emplace(match.match(2));
    // Forward content pointer
    contentChars += match.end();
  }
}
