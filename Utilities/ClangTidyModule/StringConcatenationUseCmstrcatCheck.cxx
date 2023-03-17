/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "StringConcatenationUseCmstrcatCheck.h"

#include <cassert>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Lex/Lexer.h>

namespace clang {
namespace tidy {
namespace cmake {
using namespace ast_matchers;

StringConcatenationUseCmstrcatCheck::StringConcatenationUseCmstrcatCheck(
  StringRef Name, ClangTidyContext* Context)
  : ClangTidyCheck(Name, Context)
{
}

void StringConcatenationUseCmstrcatCheck::registerMatchers(MatchFinder* Finder)
{
  auto IsString = expr(hasType(qualType(hasUnqualifiedDesugaredType(
    recordType(hasDeclaration(classTemplateSpecializationDecl(
      hasName("::std::basic_string"),
      hasTemplateArgument(
        0, templateArgument(refersToType(asString("char")))))))))));

  auto IsChar = expr(hasType(asString("char")));

  auto IsCharPtr = expr(hasType(pointerType(pointee(asString("const char")))));

  auto IsStringConcat =
    cxxOperatorCallExpr(hasOperatorName("+"),
                        anyOf(allOf(hasLHS(IsString), hasRHS(IsString)),
                              allOf(hasLHS(IsString), hasRHS(IsChar)),
                              allOf(hasLHS(IsString), hasRHS(IsCharPtr)),
                              allOf(hasLHS(IsChar), hasRHS(IsString)),
                              allOf(hasLHS(IsCharPtr), hasRHS(IsString))));

  auto IsStringAppend = cxxOperatorCallExpr(
    hasOperatorName("+="), hasLHS(IsString),
    anyOf(hasRHS(IsString), hasRHS(IsChar), hasRHS(IsCharPtr)));

  auto IsStringConcatWithLHS =
    cxxOperatorCallExpr(
      IsStringConcat,
      optionally(hasLHS(materializeTemporaryExpr(
        has(cxxBindTemporaryExpr(has(IsStringConcat.bind("lhs"))))))))
      .bind("concat");

  auto IsStringAppendWithRHS =
    cxxOperatorCallExpr(
      IsStringAppend,
      optionally(hasRHS(materializeTemporaryExpr(has(implicitCastExpr(
        has(cxxBindTemporaryExpr(has(IsStringConcat.bind("rhs"))))))))))
      .bind("append");

  Finder->addMatcher(IsStringConcatWithLHS, this);
  Finder->addMatcher(IsStringAppendWithRHS, this);
}

void StringConcatenationUseCmstrcatCheck::check(
  const MatchFinder::MatchResult& Result)
{
  const CXXOperatorCallExpr* AppendNode =
    Result.Nodes.getNodeAs<CXXOperatorCallExpr>("append");
  const CXXOperatorCallExpr* ConcatNode =
    Result.Nodes.getNodeAs<CXXOperatorCallExpr>("concat");

  if (AppendNode != nullptr) {
    if (AppendNode->getBeginLoc().isValid()) {
      assert(InProgressExprChains.find(AppendNode) ==
             InProgressExprChains.end());

      ExprChain TmpExprChain =
        std::make_pair(OperatorType::PlusEquals,
                       std::vector<const CXXOperatorCallExpr*>{ AppendNode });
      const CXXOperatorCallExpr* RHSNode =
        Result.Nodes.getNodeAs<CXXOperatorCallExpr>("rhs");

      if (RHSNode != nullptr) {
        if (RHSNode->getBeginLoc().isValid()) {
          InProgressExprChains[RHSNode] = std::move(TmpExprChain);
        }
      } else {
        issueCorrection(TmpExprChain, Result);
      }
    }
  }

  if (ConcatNode != nullptr) {
    if (ConcatNode->getBeginLoc().isValid()) {
      ExprChain TmpExprChain;

      if (!(InProgressExprChains.find(ConcatNode) ==
            InProgressExprChains.end())) {
        TmpExprChain = std::move(InProgressExprChains[ConcatNode]);
        InProgressExprChains.erase(ConcatNode);
        if (TmpExprChain.first == OperatorType::PlusEquals) {
          TmpExprChain.second.insert(TmpExprChain.second.begin() + 1,
                                     ConcatNode);
        } else {
          TmpExprChain.second.insert(TmpExprChain.second.begin(), ConcatNode);
        }
      } else {
        TmpExprChain = std::make_pair(
          OperatorType::Plus,
          std::vector<const CXXOperatorCallExpr*>{ ConcatNode });
      }

      const CXXOperatorCallExpr* LHSNode =
        Result.Nodes.getNodeAs<CXXOperatorCallExpr>("lhs");

      if (LHSNode != nullptr) {
        if (LHSNode->getBeginLoc().isValid()) {
          InProgressExprChains[LHSNode] = std::move(TmpExprChain);
        }
      } else {
        issueCorrection(TmpExprChain, Result);
      }
    }
  }
}

void StringConcatenationUseCmstrcatCheck::issueCorrection(
  const ExprChain& Chain, const MatchFinder::MatchResult& Result)
{
  std::vector<FixItHint> FixIts;
  const CXXOperatorCallExpr* ExprNode;
  std::vector<const clang::CXXOperatorCallExpr*>::const_iterator It =
    Chain.second.begin();

  if (Chain.first == OperatorType::PlusEquals) {
    ExprNode = *It;
    StringRef LHS = Lexer::getSourceText(
      CharSourceRange::getTokenRange(ExprNode->getArg(0)->getSourceRange()),
      Result.Context->getSourceManager(), Result.Context->getLangOpts());

    FixIts.push_back(FixItHint::CreateReplacement(
      ExprNode->getExprLoc(), "= cmStrCat(" + LHS.str() + ","));
    It++;
  } else {
    ExprNode = *It;
    FixIts.push_back(
      FixItHint::CreateInsertion(ExprNode->getBeginLoc(), "cmStrCat("));
  }

  while (It != std::end(Chain.second)) {
    ExprNode = *It;
    FixIts.push_back(
      FixItHint::CreateReplacement(ExprNode->getOperatorLoc(), ","));
    It++;
  }
  It--;
  ExprNode = *It;

  StringRef LastToken = Lexer::getSourceText(
    CharSourceRange::getTokenRange(
      ExprNode->getArg(1)->getSourceRange().getEnd()),
    Result.Context->getSourceManager(), Result.Context->getLangOpts());
  FixIts.push_back(FixItHint::CreateInsertion(
    ExprNode->getEndLoc().getLocWithOffset(LastToken.str().size()), ")"));

  It = Chain.second.begin();
  ExprNode = *It;

  if (Chain.first == OperatorType::PlusEquals) {
    this->diag(ExprNode->getOperatorLoc(),
               "use cmStrCat() instead of string append")
      << FixIts;
  } else {
    this->diag(ExprNode->getBeginLoc(),
               "use cmStrCat() instead of string concatenation")
      << FixIts;
  }
}
}
}
}
