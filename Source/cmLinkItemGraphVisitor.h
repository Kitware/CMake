/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>

#include "cmLinkItem.h"

class cmGeneratorTarget;

/** \class cmLinkItemGraphVisitor
 * \brief Visits a graph of linked items.
 *
 * Allows to visit items and dependency links (direct and indirect) between
 * those items.
 * This abstract class takes care of the graph traversal, making sure that:
 *   - it terminates even in the presence of cycles;
 *   - it visits every object once (and only once);
 *   - it visits the objects in the same order every time.
 *
 * Children classes only have to implement OnItem() etc. to handle whatever
 * logic they care about.
 */
class cmLinkItemGraphVisitor
{
public:
  virtual ~cmLinkItemGraphVisitor() = default;

  virtual void VisitGraph(std::string const& name) = 0;

  void VisitItem(cmLinkItem const& item);

protected:
  enum class DependencyType
  {
    LinkInterface,
    LinkPublic,
    LinkPrivate,
    Object,
    Utility
  };

  virtual void OnItem(cmLinkItem const& item) = 0;

  virtual void OnDirectLink(cmLinkItem const& depender,
                            cmLinkItem const& dependee, DependencyType dt) = 0;

  virtual void OnIndirectLink(cmLinkItem const& depender,
                              cmLinkItem const& dependee) = 0;

private:
  std::set<std::string> VisitedItems;

  std::set<std::pair<std::string, std::string>> VisitedLinks;

  void VisitLinks(cmLinkItem const& item, cmLinkItem const& rootItem);
  void VisitLinks(cmLinkItem const& item, cmLinkItem const& rootItem,
                  std::string const& config);

  using Dependency = std::pair<DependencyType, cmLinkItem>;
  using DependencyMap = std::map<std::string, Dependency>;

  bool ItemVisited(cmLinkItem const& item);
  bool LinkVisited(cmLinkItem const& depender, cmLinkItem const& dependee);

  static void GetDependencies(cmGeneratorTarget const& target,
                              std::string const& config,
                              DependencyMap& dependencies);
};
