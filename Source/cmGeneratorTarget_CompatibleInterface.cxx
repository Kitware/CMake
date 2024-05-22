/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {
using UseTo = cmGeneratorTarget::UseTo;
}

const cmGeneratorTarget::CompatibleInterfacesBase&
cmGeneratorTarget::GetCompatibleInterfaces(std::string const& config) const
{
  cmGeneratorTarget::CompatibleInterfaces& compat =
    this->CompatibleInterfacesMap[config];
  if (!compat.Done) {
    compat.Done = true;
    compat.PropsBool.insert("POSITION_INDEPENDENT_CODE");
    compat.PropsString.insert("AUTOUIC_OPTIONS");
    std::vector<cmGeneratorTarget const*> const& deps =
      this->GetLinkImplementationClosure(config, UseTo::Compile);
    for (cmGeneratorTarget const* li : deps) {
#define CM_READ_COMPATIBLE_INTERFACE(X, x)                                    \
  if (cmValue prop = li->GetProperty("COMPATIBLE_INTERFACE_" #X)) {           \
    cmList props(*prop);                                                      \
    compat.Props##x.insert(props.begin(), props.end());                       \
  }
      CM_READ_COMPATIBLE_INTERFACE(BOOL, Bool)
      CM_READ_COMPATIBLE_INTERFACE(STRING, String)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MIN, NumberMin)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MAX, NumberMax)
#undef CM_READ_COMPATIBLE_INTERFACE
    }
  }
  return compat;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentBoolProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsBool.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentStringProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsString.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMinProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsNumberMin.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMaxProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsNumberMax.count(p) > 0;
}

enum CompatibleType
{
  BoolType,
  StringType,
  NumberMinType,
  NumberMaxType
};

template <typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string& prop,
                                               const std::string& config,
                                               CompatibleType, PropertyType*);

template <>
bool getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                       const std::string& prop,
                                       const std::string& config,
                                       CompatibleType /*unused*/,
                                       bool* /*unused*/)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template <>
const char* getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                              const std::string& prop,
                                              const std::string& config,
                                              CompatibleType t,
                                              const char** /*unused*/)
{
  switch (t) {
    case BoolType:
      assert(false &&
             "String compatibility check function called for boolean");
      return nullptr;
    case StringType:
      return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
    case NumberMinType:
      return tgt->GetLinkInterfaceDependentNumberMinProperty(prop, config);
    case NumberMaxType:
      return tgt->GetLinkInterfaceDependentNumberMaxProperty(prop, config);
  }
  assert(false && "Unreachable!");
  return nullptr;
}

template <typename PropertyType>
void checkPropertyConsistency(cmGeneratorTarget const* depender,
                              cmGeneratorTarget const* dependee,
                              const std::string& propName,
                              std::set<std::string>& emitted,
                              const std::string& config, CompatibleType t,
                              PropertyType* /*unused*/)
{
  cmValue prop = dependee->GetProperty(propName);
  if (!prop) {
    return;
  }

  cmList props{ *prop };
  std::string pdir =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Help/prop_tgt/");

  for (std::string const& p : props) {
    std::string pname = cmSystemTools::HelpFileName(p);
    std::string pfile = pdir + pname + ".rst";
    if (cmSystemTools::FileExists(pfile, true)) {
      std::ostringstream e;
      e << "Target \"" << dependee->GetName() << "\" has property \"" << p
        << "\" listed in its " << propName
        << " property.  "
           "This is not allowed.  Only user-defined properties may appear "
           "listed in the "
        << propName << " property.";
      depender->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR,
                                                  e.str());
      return;
    }
    if (emitted.insert(p).second) {
      getLinkInterfaceDependentProperty<PropertyType>(depender, p, config, t,
                                                      nullptr);
      if (cmSystemTools::GetErrorOccurredFlag()) {
        return;
      }
    }
  }
}

namespace {
std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2)
{
  std::set<std::string> intersect;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(intersect, intersect.begin()));
  if (!intersect.empty()) {
    return *intersect.begin();
  }
  return "";
}

std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2,
                      const std::set<std::string>& s3)
{
  std::string result;
  result = intersect(s1, s2);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s3);
  if (!result.empty()) {
    return result;
  }
  return intersect(s2, s3);
}

std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2,
                      const std::set<std::string>& s3,
                      const std::set<std::string>& s4)
{
  std::string result;
  result = intersect(s1, s2);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s3);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s4);
  if (!result.empty()) {
    return result;
  }
  return intersect(s2, s3, s4);
}
}

void cmGeneratorTarget::CheckPropertyCompatibility(
  cmComputeLinkInformation& info, const std::string& config) const
{
  const cmComputeLinkInformation::ItemVector& deps = info.GetItems();

  std::set<std::string> emittedBools;
  static const std::string strBool = "COMPATIBLE_INTERFACE_BOOL";
  std::set<std::string> emittedStrings;
  static const std::string strString = "COMPATIBLE_INTERFACE_STRING";
  std::set<std::string> emittedMinNumbers;
  static const std::string strNumMin = "COMPATIBLE_INTERFACE_NUMBER_MIN";
  std::set<std::string> emittedMaxNumbers;
  static const std::string strNumMax = "COMPATIBLE_INTERFACE_NUMBER_MAX";

  for (auto const& dep : deps) {
    if (!dep.Target || dep.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      continue;
    }

    checkPropertyConsistency<bool>(this, dep.Target, strBool, emittedBools,
                                   config, BoolType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strString,
                                          emittedStrings, config, StringType,
                                          nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMin,
                                          emittedMinNumbers, config,
                                          NumberMinType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMax,
                                          emittedMaxNumbers, config,
                                          NumberMaxType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
  }

  std::string prop = intersect(emittedBools, emittedStrings, emittedMinNumbers,
                               emittedMaxNumbers);

  if (!prop.empty()) {
    // Use a sorted std::vector to keep the error message sorted.
    std::vector<std::string> props;
    auto i = emittedBools.find(prop);
    if (i != emittedBools.end()) {
      props.push_back(strBool);
    }
    i = emittedStrings.find(prop);
    if (i != emittedStrings.end()) {
      props.push_back(strString);
    }
    i = emittedMinNumbers.find(prop);
    if (i != emittedMinNumbers.end()) {
      props.push_back(strNumMin);
    }
    i = emittedMaxNumbers.find(prop);
    if (i != emittedMaxNumbers.end()) {
      props.push_back(strNumMax);
    }
    std::sort(props.begin(), props.end());

    std::string propsString = cmStrCat(
      cmJoin(cmMakeRange(props).retreat(1), ", "), " and the ", props.back());

    std::ostringstream e;
    e << "Property \"" << prop << "\" appears in both the " << propsString
      << " property in the dependencies of target \"" << this->GetName()
      << "\".  This is not allowed. A property may only require "
         "compatibility "
         "in a boolean interpretation, a numeric minimum, a numeric maximum "
         "or a "
         "string interpretation, but not a mixture.";
    this->LocalGenerator->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
}

template <typename PropertyType>
std::string valueAsString(PropertyType);
template <>
std::string valueAsString<bool>(bool value)
{
  return value ? "TRUE" : "FALSE";
}
template <>
std::string valueAsString<const char*>(const char* value)
{
  return value ? value : "(unset)";
}
template <>
std::string valueAsString<std::string>(std::string value)
{
  return value;
}
template <>
std::string valueAsString<cmValue>(cmValue value)
{
  return value ? *value : std::string("(unset)");
}
template <>
std::string valueAsString<std::nullptr_t>(std::nullptr_t /*unused*/)
{
  return "(unset)";
}

static std::string compatibilityType(CompatibleType t)
{
  switch (t) {
    case BoolType:
      return "Boolean compatibility";
    case StringType:
      return "String compatibility";
    case NumberMaxType:
      return "Numeric maximum compatibility";
    case NumberMinType:
      return "Numeric minimum compatibility";
  }
  assert(false && "Unreachable!");
  return "";
}

static std::string compatibilityAgree(CompatibleType t, bool dominant)
{
  switch (t) {
    case BoolType:
    case StringType:
      return dominant ? "(Disagree)\n" : "(Agree)\n";
    case NumberMaxType:
    case NumberMinType:
      return dominant ? "(Dominant)\n" : "(Ignored)\n";
  }
  assert(false && "Unreachable!");
  return "";
}

template <typename PropertyType>
PropertyType getTypedProperty(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter = nullptr);

template <>
bool getTypedProperty<bool>(cmGeneratorTarget const* tgt,
                            const std::string& prop,
                            cmGeneratorExpressionInterpreter* genexInterpreter)
{
  if (genexInterpreter == nullptr) {
    return tgt->GetPropertyAsBool(prop);
  }

  cmValue value = tgt->GetProperty(prop);
  return cmIsOn(genexInterpreter->Evaluate(value ? *value : "", prop));
}

template <>
const char* getTypedProperty<const char*>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  cmValue value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return value.GetCStr();
  }

  return genexInterpreter->Evaluate(value ? *value : "", prop).c_str();
}

template <>
std::string getTypedProperty<std::string>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  cmValue value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return valueAsString(value);
  }

  return genexInterpreter->Evaluate(value ? *value : "", prop);
}

template <typename PropertyType>
PropertyType impliedValue(PropertyType);
template <>
bool impliedValue<bool>(bool /*unused*/)
{
  return false;
}
template <>
const char* impliedValue<const char*>(const char* /*unused*/)
{
  return "";
}
template <>
std::string impliedValue<std::string>(std::string /*unused*/) // NOLINT(*)
{
  return std::string();
}

template <typename PropertyType>
std::pair<bool, PropertyType> consistentProperty(PropertyType lhs,
                                                 PropertyType rhs,
                                                 CompatibleType t);

template <>
std::pair<bool, bool> consistentProperty(bool lhs, bool rhs,
                                         CompatibleType /*unused*/)
{
  return { lhs == rhs, lhs };
}

static std::pair<bool, const char*> consistentStringProperty(const char* lhs,
                                                             const char* rhs)
{
  const bool b = strcmp(lhs, rhs) == 0;
  return { b, b ? lhs : nullptr };
}

static std::pair<bool, std::string> consistentStringProperty(
  const std::string& lhs, const std::string& rhs)
{
  const bool b = lhs == rhs;
  return { b, b ? lhs : valueAsString(nullptr) };
}

static std::pair<bool, const char*> consistentNumberProperty(const char* lhs,
                                                             const char* rhs,
                                                             CompatibleType t)
{
  char* pEnd;

  long lnum = strtol(lhs, &pEnd, 0);
  if (pEnd == lhs || *pEnd != '\0' || errno == ERANGE) {
    return { false, nullptr };
  }

  long rnum = strtol(rhs, &pEnd, 0);
  if (pEnd == rhs || *pEnd != '\0' || errno == ERANGE) {
    return { false, nullptr };
  }

  if (t == NumberMaxType) {
    return { true, std::max(lnum, rnum) == lnum ? lhs : rhs };
  }

  return { true, std::min(lnum, rnum) == lnum ? lhs : rhs };
}

template <>
std::pair<bool, const char*> consistentProperty(const char* lhs,
                                                const char* rhs,
                                                CompatibleType t)
{
  if (!lhs && !rhs) {
    return { true, lhs };
  }
  if (!lhs) {
    return { true, rhs };
  }
  if (!rhs) {
    return { true, lhs };
  }

  switch (t) {
    case BoolType: {
      bool same = cmIsOn(lhs) == cmIsOn(rhs);
      return { same, same ? lhs : nullptr };
    }
    case StringType:
      return consistentStringProperty(lhs, rhs);
    case NumberMinType:
    case NumberMaxType:
      return consistentNumberProperty(lhs, rhs, t);
  }
  assert(false && "Unreachable!");
  return { false, nullptr };
}

static std::pair<bool, std::string> consistentProperty(const std::string& lhs,
                                                       const std::string& rhs,
                                                       CompatibleType t)
{
  const std::string null_ptr = valueAsString(nullptr);

  if (lhs == null_ptr && rhs == null_ptr) {
    return { true, lhs };
  }
  if (lhs == null_ptr) {
    return { true, rhs };
  }
  if (rhs == null_ptr) {
    return { true, lhs };
  }

  switch (t) {
    case BoolType: {
      bool same = cmIsOn(lhs) == cmIsOn(rhs);
      return { same, same ? lhs : null_ptr };
    }
    case StringType:
      return consistentStringProperty(lhs, rhs);
    case NumberMinType:
    case NumberMaxType: {
      auto value = consistentNumberProperty(lhs.c_str(), rhs.c_str(), t);
      return { value.first,
               value.first ? std::string(value.second) : null_ptr };
    }
  }
  assert(false && "Unreachable!");
  return { false, null_ptr };
}

template <typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmGeneratorTarget const* tgt,
                                                 const std::string& p,
                                                 const std::string& config,
                                                 const char* defaultValue,
                                                 CompatibleType t,
                                                 PropertyType* /*unused*/)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt, p);

  std::vector<std::string> headPropKeys = tgt->GetPropertyKeys();
  const bool explicitlySet = cm::contains(headPropKeys, p);

  const bool impliedByUse = tgt->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet) || (!impliedByUse && !explicitlySet));

  std::vector<cmGeneratorTarget const*> const& deps =
    tgt->GetLinkImplementationClosure(config, UseTo::Compile);

  if (deps.empty()) {
    return propContent;
  }
  bool propInitialized = explicitlySet;

  std::string report = cmStrCat(" * Target \"", tgt->GetName());
  if (explicitlySet) {
    report += "\" has property content \"";
    report += valueAsString<PropertyType>(propContent);
    report += "\"\n";
  } else if (impliedByUse) {
    report += "\" property is implied by use.\n";
  } else {
    report += "\" property not set.\n";
  }

  std::string interfaceProperty = "INTERFACE_" + p;
  std::unique_ptr<cmGeneratorExpressionInterpreter> genexInterpreter;
  if (p == "POSITION_INDEPENDENT_CODE") {
    // Corresponds to EvaluatingPICExpression.
    genexInterpreter = cm::make_unique<cmGeneratorExpressionInterpreter>(
      tgt->GetLocalGenerator(), config, tgt);
  }

  for (cmGeneratorTarget const* theTarget : deps) {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    std::vector<std::string> propKeys = theTarget->GetPropertyKeys();

    const bool ifaceIsSet = cm::contains(propKeys, interfaceProperty);
    PropertyType ifacePropContent = getTypedProperty<PropertyType>(
      theTarget, interfaceProperty, genexInterpreter.get());

    std::string reportEntry;
    if (ifaceIsSet) {
      reportEntry += " * Target \"";
      reportEntry += theTarget->GetName();
      reportEntry += "\" property value \"";
      reportEntry += valueAsString<PropertyType>(ifacePropContent);
      reportEntry += "\" ";
    }

    if (explicitlySet) {
      if (ifaceIsSet) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "Property " << p << " on target \"" << tgt->GetName()
            << "\" does\nnot match the "
               "INTERFACE_"
            << p
            << " property requirement\nof "
               "dependency \""
            << theTarget->GetName() << "\".\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      // Explicitly set on target and not set in iface. Can't disagree.
      continue;
    }
    if (impliedByUse) {
      propContent = impliedValue<PropertyType>(propContent);

      if (ifaceIsSet) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "Property " << p << " on target \"" << tgt->GetName()
            << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_"
            << p << " property on\ndependency \"" << theTarget->GetName()
            << "\" is in conflict.\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      // Implicitly set on target and not set in iface. Can't disagree.
      continue;
    }
    if (ifaceIsSet) {
      if (propInitialized) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "The INTERFACE_" << p << " property of \""
            << theTarget->GetName() << "\" does\nnot agree with the value of "
            << p << " already determined\nfor \"" << tgt->GetName() << "\".\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      report += reportEntry + "(Interface set)\n";
      propContent = ifacePropContent;
      propInitialized = true;
    } else {
      // Not set. Nothing to agree on.
      continue;
    }
  }

  tgt->ReportPropertyOrigin(p, valueAsString<PropertyType>(propContent),
                            report, compatibilityType(t));
  return propContent;
}

bool cmGeneratorTarget::GetLinkInterfaceDependentBoolProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config, "FALSE",
                                                   BoolType, nullptr);
}

std::string cmGeneratorTarget::GetLinkInterfaceDependentStringAsBoolProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<std::string>(
    this, p, config, "FALSE", BoolType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentStringProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", StringType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentNumberMinProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", NumberMinType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentNumberMaxProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", NumberMaxType, nullptr);
}
