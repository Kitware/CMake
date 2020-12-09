/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTransformDepfile.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmsys/FStream.hxx"

#include "cmGccDepfileReader.h"
#include "cmGccDepfileReaderTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
void WriteFilenameGcc(cmsys::ofstream& fout, const std::string& filename)
{
  for (auto c : filename) {
    switch (c) {
      case ' ':
        fout << "\\ ";
        break;
      case '\\':
        fout << "\\\\";
        break;
      default:
        fout << c;
        break;
    }
  }
}

void WriteGccDepfile(cmsys::ofstream& fout, const cmGccDepfileContent& content)
{
  for (auto const& dep : content) {
    bool first = true;
    for (auto const& rule : dep.rules) {
      if (!first) {
        fout << " \\\n  ";
      }
      first = false;
      WriteFilenameGcc(fout, rule);
    }
    fout << ':';
    for (auto const& path : dep.paths) {
      fout << " \\\n  ";
      WriteFilenameGcc(fout, path);
    }
    fout << '\n';
  }
}

void WriteVsTlog(cmsys::ofstream& fout, const cmGccDepfileContent& content)
{
  for (auto const& dep : content) {
    fout << '^';
    bool first = true;
    for (auto const& rule : dep.rules) {
      if (!first) {
        fout << '|';
      }
      first = false;
      fout << cmSystemTools::ConvertToOutputPath(rule);
    }
    fout << "\r\n";
    for (auto const& path : dep.paths) {
      fout << cmSystemTools::ConvertToOutputPath(path) << "\r\n";
    }
  }
}
}

bool cmTransformDepfile(cmDepfileFormat format, const std::string& prefix,
                        const std::string& infile, const std::string& outfile)
{
  cmGccDepfileContent content;
  if (cmSystemTools::FileExists(infile)) {
    auto result = cmReadGccDepfile(infile.c_str());
    if (!result) {
      return false;
    }
    content = *std::move(result);
  }

  for (auto& dep : content) {
    for (auto& rule : dep.rules) {
      if (!cmSystemTools::FileIsFullPath(rule)) {
        rule = cmStrCat(prefix, rule);
      }
    }
    for (auto& path : dep.paths) {
      if (!cmSystemTools::FileIsFullPath(path)) {
        path = cmStrCat(prefix, path);
      }
    }
  }

  cmsys::ofstream fout(outfile.c_str());
  if (!fout) {
    return false;
  }
  switch (format) {
    case cmDepfileFormat::GccDepfile:
      WriteGccDepfile(fout, content);
      break;
    case cmDepfileFormat::VsTlog:
      WriteVsTlog(fout, content);
      break;
  }
  return true;
}
