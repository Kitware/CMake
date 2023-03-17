/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
  void check(const ast_matchers::MatchFinder::MatchResult& Result) override;

private:
  enum class OperatorType
  {
    Plus,
    PlusEquals
  };
  typedef std::pair<OperatorType, std::vector<const CXXOperatorCallExpr*>>
    ExprChain;
  std::map<const CXXOperatorCallExpr*, ExprChain> InProgressExprChains;

  void issueCorrection(const ExprChain& ExprChain,
                       const ast_matchers::MatchFinder::MatchResult& Result);
};
}
}
}
