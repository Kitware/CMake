/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <clang-tidy/ClangTidyCheck.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
class UseCmsysFstreamCheck : public ClangTidyCheck
{
public:
  UseCmsysFstreamCheck(StringRef Name, ClangTidyContext* Context);
  void registerMatchers(ast_matchers::MatchFinder* Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult& Result) override;

private:
  void createMatcher(StringRef name, StringRef CmsysName,
                     ast_matchers::MatchFinder* Finder, StringRef bind);
};
}
}
}
