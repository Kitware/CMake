/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmWIXPatchParser.h"
#include "cmWIXSourceWriter.h"

/** \class cmWIXPatch
 * \brief Class that maintains and applies patch fragments
 */
class cmWIXPatch
{
public:
  cmWIXPatch(cmCPackLog* logger);

  bool LoadFragments(std::string const& patchFilePath);

  void ApplyFragment(std::string const& id, cmWIXSourceWriter& writer);

  bool CheckForUnappliedFragments();

private:
  void ApplyElementChildren(const cmWIXPatchElement& element,
                            cmWIXSourceWriter& writer);

  void ApplyElement(const cmWIXPatchElement& element,
                    cmWIXSourceWriter& writer);

  cmCPackLog* Logger;

  cmWIXPatchParser::fragment_map_t Fragments;
};
