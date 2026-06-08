/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

class cmLocalGenerator;
class cmSourceFile;
class cmSourceGroup;
class cmSourceGroupInternals;

using SourceGroupVector = std::vector<std::unique_ptr<cmSourceGroup>>;

/** \class cmSourceGroup
 * \brief Hold a group of sources as specified by a SOURCE_GROUP command.
 *
 * cmSourceGroup holds a regular expression and a list of files.  When
 * local generators are about to generate the rules for a target's
 * files, the set of source groups is consulted to group files
 * together.  A file is placed into the last source group that lists
 * the file by name.  If no group lists the file, it is placed into
 * the last group whose regex matches it.
 */
class cmSourceGroup
{
public:
  cmSourceGroup(std::string name, cm::string_view regex,
                cm::string_view parentName = {});
  cmSourceGroup(cmSourceGroup const& r) = delete;
  ~cmSourceGroup();
  cmSourceGroup& operator=(cmSourceGroup const&) = delete;

  /**
   * Set the regular expression for this group.
   * Returns false if the regular expression cannot be compiled.
   */
  bool SetGroupRegex(cm::string_view regex);

  /**
   * Resolve genex.
   */
  void ResolveGenex(cmLocalGenerator* lg, std::string const& config);

  /**
   * Add a file name to the explicit list of files for this group.
   */
  void AddGroupFile(std::string const& name);

  /**
   * Add child to this sourcegroup
   */
  void AddChild(std::unique_ptr<cmSourceGroup> child);

  /**
   * Looks up child and returns it
   */
  cmSourceGroup* LookupChild(std::string const& name) const;

  /**
   * Get the name of this group.
   */
  std::string const& GetName() const;

  /**
   * Get the full path name for group.
   */
  std::string const& GetFullName() const;

  /**
   * Check if the given name matches this group's regex.
   */
  bool MatchesRegex(std::string const& name) const;

  /**
   * Check if the given name matches this group's explicit file list.
   */
  bool MatchesFiles(std::string const& name) const;

  /**
   * Check if the given name matches this group's explicit file list
   * in children.
   */
  cmSourceGroup* MatchChildrenFiles(std::string const& name);

  /**
   * Check if the given name matches this group's explicit file list
   * in children.
   */
  cmSourceGroup const* MatchChildrenFiles(std::string const& name) const;

  /**
   * Check if the given name matches this group's regex in children.
   */
  cmSourceGroup* MatchChildrenRegex(std::string const& name) const;

  /**
   * Get the set of file names explicitly added to this source group.
   */
  std::set<std::string> const& GetGroupFiles() const;

  SourceGroupVector const& GetGroupChildren() const;

  /**
   * Given a source group collection, find the source group for a given source.
   */
  static cmSourceGroup* FindSourceGroup(std::string const& source,
                                        SourceGroupVector const& groups);

private:
  /**
   * The name of the source group.
   */
  std::string Name;
  // Full path to group
  std::string FullName;

  /**
   * The regular expression matching the files in the group.
   */
  cmsys::RegularExpression GroupRegex;

  /**
   * Set of file names explicitly added to this group.
   */
  std::set<std::string> GroupFiles;

  std::unique_ptr<cmSourceGroupInternals> Internal;
};

/** \class cmSourceGroup
 * \brief Used by generators to organize a target's sources into groups.
 */
class cmSourceGroupFiles
{
  std::map<cmSourceGroup const*, std::vector<cmSourceFile const*>> SourceFiles;

public:
  void Add(cmSourceGroup const* sg, cmSourceFile const* sf);
  std::vector<cmSourceFile const*> const& GetSourceFiles(
    cmSourceGroup const* sg) const;
};
