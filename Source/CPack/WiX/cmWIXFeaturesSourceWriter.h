/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmCPackGenerator.h"
#include "cmWIXPatch.h"
#include "cmWIXSourceWriter.h"

/** \class cmWIXFeaturesSourceWriter
 * \brief Helper class to generate features.wxs
 */
class cmWIXFeaturesSourceWriter : public cmWIXSourceWriter
{
public:
  cmWIXFeaturesSourceWriter(unsigned long wixVersion, cmCPackLog* logger,
                            std::string const& filename,
                            GuidType componentGuidType);

  void CreateCMakePackageRegistryEntry(std::string const& package,
                                       std::string const& upgradeGuid);

  void EmitFeatureForComponentGroup(cmCPackComponentGroup const& group,
                                    cmWIXPatch& patch);

  void EmitFeatureForComponent(cmCPackComponent const& component,
                               cmWIXPatch& patch);

  void EmitComponentRef(std::string const& id);
};
