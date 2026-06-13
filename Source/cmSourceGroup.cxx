/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSourceGroup.h"

#include <utility>

#include <cm/memory>

#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"

class cmSourceGroupInternals
{
public:
  SourceGroupVector GroupChildren;
};

cmSourceGroup::cmSourceGroup(std::string name, cm::string_view regex,
                             cm::string_view parentName)
  : Name(std::move(name))
{
  this->Internal = cm::make_unique<cmSourceGroupInternals>();
  this->SetGroupRegex(regex);
  if (parentName.empty()) {
    this->FullName = this->Name;
  } else {
    this->FullName = cmStrCat(parentName, '\\', this->Name);
  }
}

cmSourceGroup::~cmSourceGroup() = default;

bool cmSourceGroup::SetGroupRegex(cm::string_view regex)
{
  if (regex.data()) {
    return this->GroupRegex.compile(static_cast<std::string>(regex));
  }
  return this->GroupRegex.compile("^$");
}

void cmSourceGroup::ResolveGenex(cmLocalGenerator* lg,
                                 std::string const& config)
{
  std::set<std::string> files;

  for (auto& pair : this->GroupFileSets) {
    files.clear();
    for (std::string const& fileSet : pair.second) {
      cmList list{ cmGeneratorExpression::Evaluate(fileSet, lg, config) };
      files.insert(list.begin(), list.end());
    }
    pair.second = std::move(files);
  }

  files.clear();
  for (std::string const& file : this->GroupFiles) {
    cmList list{ cmGeneratorExpression::Evaluate(file, lg, config) };
    files.insert(list.begin(), list.end());
  }
  this->GroupFiles = std::move(files);

  if (!this->Internal) {
    return;
  }

  for (auto const& group : this->Internal->GroupChildren) {
    group->ResolveGenex(lg, config);
  }
}

void cmSourceGroup::AddGroupFile(std::string const& name)
{
  this->GroupFiles.insert(name);
}

void cmSourceGroup::AddGroupFileSets(std::string const& target,
                                     std::set<std::string> const& fileSets)
{
  auto& fs = this->GroupFileSets[target];
  fs.insert(fileSets.begin(), fileSets.end());
}

std::string const& cmSourceGroup::GetName() const
{
  return this->Name;
}

std::string const& cmSourceGroup::GetFullName() const
{
  return this->FullName;
}

bool cmSourceGroup::MatchesRegex(std::string const& name) const
{
  cmsys::RegularExpressionMatch match;
  return this->GroupRegex.find(name.c_str(), match);
}

bool cmSourceGroup::MatchesFiles(std::string const& name) const
{
  return this->GroupFiles.find(name) != this->GroupFiles.cend();
}

std::set<std::string> const& cmSourceGroup::GetGroupFiles() const
{
  return this->GroupFiles;
}
std::map<std::string, std::set<std::string>> const&
cmSourceGroup::GetGroupFileSets() const
{
  return this->GroupFileSets;
}

void cmSourceGroup::AddChild(std::unique_ptr<cmSourceGroup> child)
{
  this->Internal->GroupChildren.push_back(std::move(child));
}

cmSourceGroup* cmSourceGroup::LookupChild(std::string const& name) const
{
  for (auto& group : this->Internal->GroupChildren) {
    // look if descendant is the one we're looking for
    if (group->GetName() == name) {
      return group.get(); // if so return it
    }
  }

  // if no child with this name was found return NULL
  return nullptr;
}

cmSourceGroup* cmSourceGroup::MatchChildrenFiles(std::string const& name)
{
  if (this->MatchesFiles(name)) {
    return this;
  }
  for (auto& group : this->Internal->GroupChildren) {
    cmSourceGroup* result = group->MatchChildrenFiles(name);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

cmSourceGroup const* cmSourceGroup::MatchChildrenFiles(
  std::string const& name) const
{
  if (this->MatchesFiles(name)) {
    return this;
  }
  for (auto const& group : this->Internal->GroupChildren) {
    cmSourceGroup const* result = group->MatchChildrenFiles(name);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

cmSourceGroup* cmSourceGroup::MatchChildrenRegex(std::string const& name) const
{
  for (auto& group : this->Internal->GroupChildren) {
    cmSourceGroup* result = group->MatchChildrenRegex(name);
    if (result) {
      return result;
    }
  }
  if (this->MatchesRegex(name)) {
    return const_cast<cmSourceGroup*>(this);
  }

  return nullptr;
}

cmSourceGroup* cmSourceGroup::MatchChildrenFileSets(std::string const& target,
                                                    std::string const& fileSet)
{
  auto item = this->GroupFileSets.find(target);
  if (item != this->GroupFileSets.end() &&
      item->second.find(fileSet) != item->second.end()) {
    return this;
  }
  for (auto const& group : this->Internal->GroupChildren) {
    if (cmSourceGroup* result =
          group->MatchChildrenFileSets(target, fileSet)) {
      return result;
    }
  }
  return nullptr;
}

SourceGroupVector const& cmSourceGroup::GetGroupChildren() const
{
  return this->Internal->GroupChildren;
}

/**
 * Find a source group whose regular expression matches the filename
 * part of the given source name.  Search backward through the list of
 * source groups, and take the first matching group found.  This way
 * non-inherited source_group() commands will have precedence over
 * inherited ones.
 */
cmSourceGroup* cmSourceGroup::FindSourceGroup(std::string const& source,
                                              SourceGroupVector const& groups)
{
  // First search for a group that lists the file explicitly.
  for (auto sg = groups.rbegin(); sg != groups.rend(); ++sg) {
    cmSourceGroup* result = (*sg)->MatchChildrenFiles(source);
    if (result) {
      return result;
    }
  }

  // Now search for a group whose regex matches the file.
  for (auto sg = groups.rbegin(); sg != groups.rend(); ++sg) {
    cmSourceGroup* result = (*sg)->MatchChildrenRegex(source);
    if (result) {
      return result;
    }
  }

  // Shouldn't get here, but just in case, return the default group.
  return groups.data()->get();
}

/**
 * Find a source group whose matches the target and file set.
 * Search backward through the list of source groups, and take the first
 * matching group found.  This way non-inherited source_group() commands will
 * have precedence over inherited ones.
 */
cmSourceGroup* cmSourceGroup::FindSourceGroup(std::string const& target,
                                              std::string const& fileSet,
                                              SourceGroupVector const& groups)
{
  // First search for a group that lists the file set explicitly.
  for (auto sg = groups.rbegin(); sg != groups.rend(); ++sg) {
    if (cmSourceGroup* result =
          (*sg)->MatchChildrenFileSets(target, fileSet)) {
      return result;
    }
  }

  return nullptr;
}

void cmSourceGroupFiles::Add(cmSourceGroup const* sg, cmSourceFile const* sf)
{
  this->SourceFiles[sg].push_back(sf);
}

std::vector<cmSourceFile const*> const& cmSourceGroupFiles::GetSourceFiles(
  cmSourceGroup const* sg) const
{
  auto i = this->SourceFiles.find(sg);
  if (i != this->SourceFiles.end()) {
    return i->second;
  }
  static std::vector<cmSourceFile const*> const empty;
  return empty;
}
