/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTransformDepfile.h"

#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmsys/FStream.hxx"

#include "cmGccDepfileReader.h"
#include "cmGccDepfileReaderTypes.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
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

void WriteDepfile(cmDepfileFormat format, cmsys::ofstream& fout,
                  const cmLocalGenerator& lg,
                  const cmGccDepfileContent& content)
{
  std::function<std::string(const std::string&)> formatPath =
    [&lg](const std::string& path) -> std::string {
    return lg.MaybeRelativeToTopBinDir(path);
  };
  if (lg.GetGlobalGenerator()->GetName() == "Xcode") {
    // full paths must be preserved for Xcode compliance
    formatPath = [](const std::string& path) -> std::string { return path; };
  }

  for (auto const& dep : content) {
    bool first = true;
    for (auto const& rule : dep.rules) {
      if (!first) {
        fout << " \\\n  ";
      }
      first = false;
      WriteFilenameGcc(fout, formatPath(rule));
    }
    fout << ':';
    for (auto const& path : dep.paths) {
      fout << " \\\n  ";
      WriteFilenameGcc(fout, formatPath(path));
    }
    fout << '\n';
  }

  if (format == cmDepfileFormat::MakeDepfile) {
    // In this case, phony targets must be added for all dependencies
    fout << "\n";
    for (auto const& dep : content) {
      for (auto const& path : dep.paths) {
        fout << "\n";
        WriteFilenameGcc(fout, formatPath(path));
        fout << ":\n";
      }
    }
  }
}

// tlog format : always windows paths on Windows regardless the generator
std::string ConvertToTLogOutputPath(const std::string& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return cmSystemTools::ConvertToWindowsOutputPath(path);
#else
  return cmSystemTools::ConvertToOutputPath(path);
#endif
}

void WriteVsTlog(cmsys::ofstream& fout, const cmLocalGenerator& lg,
                 const cmGccDepfileContent& content)
{
  for (auto const& dep : content) {
    fout << '^';
    bool first = true;
    for (auto const& rule : dep.rules) {
      if (!first) {
        fout << '|';
      }
      first = false;
      fout << ConvertToTLogOutputPath(lg.MaybeRelativeToTopBinDir(rule));
    }
    fout << "\r\n";
    for (auto const& path : dep.paths) {
      fout << ConvertToTLogOutputPath(lg.MaybeRelativeToTopBinDir(path))
           << "\r\n";
    }
  }
}
}

bool cmTransformDepfile(cmDepfileFormat format, const cmLocalGenerator& lg,
                        const std::string& infile, const std::string& outfile)
{
  cmGccDepfileContent content;
  if (cmSystemTools::FileExists(infile)) {
    auto result =
      cmReadGccDepfile(infile.c_str(), lg.GetCurrentBinaryDirectory());
    if (!result) {
      return false;
    }
    content = *std::move(result);
  }

  cmsys::ofstream fout(outfile.c_str());
  if (!fout) {
    return false;
  }
  switch (format) {
    case cmDepfileFormat::GccDepfile:
    case cmDepfileFormat::MakeDepfile:
      WriteDepfile(format, fout, lg, content);
      break;
    case cmDepfileFormat::VsTlog:
      WriteVsTlog(fout, lg, content);
      break;
  }
  return true;
}
