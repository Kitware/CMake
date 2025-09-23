/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <clang-tidy/ClangTidyCheck.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
class StringConcatenationUseCmstrcatCheck : public ClangTidyCheck
{
public:
  StringConcatenationUseCmstrcatCheck(StringRef Name,
                                      ClangTidyContext* Context);
  void registerMatchers(ast_matchers::MatchFinder* Finder) override;
  void check(ast_matchers::MatchFinder::MatchResult const& Result) override;

private:
  enum class OperatorType
  {
    Plus,
    PlusEquals
  };
  typedef std::pair<OperatorType, std::vector<CXXOperatorCallExpr const*>>
    ExprChain;
  std::map<CXXOperatorCallExpr const*, ExprChain> InProgressExprChains;

  void issueCorrection(ExprChain const& ExprChain,
                       ast_matchers::MatchFinder::MatchResult const& Result);
};
}
}
}
