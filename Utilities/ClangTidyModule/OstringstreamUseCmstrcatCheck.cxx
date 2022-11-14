/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "OstringstreamUseCmstrcatCheck.h"

#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
using namespace ast_matchers;

OstringstreamUseCmstrcatCheck::OstringstreamUseCmstrcatCheck(
  StringRef Name, ClangTidyContext* Context)
  : ClangTidyCheck(Name, Context)
{
}

void OstringstreamUseCmstrcatCheck::registerMatchers(MatchFinder* Finder)
{
  Finder->addMatcher(
    typeLoc(unless(elaboratedTypeLoc()),
            optionally(hasParent(elaboratedTypeLoc().bind("parentType"))),
            loc(qualType(
              hasDeclaration(namedDecl(hasName("::std::ostringstream"))))))
      .bind("ostringstream"),
    this);
}

void OstringstreamUseCmstrcatCheck::check(
  const MatchFinder::MatchResult& Result)
{
  const TypeLoc* ParentTypeNode =
    Result.Nodes.getNodeAs<TypeLoc>("parentType");
  const TypeLoc* RootNode = Result.Nodes.getNodeAs<TypeLoc>("ostringstream");

  if (ParentTypeNode != nullptr) {
    if (ParentTypeNode->getBeginLoc().isValid()) {
      this->diag(ParentTypeNode->getBeginLoc(),
                 "use strings and cmStrCat() instead of std::ostringstream");
    }

  } else if (RootNode != nullptr) {
    if (RootNode->getBeginLoc().isValid()) {
      this->diag(RootNode->getBeginLoc(),
                 "use strings and cmStrCat() instead of std::ostringstream");
    }
  }
}
}
}
}
