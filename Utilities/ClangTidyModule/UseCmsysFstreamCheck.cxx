/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "UseCmsysFstreamCheck.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
namespace tidy {
namespace cmake {
using namespace ast_matchers;

UseCmsysFstreamCheck::UseCmsysFstreamCheck(StringRef Name,
                                           ClangTidyContext* Context)
  : ClangTidyCheck(Name, Context)
{
}

void UseCmsysFstreamCheck::registerMatchers(MatchFinder* Finder)
{
  this->createMatcher("::std::basic_ifstream", "::cmsys::ifstream", Finder,
                      "ifstream");
  this->createMatcher("::std::basic_ofstream", "::cmsys::ofstream", Finder,
                      "ofstream");
  this->createMatcher("::std::basic_fstream", "::cmsys::fstream", Finder,
                      "fstream");
}

void UseCmsysFstreamCheck::check(const MatchFinder::MatchResult& Result)
{
  const TypeLoc* ParentTypeNode =
    Result.Nodes.getNodeAs<TypeLoc>("parentType");
  const NestedNameSpecifierLoc* ParentNameNode =
    Result.Nodes.getNodeAs<NestedNameSpecifierLoc>("parentName");
  const TypeLoc* RootNode = nullptr;
  StringRef BindName;
  StringRef Warning;

  if ((RootNode = Result.Nodes.getNodeAs<TypeLoc>("ifstream")) != nullptr) {
    BindName = "cmsys::ifstream";
    Warning = "use cmsys::ifstream";
  } else if ((RootNode = Result.Nodes.getNodeAs<TypeLoc>("ofstream")) !=
             nullptr) {
    BindName = "cmsys::ofstream";
    Warning = "use cmsys::ofstream";
  } else if ((RootNode = Result.Nodes.getNodeAs<TypeLoc>("fstream")) !=
             nullptr) {
    BindName = "cmsys::fstream";
    Warning = "use cmsys::fstream";
  }

  if (ParentTypeNode != nullptr) {
    if (ParentTypeNode->getBeginLoc().isValid()) {
      this->diag(ParentTypeNode->getBeginLoc(), Warning)
        << FixItHint::CreateReplacement(ParentTypeNode->getSourceRange(),
                                        BindName);
    }
  } else if (ParentNameNode != nullptr) {
    if (ParentNameNode->getBeginLoc().isValid()) {
      this->diag(ParentNameNode->getBeginLoc(), Warning)
        << FixItHint::CreateReplacement(
             SourceRange(ParentNameNode->getBeginLoc(), RootNode->getEndLoc()),
             BindName);
    }
  } else if (RootNode != nullptr) {
    if (RootNode->getBeginLoc().isValid()) {
      this->diag(RootNode->getBeginLoc(), Warning)
        << FixItHint::CreateReplacement(RootNode->getSourceRange(), BindName);
    }
  }
}

void UseCmsysFstreamCheck::createMatcher(StringRef StdName,
                                         StringRef CmsysName,
                                         ast_matchers::MatchFinder* Finder,
                                         StringRef Bind)
{
  TypeLocMatcher IsStd = loc(qualType(hasUnqualifiedDesugaredType(
    recordType(hasDeclaration(classTemplateSpecializationDecl(
      hasName(StdName),
      hasTemplateArgument(
        0, templateArgument(refersToType(asString("char"))))))))));

  // TODO This only checks to see if the type directly refers to
  // cmsys::fstream. There are some corner cases involving template parameters
  // that refer to cmsys::fstream that are missed by this matcher, resulting in
  // a false positive. Figure out how to find these indirect references to
  // cmsys::fstream and filter them out. In the meantime, such false positives
  // can be silenced with NOLINT(cmake-use-cmsys-fstream).
  TypeLocMatcher IsCmsys =
    loc(usingType(throughUsingDecl(namedDecl(hasName(CmsysName)))));

  Finder->addMatcher(
    typeLoc(IsStd, unless(IsCmsys), unless(elaboratedTypeLoc()),
            optionally(hasParent(elaboratedTypeLoc().bind("parentType"))),
            optionally(hasParent(nestedNameSpecifierLoc().bind("parentName"))))
      .bind(Bind),
    this);
}
}
}
}
