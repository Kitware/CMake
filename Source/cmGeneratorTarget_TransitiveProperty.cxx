/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionNode.h"
#include "cmLinkItem.h"
#include "cmLocalGenerator.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

namespace {
using UseTo = cmGeneratorTarget::UseTo;
using TransitiveProperty = cmGeneratorTarget::TransitiveProperty;
}

const std::map<cm::string_view, TransitiveProperty>
  cmGeneratorTarget::BuiltinTransitiveProperties = {
    { "AUTOMOC_MACRO_NAMES"_s,
      { "INTERFACE_AUTOMOC_MACRO_NAMES"_s, UseTo::Compile } },
    { "AUTOUIC_OPTIONS"_s, { "INTERFACE_AUTOUIC_OPTIONS"_s, UseTo::Compile } },
    { "COMPILE_DEFINITIONS"_s,
      { "INTERFACE_COMPILE_DEFINITIONS"_s, UseTo::Compile } },
    { "COMPILE_FEATURES"_s,
      { "INTERFACE_COMPILE_FEATURES"_s, UseTo::Compile } },
    { "COMPILE_OPTIONS"_s, { "INTERFACE_COMPILE_OPTIONS"_s, UseTo::Compile } },
    { "INCLUDE_DIRECTORIES"_s,
      { "INTERFACE_INCLUDE_DIRECTORIES"_s, UseTo::Compile } },
    { "LINK_DEPENDS"_s, { "INTERFACE_LINK_DEPENDS"_s, UseTo::Link } },
    { "LINK_DIRECTORIES"_s, { "INTERFACE_LINK_DIRECTORIES"_s, UseTo::Link } },
    { "LINK_OPTIONS"_s, { "INTERFACE_LINK_OPTIONS"_s, UseTo::Link } },
    { "PRECOMPILE_HEADERS"_s,
      { "INTERFACE_PRECOMPILE_HEADERS"_s, UseTo::Compile } },
    { "SOURCES"_s, { "INTERFACE_SOURCES"_s, UseTo::Compile } },
    { "SYSTEM_INCLUDE_DIRECTORIES"_s,
      { "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES"_s, UseTo::Compile } },
  };

bool cmGeneratorTarget::MaybeHaveInterfaceProperty(
  std::string const& prop, cmGeneratorExpressionContext* context,
  UseTo usage) const
{
  std::string const key = prop + '@' + context->Config;
  auto i = this->MaybeInterfacePropertyExists.find(key);
  if (i == this->MaybeInterfacePropertyExists.end()) {
    // Insert an entry now in case there is a cycle.
    i = this->MaybeInterfacePropertyExists.emplace(key, false).first;
    bool& maybeInterfaceProp = i->second;

    // If this target itself has a non-empty property value, we are done.
    maybeInterfaceProp = cmNonempty(this->GetProperty(prop));

    // Otherwise, recurse to interface dependencies.
    if (!maybeInterfaceProp) {
      cmGeneratorTarget const* headTarget =
        context->HeadTarget ? context->HeadTarget : this;
      if (cmLinkInterfaceLibraries const* iface =
            this->GetLinkInterfaceLibraries(context->Config, headTarget,
                                            usage)) {
        if (iface->HadHeadSensitiveCondition) {
          // With a different head target we may get to a library with
          // this interface property.
          maybeInterfaceProp = true;
        } else {
          // The transitive interface libraries do not depend on the
          // head target, so we can follow them.
          for (cmLinkItem const& lib : iface->Libraries) {
            if (lib.Target &&
                lib.Target->MaybeHaveInterfaceProperty(prop, context, usage)) {
              maybeInterfaceProp = true;
              break;
            }
          }
        }
      }
    }
  }
  return i->second;
}

std::string cmGeneratorTarget::EvaluateInterfaceProperty(
  std::string const& prop, cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagCheckerParent, UseTo usage) const
{
  std::string result;

  // If the property does not appear transitively at all, we are done.
  if (!this->MaybeHaveInterfaceProperty(prop, context, usage)) {
    return result;
  }

  // Evaluate $<TARGET_PROPERTY:this,prop> as if it were compiled.  This is
  // a subset of TargetPropertyNode::Evaluate without stringify/parse steps
  // but sufficient for transitive interface properties.
  cmGeneratorExpressionDAGChecker dagChecker(context->Backtrace, this, prop,
                                             nullptr, dagCheckerParent,
                                             this->LocalGenerator);
  switch (dagChecker.Check()) {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dagChecker.ReportError(
        context, "$<TARGET_PROPERTY:" + this->GetName() + "," + prop + ">");
      return result;
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return result;
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  cmGeneratorTarget const* headTarget =
    context->HeadTarget ? context->HeadTarget : this;

  if (cmValue p = this->GetProperty(prop)) {
    result = cmGeneratorExpressionNode::EvaluateDependentExpression(
      *p, context->LG, context, headTarget, &dagChecker, this);
  }

  if (cmLinkInterfaceLibraries const* iface =
        this->GetLinkInterfaceLibraries(context->Config, headTarget, usage)) {
    context->HadContextSensitiveCondition =
      context->HadContextSensitiveCondition ||
      iface->HadContextSensitiveCondition;
    for (cmLinkItem const& lib : iface->Libraries) {
      // Broken code can have a target in its own link interface.
      // Don't follow such link interface entries so as not to create a
      // self-referencing loop.
      if (lib.Target && lib.Target != this) {
        // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in the
        // above property and hand-evaluate it as if it were compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cmGeneratorExpressionContext libContext(
          context->LG, context->Config, context->Quiet, headTarget, this,
          context->EvaluateForBuildsystem, context->Backtrace,
          context->Language);
        std::string libResult = cmGeneratorExpression::StripEmptyListElements(
          lib.Target->EvaluateInterfaceProperty(prop, &libContext, &dagChecker,
                                                usage));
        if (!libResult.empty()) {
          if (result.empty()) {
            result = std::move(libResult);
          } else {
            result.reserve(result.size() + 1 + libResult.size());
            result += ";";
            result += libResult;
          }
        }
        context->HadContextSensitiveCondition =
          context->HadContextSensitiveCondition ||
          libContext.HadContextSensitiveCondition;
        context->HadHeadSensitiveCondition =
          context->HadHeadSensitiveCondition ||
          libContext.HadHeadSensitiveCondition;
      }
    }
  }

  return result;
}

cm::optional<cmGeneratorTarget::TransitiveProperty>
cmGeneratorTarget::IsTransitiveProperty(cm::string_view prop,
                                        cmLocalGenerator const* lg) const
{
  cm::optional<TransitiveProperty> result;
  static const cm::string_view kINTERFACE_ = "INTERFACE_"_s;
  if (cmHasPrefix(prop, kINTERFACE_)) {
    prop = prop.substr(kINTERFACE_.length());
  }
  auto i = BuiltinTransitiveProperties.find(prop);
  if (i != BuiltinTransitiveProperties.end()) {
    result = i->second;
    if (result->Usage != cmGeneratorTarget::UseTo::Compile) {
      cmPolicies::PolicyStatus cmp0166 =
        lg->GetPolicyStatus(cmPolicies::CMP0166);
      if ((cmp0166 == cmPolicies::WARN || cmp0166 == cmPolicies::OLD) &&
          (prop == "LINK_DIRECTORIES"_s || prop == "LINK_DEPENDS"_s ||
           prop == "LINK_OPTIONS"_s)) {
        result->Usage = cmGeneratorTarget::UseTo::Compile;
      }
    }
  } else if (cmHasLiteralPrefix(prop, "COMPILE_DEFINITIONS_")) {
    cmPolicies::PolicyStatus cmp0043 =
      lg->GetPolicyStatus(cmPolicies::CMP0043);
    if (cmp0043 == cmPolicies::WARN || cmp0043 == cmPolicies::OLD) {
      result = TransitiveProperty{ "INTERFACE_COMPILE_DEFINITIONS"_s,
                                   UseTo::Compile };
    }
  }
  return result;
}
