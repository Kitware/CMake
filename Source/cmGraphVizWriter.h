/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmGeneratedFileStream.h"
#include "cmLinkItem.h"
#include "cmLinkItemGraphVisitor.h"
#include "cmStateTypes.h"

class cmGlobalGenerator;

/** This class implements writing files for graphviz (dot) for graphs
 * representing the dependencies between the targets in the project. */
class cmGraphVizWriter : public cmLinkItemGraphVisitor
{
public:
  cmGraphVizWriter(std::string const& fileName,
                   cmGlobalGenerator const* globalGenerator);
  ~cmGraphVizWriter() override;

  void VisitGraph(std::string const& name) override;

  void OnItem(cmLinkItem const& item) override;

  void OnDirectLink(cmLinkItem const& depender, cmLinkItem const& dependee,
                    DependencyType dt) override;

  void OnIndirectLink(cmLinkItem const& depender,
                      cmLinkItem const& dependee) override;

  void ReadSettings(std::string const& settingsFileName,
                    std::string const& fallbackSettingsFileName);

  void Write();

private:
  struct Connection
  {
    Connection(cmLinkItem s, cmLinkItem d, std::string scope)
      : src(std::move(s))
      , dst(std::move(d))
      , scopeType(std::move(scope))
    {
    }

    cmLinkItem src;
    cmLinkItem dst;
    std::string scopeType;
  };
  using Connections = std::vector<Connection>;
  using ConnectionsMap = std::map<cmLinkItem, Connections>;

  void VisitLink(cmLinkItem const& depender, cmLinkItem const& dependee,
                 bool isDirectLink, std::string const& scopeType = "");

  void WriteHeader(cmGeneratedFileStream& fs, std::string const& name);

  void WriteFooter(cmGeneratedFileStream& fs);

  void WriteLegend(cmGeneratedFileStream& fs);

  void WriteNode(cmGeneratedFileStream& fs, cmLinkItem const& item);

  std::unique_ptr<cmGeneratedFileStream> CreateTargetFile(
    cmLinkItem const& target, std::string const& fileNameSuffix = "");

  void WriteConnection(cmGeneratedFileStream& fs,
                       cmLinkItem const& dependerTargetName,
                       cmLinkItem const& dependeeTargetName,
                       std::string const& edgeStyle);

  void FindAllConnections(ConnectionsMap const& connectionMap,
                          cmLinkItem const& rootItem,
                          Connections& extendedCons,
                          std::set<cmLinkItem>& visitedItems);

  void FindAllConnections(ConnectionsMap const& connectionMap,
                          cmLinkItem const& rootItem,
                          Connections& extendedCons);

  template <typename DirFunc>
  void WritePerTargetConnections(ConnectionsMap const& connections,
                                 std::string const& fileNameSuffix = "");

  bool ItemExcluded(cmLinkItem const& item);
  bool ItemNameFilteredOut(std::string const& itemName);
  bool TargetTypeEnabled(cmStateEnums::TargetType targetType) const;

  std::string ItemNameWithAliases(std::string const& itemName) const;

  static std::string GetEdgeStyle(DependencyType dt);

  static std::string EscapeForDotFile(std::string const& str);

  static std::string PathSafeString(std::string const& str);

  std::string FileName;
  cmGeneratedFileStream GlobalFileStream;

  ConnectionsMap PerTargetConnections;
  ConnectionsMap TargetDependersConnections;

  std::string GraphName;
  std::string GraphHeader;
  std::string GraphNodePrefix;

  std::vector<cmsys::RegularExpression> TargetsToIgnoreRegex;

  cmGlobalGenerator const* GlobalGenerator;

  int NextNodeId = 0;
  // maps from the actual item names to node names in dot:
  std::map<std::string, std::string> NodeNames;

  bool GenerateForExecutables = true;
  bool GenerateForStaticLibs = true;
  bool GenerateForSharedLibs = true;
  bool GenerateForModuleLibs = true;
  bool GenerateForInterfaceLibs = true;
  bool GenerateForObjectLibs = true;
  bool GenerateForUnknownLibs = true;
  bool GenerateForCustomTargets = false;
  bool GenerateForExternals = true;
  bool GeneratePerTarget = true;
  bool GenerateDependers = true;
};
