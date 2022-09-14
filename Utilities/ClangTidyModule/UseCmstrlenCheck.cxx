/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "UseCmstrlenCheck.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
using namespace ast_matchers;

UseCmstrlenCheck::UseCmstrlenCheck(StringRef Name, ClangTidyContext* Context)
  : ClangTidyCheck(Name, Context)
{
}

void UseCmstrlenCheck::registerMatchers(MatchFinder* Finder)
{
  Finder->addMatcher(callExpr(callee(functionDecl(hasName("::strlen"))),
                              callee(expr().bind("callee")),
                              hasArgument(0, stringLiteral())),
                     this);
}

void UseCmstrlenCheck::check(const MatchFinder::MatchResult& Result)
{
  const Expr* Node = Result.Nodes.getNodeAs<Expr>("callee");

  this->diag(Node->getBeginLoc(), "use cmStrLen() for string literals")
    << FixItHint::CreateReplacement(Node->getSourceRange(), "cmStrLen");
}
}
}
}
