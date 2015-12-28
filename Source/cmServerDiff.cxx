/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmServerDiff.h"

#include "cmSystemTools.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_writer.h"
#endif

#include <cmsys/FStream.hxx>

#include "dtl/dtl.hpp"

DifferentialFileContent GetDiffImpl(Json::Value value,
                                    std::string file_path_key,
                                    std::string file_content_key)
{
  DifferentialFileContent diff;

  {
    cmsys::ifstream fin(value[file_path_key].asString().c_str());
    if (!fin) {
      return diff;
    }
    for (std::string line; cmSystemTools::GetLineFromStream(fin, line);) {
      diff.OrigLines.push_back(line);
    }
    diff.OrigLines.push_back("");
  }

  if (!value.isMember(file_content_key)) {
    diff.EditorLines = diff.OrigLines;
  } else {
    auto editorContent = value[file_content_key].asString();

    std::stringstream ss(editorContent);

    for (std::string line; std::getline(ss, line, '\n');) {
      diff.EditorLines.push_back(line);
    }
    diff.EditorLines.push_back("");
  }

  dtl::Diff<std::string> contentDiff(diff.OrigLines, diff.EditorLines);
  contentDiff.compose();

  auto seq = contentDiff.getSes().getSequence();

  auto it = seq.begin();

  if (it == seq.end()) {
    return diff;
  }

  auto lastType = it->second.type;

  long origLineNumber = 1;
  long newLineNumber = 1;

  auto& chunks = diff.Chunks;

  chunks.push_back({});

  if (lastType == dtl::SES_COMMON) {
    ++chunks.back().NumCommon;
    ++newLineNumber;
    ++origLineNumber;
  }
  if (lastType == dtl::SES_ADD) {
    ++chunks.back().NumAdded;
    ++newLineNumber;
  }
  if (lastType == dtl::SES_DELETE) {
    ++chunks.back().NumRemoved;
    ++origLineNumber;
  }

  ++it;

  for (; it != seq.end(); ++it) {
    auto const& hunk = *it;

    auto& info = hunk.second;
    switch (info.type) {
      case dtl::SES_ADD: {
        if (lastType == dtl::SES_COMMON) {
          chunks.push_back({});
          chunks.back().OrigStart = origLineNumber;
          chunks.back().NewStart = newLineNumber;
        }
        ++chunks.back().NumAdded;
        ++newLineNumber;
        break;
      }
      case dtl::SES_DELETE: {
        if (lastType == dtl::SES_COMMON) {
          chunks.push_back({});
          chunks.back().OrigStart = origLineNumber;
          chunks.back().NewStart = newLineNumber;
        }
        ++chunks.back().NumRemoved;
        ++origLineNumber;
        break;
      }
      case dtl::SES_COMMON: {
        if (lastType != dtl::SES_COMMON) {
          chunks.push_back({});
          chunks.back().OrigStart = origLineNumber;
          chunks.back().NewStart = newLineNumber;
        }
        ++chunks.back().NumCommon;
        ++origLineNumber;
        ++newLineNumber;
        break;
      }
    }
    lastType = info.type;
  }

  return diff;
}

std::pair<DifferentialFileContent, DifferentialFileContent>
cmServerDiff::GetDiffs(Json::Value value)
{
  return std::make_pair(GetDiffImpl(value, "file_path1", "file_content1"),
                        GetDiffImpl(value, "file_path2", "file_content2"));
}

DifferentialFileContent cmServerDiff::GetDiff(Json::Value value)
{
  return GetDiffImpl(value, "file_path", "file_content");
}
