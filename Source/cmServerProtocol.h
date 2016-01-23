/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#pragma once

#include "cmListFileCache.h"
#include "cmState.h"

class cmake;
class cmMetadataServer;

struct DifferentialFileContent;

struct OrderFileThenLine
{
  bool operator()(cmListFileContext const& l, cmListFileContext const& r) const
  {
    std::pair<std::string, long> lhs(l.FilePath, l.Line);
    std::pair<std::string, long> rhs(r.FilePath, r.Line);
    const bool res = lhs < rhs;
    return res;
  }
};

class cmServerProtocol
{
public:
  cmServerProtocol(cmMetadataServer* server, std::string buildDir);
  ~cmServerProtocol();

  void processRequest(const std::string& json);

private:
  void ProcessHandshake(const std::string& protocolVersion);
  void ProcessVersion();
  void ProcessBuildsystem();
  void ProcessTargetInfo(std::string tgtName, std::string config,
                         const char* language);
  void ProcessFileInfo(std::string tgtName, std::string config,
                       std::string file_path);
  void ProcessContent(std::string filePath, long fileLine,
                      DifferentialFileContent diff, std::string matcher);
  void ProcessParse(std::string file_path, DifferentialFileContent diff);
  void ProcessContextualHelp(std::string filePath, long fileLine,
                             long fileColumn, std::string fileContent);
  void ProcessContentDiff(
    std::string filePath1, long fileLine1, std::string filePath2,
    long fileLine2,
    std::pair<DifferentialFileContent, DifferentialFileContent> diffs);
  void ProcessCodeComplete(std::string filePath, long fileLine,
                           long fileColumn, DifferentialFileContent diff);
  void ProcessContextWriters(std::string filePath, long fileLine,
                             long fileColumn, DifferentialFileContent diff);

private:
  std::pair<cmState::Snapshot, long> GetSnapshotAndStartLine(
    std::string filePath, long fileLine, DifferentialFileContent diff);

  std::pair<cmState::Snapshot, cmListFileFunction> GetDesiredSnapshot(
    std::vector<std::string> const& editorLines, long startLine,
    cmState::Snapshot snp, long fileLine, bool completionMode = false);

  bool IsNotExecuted(std::string filePath, long fileLine);

  void writeContent(cmState::Snapshot snp, std::string matcher);

  std::pair<cmState::Snapshot, long> GetSnapshotContext(std::string filePath,
                                                        long fileLine);

  bool WriteContextualHelp(std::string const& context,
                           std::string const& help_key);
  bool EmitTypedIdentifier(std::string const& commandName,
                           std::vector<cmListFileArgument> args,
                           size_t argIndex);

private:
  cmMetadataServer* Server;
  cmake* CMakeInstance;
  std::string m_buildDir;
  std::map<cmListFileContext, std::vector<cmState::Snapshot>,
           OrderFileThenLine>
    Snapshots;
};
