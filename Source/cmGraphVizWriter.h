/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef CMGRAPHVIZWRITER_H
#define CMGRAPHVIZWRITER_H

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
                   const cmGlobalGenerator* globalGenerator);
  ~cmGraphVizWriter() override;

  void VisitGraph(std::string const& name) override;

  void OnItem(cmLinkItem const& item) override;

  void OnDirectLink(cmLinkItem const& depender, cmLinkItem const& dependee,
                    DependencyType dt) override;

  void OnIndirectLink(cmLinkItem const& depender,
                      cmLinkItem const& dependee) override;

  void ReadSettings(const std::string& settingsFileName,
                    const std::string& fallbackSettingsFileName);

  void Write();

private:
  using FileStreamMap =
    std::map<std::string, std::unique_ptr<cmGeneratedFileStream>>;

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

  void CreateTargetFile(FileStreamMap& fileStreamMap, cmLinkItem const& target,
                        std::string const& fileNameSuffix = "");

  void WriteConnection(cmGeneratedFileStream& fs,
                       cmLinkItem const& dependerTargetName,
                       cmLinkItem const& dependeeTargetName,
                       std::string const& edgeStyle);

  void FindAllConnections(const ConnectionsMap& connectionMap,
                          const cmLinkItem& rootItem,
                          Connections& extendedCons,
                          std::set<cmLinkItem>& visitedItems);

  void FindAllConnections(const ConnectionsMap& connectionMap,
                          const cmLinkItem& rootItem,
                          Connections& extendedCons);

  template <typename DirFunc>
  void WritePerTargetConnections(const ConnectionsMap& connections,
                                 const FileStreamMap& streams);

  bool ItemExcluded(cmLinkItem const& item);
  bool ItemNameFilteredOut(std::string const& itemName);
  bool TargetTypeEnabled(cmStateEnums::TargetType targetType) const;

  std::string ItemNameWithAliases(std::string const& itemName) const;

  static std::string GetEdgeStyle(DependencyType dt);

  static std::string EscapeForDotFile(std::string const& str);

  static std::string PathSafeString(std::string const& str);

  std::string FileName;
  cmGeneratedFileStream GlobalFileStream;
  FileStreamMap PerTargetFileStreams;
  FileStreamMap TargetDependersFileStreams;

  ConnectionsMap PerTargetConnections;
  ConnectionsMap TargetDependersConnections;

  std::string GraphName;
  std::string GraphHeader;
  std::string GraphNodePrefix;

  std::vector<cmsys::RegularExpression> TargetsToIgnoreRegex;

  cmGlobalGenerator const* GlobalGenerator;

  int NextNodeId;
  // maps from the actual item names to node names in dot:
  std::map<std::string, std::string> NodeNames;

  bool GenerateForExecutables;
  bool GenerateForStaticLibs;
  bool GenerateForSharedLibs;
  bool GenerateForModuleLibs;
  bool GenerateForInterfaceLibs;
  bool GenerateForObjectLibs;
  bool GenerateForUnknownLibs;
  bool GenerateForCustomTargets;
  bool GenerateForExternals;
  bool GeneratePerTarget;
  bool GenerateDependers;
};

#endif
