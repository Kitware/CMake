/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmTargetPropertyHelper.h"

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"
#include "cmValue.h"

cmGetTargetPropertyResult cmGetTargetProperty(std::string const& targetName,
                                              std::string const& propertyName,
                                              cmMakefile& mf,
                                              cmValue& propertyValue)
{
  cmTarget* target = mf.FindTargetToUse(targetName);
  if (!target) {
    return cmGetTargetPropertyResult::TargetNotFound;
  }
  return cmGetTargetProperty(targetName, target, propertyName, mf,
                             propertyValue);
}

cmGetTargetPropertyResult cmGetTargetProperty(std::string const& targetName,
                                              cmTarget const* target,
                                              std::string const& propertyName,
                                              cmMakefile& mf,
                                              cmValue& propertyValue)
{
  if (propertyName == "ALIASED_TARGET" || propertyName == "ALIAS_GLOBAL") {
    if (mf.IsAlias(targetName)) {
      if (propertyName == "ALIASED_TARGET") {
        propertyValue = cmValue(target->GetName());
      } else {
        static std::string const sTrue = "TRUE";
        static std::string const sFalse = "FALSE";
        propertyValue = cmValue(
          mf.GetGlobalGenerator()->IsAlias(targetName) ? sTrue : sFalse);
      }
    } else {
      propertyValue = cmValue(nullptr);
    }
    return cmGetTargetPropertyResult::Success;
  }

  propertyValue = target->GetComputedProperty(propertyName, mf);
  if (!propertyValue) {
    propertyValue = target->GetProperty(propertyName);
  }
  return cmGetTargetPropertyResult::Success;
}
