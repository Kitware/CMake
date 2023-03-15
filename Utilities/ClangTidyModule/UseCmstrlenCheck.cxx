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
                              callee(expr().bind("strlen")),
                              hasArgument(0, stringLiteral())),
                     this);

  auto IsSizeOfStringLiteral =
    unaryExprOrTypeTraitExpr(
      ofKind(UETT_SizeOf),
      anyOf(has(parenExpr(has(stringLiteral())).bind("paren")),
            has(stringLiteral())))
      .bind("sizeOf");
  Finder->addMatcher(
    binaryOperator(
      hasOperatorName("-"),
      hasLHS(anyOf(
        binaryOperator(hasOperatorName("+"), hasRHS(IsSizeOfStringLiteral)),
        IsSizeOfStringLiteral)),
      hasRHS(implicitCastExpr(has(integerLiteral(equals(1)).bind("literal")))))
      .bind("sizeOfMinus"),
    this);
}

void UseCmstrlenCheck::check(const MatchFinder::MatchResult& Result)
{
  const Expr* Strlen = Result.Nodes.getNodeAs<Expr>("strlen");
  const BinaryOperator* SizeOfMinus =
    Result.Nodes.getNodeAs<BinaryOperator>("sizeOfMinus");

  if (Strlen) {
    this->diag(Strlen->getBeginLoc(), "use cmStrLen() for string literals")
      << FixItHint::CreateReplacement(Strlen->getSourceRange(), "cmStrLen");
  }

  if (SizeOfMinus) {
    const ParenExpr* Paren = Result.Nodes.getNodeAs<ParenExpr>("paren");
    const UnaryExprOrTypeTraitExpr* SizeOf =
      Result.Nodes.getNodeAs<UnaryExprOrTypeTraitExpr>("sizeOf");
    const IntegerLiteral* Literal =
      Result.Nodes.getNodeAs<IntegerLiteral>("literal");

    std::vector<FixItHint> FixIts;
    if (Paren) {
      FixIts.push_back(
        FixItHint::CreateReplacement(SizeOf->getOperatorLoc(), "cmStrLen"));
      FixIts.push_back(FixItHint::CreateRemoval(
        SourceRange(SizeOfMinus->getOperatorLoc(), Literal->getLocation())));
    } else {
      FixIts.push_back(
        FixItHint::CreateReplacement(SizeOf->getOperatorLoc(), "cmStrLen("));
      FixIts.push_back(FixItHint::CreateReplacement(
        SourceRange(SizeOfMinus->getOperatorLoc(), Literal->getLocation()),
        ")"));
    }
    this->diag(SizeOf->getOperatorLoc(), "use cmStrLen() for string literals")
      << FixIts;
  }
}
}
}
}
