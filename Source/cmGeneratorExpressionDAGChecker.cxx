/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionDAGChecker.h"

#include <cstring>
#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

cmGeneratorExpressionDAGChecker::cmGeneratorExpressionDAGChecker(
  cmListFileBacktrace backtrace, cmGeneratorTarget const* target,
  std::string property, const GeneratorExpressionContent* content,
  cmGeneratorExpressionDAGChecker* parent)
  : Parent(parent)
  , Target(target)
  , Property(std::move(property))
  , Content(content)
  , Backtrace(std::move(backtrace))
  , TransitivePropertiesOnly(false)
{
  this->Initialize();
}

cmGeneratorExpressionDAGChecker::cmGeneratorExpressionDAGChecker(
  cmGeneratorTarget const* target, std::string property,
  const GeneratorExpressionContent* content,
  cmGeneratorExpressionDAGChecker* parent)
  : Parent(parent)
  , Target(target)
  , Property(std::move(property))
  , Content(content)
  , Backtrace()
  , TransitivePropertiesOnly(false)
{
  this->Initialize();
}

void cmGeneratorExpressionDAGChecker::Initialize()
{
  const auto* top = this->Top();
  this->CheckResult = this->CheckGraph();

#define TEST_TRANSITIVE_PROPERTY_METHOD(METHOD) top->METHOD() ||

  if (this->CheckResult == DAG &&
      (CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(
        TEST_TRANSITIVE_PROPERTY_METHOD) false)) // NOLINT(*)
#undef TEST_TRANSITIVE_PROPERTY_METHOD
  {
    auto it = top->Seen.find(this->Target);
    if (it != top->Seen.end()) {
      const std::set<std::string>& propSet = it->second;
      if (propSet.find(this->Property) != propSet.end()) {
        this->CheckResult = ALREADY_SEEN;
        return;
      }
    }
    top->Seen[this->Target].insert(this->Property);
  }
}

cmGeneratorExpressionDAGChecker::Result
cmGeneratorExpressionDAGChecker::Check() const
{
  return this->CheckResult;
}

void cmGeneratorExpressionDAGChecker::ReportError(
  cmGeneratorExpressionContext* context, const std::string& expr)
{
  if (this->CheckResult == DAG) {
    return;
  }

  context->HadError = true;
  if (context->Quiet) {
    return;
  }

  const cmGeneratorExpressionDAGChecker* parent = this->Parent;

  if (parent && !parent->Parent) {
    std::ostringstream e;
    e << "Error evaluating generator expression:\n"
      << "  " << expr << "\n"
      << "Self reference on target \"" << context->HeadTarget->GetName()
      << "\".\n";
    context->LG->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                                  e.str(), parent->Backtrace);
    return;
  }

  {
    std::ostringstream e;
    /* clang-format off */
  e << "Error evaluating generator expression:\n"
    << "  " << expr << "\n"
    << "Dependency loop found.";
    /* clang-format on */
    context->LG->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                                  e.str(), context->Backtrace);
  }

  int loopStep = 1;
  while (parent) {
    std::ostringstream e;
    e << "Loop step " << loopStep << "\n"
      << "  "
      << (parent->Content ? parent->Content->GetOriginalExpression() : expr)
      << "\n";
    context->LG->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                                  e.str(), parent->Backtrace);
    parent = parent->Parent;
    ++loopStep;
  }
}

cmGeneratorExpressionDAGChecker::Result
cmGeneratorExpressionDAGChecker::CheckGraph() const
{
  const cmGeneratorExpressionDAGChecker* parent = this->Parent;
  while (parent) {
    if (this->Target == parent->Target && this->Property == parent->Property) {
      return (parent == this->Parent) ? SELF_REFERENCE : CYCLIC_REFERENCE;
    }
    parent = parent->Parent;
  }
  return DAG;
}

bool cmGeneratorExpressionDAGChecker::GetTransitivePropertiesOnly() const
{
  return this->Top()->TransitivePropertiesOnly;
}

bool cmGeneratorExpressionDAGChecker::EvaluatingGenexExpression() const
{
  return cmHasLiteralPrefix(this->Property, "TARGET_GENEX_EVAL:") ||
    cmHasLiteralPrefix(this->Property, "GENEX_EVAL:");
}

bool cmGeneratorExpressionDAGChecker::EvaluatingPICExpression() const
{
  return this->Top()->Property == "INTERFACE_POSITION_INDEPENDENT_CODE";
}

bool cmGeneratorExpressionDAGChecker::EvaluatingCompileExpression() const
{
  cm::string_view property(this->Top()->Property);

  return property == "INCLUDE_DIRECTORIES"_s ||
    property == "COMPILE_DEFINITIONS"_s || property == "COMPILE_OPTIONS"_s;
}

bool cmGeneratorExpressionDAGChecker::EvaluatingLinkExpression() const
{
  cm::string_view property(this->Top()->Property);

  return property == "LINK_DIRECTORIES"_s || property == "LINK_OPTIONS"_s ||
    property == "LINK_DEPENDS"_s || property == "LINK_LIBRARY_OVERRIDE"_s;
}

bool cmGeneratorExpressionDAGChecker::EvaluatingLinkOptionsExpression() const
{
  cm::string_view property(this->Top()->Property);

  return property == "LINK_OPTIONS"_s;
}

bool cmGeneratorExpressionDAGChecker::EvaluatingLinkLibraries(
  cmGeneratorTarget const* tgt, ForGenex genex) const
{
  const auto* top = this->Top();

  cm::string_view prop(top->Property);

  if (tgt) {
    return top->Target == tgt && prop == "LINK_LIBRARIES"_s;
  }

  auto result = prop == "LINK_LIBRARIES"_s ||
    prop == "INTERFACE_LINK_LIBRARIES"_s ||
    prop == "INTERFACE_LINK_LIBRARIES_DIRECT"_s ||
    prop == "LINK_INTERFACE_LIBRARIES"_s ||
    prop == "IMPORTED_LINK_INTERFACE_LIBRARIES"_s ||
    cmHasLiteralPrefix(prop, "LINK_INTERFACE_LIBRARIES_") ||
    cmHasLiteralPrefix(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES_");

  return genex == ForGenex::LINK_LIBRARY || genex == ForGenex::LINK_GROUP
    ? result
    : (result || prop == "INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE"_s);
}

cmGeneratorExpressionDAGChecker const* cmGeneratorExpressionDAGChecker::Top()
  const
{
  const cmGeneratorExpressionDAGChecker* top = this;
  const cmGeneratorExpressionDAGChecker* parent = this->Parent;
  while (parent) {
    top = parent;
    parent = parent->Parent;
  }
  return top;
}

cmGeneratorTarget const* cmGeneratorExpressionDAGChecker::TopTarget() const
{
  return this->Top()->Target;
}

enum TransitiveProperty
{
#define DEFINE_ENUM_ENTRY(NAME) NAME,
  CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(DEFINE_ENUM_ENTRY)
#undef DEFINE_ENUM_ENTRY
    TransitivePropertyTerminal
};

template <TransitiveProperty>
bool additionalTest(const char* const /*unused*/)
{
  return false;
}

template <>
bool additionalTest<COMPILE_DEFINITIONS>(const char* const prop)
{
  return cmHasLiteralPrefix(prop, "COMPILE_DEFINITIONS_");
}

#define DEFINE_TRANSITIVE_PROPERTY_METHOD(METHOD, PROPERTY)                   \
  bool cmGeneratorExpressionDAGChecker::METHOD() const                        \
  {                                                                           \
    const char* const prop = this->Property.c_str();                          \
    if (strcmp(prop, #PROPERTY) == 0 ||                                       \
        strcmp(prop, "INTERFACE_" #PROPERTY) == 0) {                          \
      return true;                                                            \
    }                                                                         \
    return additionalTest<PROPERTY>(prop);                                    \
  }

CM_FOR_EACH_TRANSITIVE_PROPERTY(DEFINE_TRANSITIVE_PROPERTY_METHOD)

#undef DEFINE_TRANSITIVE_PROPERTY_METHOD
