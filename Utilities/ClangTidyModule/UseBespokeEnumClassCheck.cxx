/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "UseBespokeEnumClassCheck.h"

#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
using namespace ast_matchers;

UseBespokeEnumClassCheck::UseBespokeEnumClassCheck(StringRef Name,
                                                   ClangTidyContext* Context)
  : ClangTidyCheck(Name, Context)
{
}

void UseBespokeEnumClassCheck::registerMatchers(MatchFinder* Finder)
{
  Finder->addMatcher(
    parmVarDecl(
      hasTypeLoc(typeLoc(loc(qualType(asString("_Bool")))).bind("type"))),
    this);
}

void UseBespokeEnumClassCheck::check(const MatchFinder::MatchResult& Result)
{
  const TypeLoc* Node = Result.Nodes.getNodeAs<TypeLoc>("type");
  this->diag(Node->getBeginLoc(),
             "use a bespoke enum class instead of booleans as parameters");
}
}
}
}
