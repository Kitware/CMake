/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGccDepfileReader.h"

#include <type_traits>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmGccDepfileLexerHelper.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cm::optional<cmGccDepfileContent> cmReadGccDepfile(
  const char* filePath, const std::string& prefix,
  GccDepfilePrependPaths prependPaths)
{
  cmGccDepfileLexerHelper helper;
  if (!helper.readFile(filePath)) {
    return cm::nullopt;
  }
  auto deps = cm::make_optional(std::move(helper).extractContent());

  for (auto& dep : *deps) {
    for (auto& rule : dep.rules) {
      if (prependPaths == GccDepfilePrependPaths::All && !prefix.empty() &&
          !cmSystemTools::FileIsFullPath(rule)) {
        rule = cmStrCat(prefix, '/', rule);
      }
      if (cmSystemTools::FileIsFullPath(rule)) {
        rule = cmSystemTools::CollapseFullPath(rule);
      }
      cmSystemTools::ConvertToLongPath(rule);
    }
    for (auto& path : dep.paths) {
      if (!prefix.empty() && !cmSystemTools::FileIsFullPath(path)) {
        path = cmStrCat(prefix, '/', path);
      }
      if (cmSystemTools::FileIsFullPath(path)) {
        path = cmSystemTools::CollapseFullPath(path);
      }
      cmSystemTools::ConvertToLongPath(path);
    }
  }

  return deps;
}
