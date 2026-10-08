/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <cmext/algorithm>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCryptoHash.h"
#include "cmDuration.h"
#include "cmFileTime.h"
#include "cmGeneratedFileStream.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// - Static functions

/// @brief Merges newOpts into baseOpts
/// @arg valueOpts list of options that accept a value
static void MergeOptions(std::vector<std::string>& baseOpts,
                         std::vector<std::string> const& newOpts,
                         std::initializer_list<cm::string_view> valueOpts,
                         bool isQt5OrLater)
{
  if (newOpts.empty()) {
    return;
  }
  if (baseOpts.empty()) {
    baseOpts = newOpts;
    return;
  }

  std::vector<std::string> extraOpts;
  for (auto fit = newOpts.begin(), fitEnd = newOpts.end(); fit != fitEnd;
       ++fit) {
    std::string const& newOpt = *fit;
    auto existIt = std::find(baseOpts.begin(), baseOpts.end(), newOpt);
    if (existIt != baseOpts.end()) {
      if (newOpt.size() >= 2) {
        // Acquire the option name
        std::string optName;
        {
          auto oit = newOpt.begin();
          if (*oit == '-') {
            ++oit;
            if (isQt5OrLater && (*oit == '-')) {
              ++oit;
            }
            optName.assign(oit, newOpt.end());
          }
        }
        // Test if this is a value option and change the existing value
        if (!optName.empty() && cm::contains(valueOpts, optName)) {
          auto const existItNext(existIt + 1);
          auto const fitNext(fit + 1);
          if ((existItNext != baseOpts.end()) && (fitNext != fitEnd)) {
            *existItNext = *fitNext;
            ++fit;
          }
        }
      }
    } else {
      extraOpts.push_back(newOpt);
    }
  }
  // Append options
  cm::append(baseOpts, extraOpts);
}

// - Class definitions

unsigned int const cmQtAutoGen::ParallelMax = 64;

cm::string_view cmQtAutoGen::GeneratorName(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return "AutoGen";
    case GenT::MOC:
      return "AutoMoc";
    case GenT::UIC:
      return "AutoUic";
    case GenT::RCC:
      return "AutoRcc";
  }
  return "AutoGen";
}

cm::string_view cmQtAutoGen::GeneratorNameUpper(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return "AUTOGEN";
    case GenT::MOC:
      return "AUTOMOC";
    case GenT::UIC:
      return "AUTOUIC";
    case GenT::RCC:
      return "AUTORCC";
  }
  return "AUTOGEN";
}

std::string cmQtAutoGen::Tools(bool moc, bool uic, bool rcc)
{
  std::array<cm::string_view, 3> lst;
  decltype(lst)::size_type num = 0;
  if (moc) {
    lst.at(num++) = "AUTOMOC";
  }
  if (uic) {
    lst.at(num++) = "AUTOUIC";
  }
  if (rcc) {
    lst.at(num++) = "AUTORCC";
  }
  switch (num) {
    case 1:
      return std::string(lst[0]);
    case 2:
      return cmStrCat(lst[0], " and ", lst[1]);
    case 3:
      return cmStrCat(lst[0], ", ", lst[1], " and ", lst[2]);
    default:
      break;
  }
  return std::string();
}

std::string cmQtAutoGen::Quoted(cm::string_view text)
{
  static std::initializer_list<std::pair<char const*, char const*>> const
    replacements = { { "\\", "\\\\" }, { "\"", "\\\"" }, { "\a", "\\a" },
                     { "\b", "\\b" },  { "\f", "\\f" },  { "\n", "\\n" },
                     { "\r", "\\r" },  { "\t", "\\t" },  { "\v", "\\v" } };

  std::string res(text);
  for (auto const& pair : replacements) {
    cmSystemTools::ReplaceString(res, pair.first, pair.second);
  }
  return cmStrCat('"', res, '"');
}

std::string cmQtAutoGen::QuotedCommand(std::vector<std::string> const& command)
{
  std::string res;
  for (std::string const& item : command) {
    if (!res.empty()) {
      res.push_back(' ');
    }
    std::string const cesc = cmQtAutoGen::Quoted(item);
    if (item.empty() || (cesc.size() > (item.size() + 2)) ||
        (cesc.find(' ') != std::string::npos)) {
      res += cesc;
    } else {
      res += item;
    }
  }
  return res;
}

std::string cmQtAutoGen::FileNameWithoutLastExtension(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos != cm::string_view::npos) {
    filename.remove_prefix(slashPos + 1);
  }
  auto dotPos = filename.rfind('.');
  return std::string(filename.substr(0, dotPos));
}

std::string cmQtAutoGen::ParentDir(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos == cm::string_view::npos) {
    return std::string();
  }
  return std::string(filename.substr(0, slashPos));
}

std::string cmQtAutoGen::SubDirPrefix(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos == cm::string_view::npos) {
    return std::string();
  }
  return std::string(filename.substr(0, slashPos + 1));
}

std::string cmQtAutoGen::AppendFilenameSuffix(cm::string_view filename,
                                              cm::string_view suffix)
{
  auto dotPos = filename.rfind('.');
  if (dotPos == cm::string_view::npos) {
    return cmStrCat(filename, suffix);
  }
  return cmStrCat(filename.substr(0, dotPos), suffix,
                  filename.substr(dotPos, filename.size() - dotPos));
}

void cmQtAutoGen::UicMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5OrLater)
{
  static std::initializer_list<cm::string_view> const valueOpts = {
    "tr",      "translate", "postfix", "generator",
    "include", // Since Qt 5.3
    "g"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5OrLater);
}

void cmQtAutoGen::RccMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5OrLater)
{
  static std::initializer_list<cm::string_view> const valueOpts = {
    "name", "root", "compress", "threshold"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5OrLater);
}

static void RccListParseContent(std::string const& content,
                                std::vector<std::string>& files)
{
  cmsys::RegularExpression fileMatchRegex("(<file[^<]+)");
  cmsys::RegularExpression fileReplaceRegex("(^<file[^>]*>)");

  char const* contentChars = content.c_str();
  while (fileMatchRegex.find(contentChars)) {
    std::string const qrcEntry = fileMatchRegex.match(1);
    contentChars += qrcEntry.size();
    {
      fileReplaceRegex.find(qrcEntry);
      std::string const tag = fileReplaceRegex.match(1);
      files.push_back(qrcEntry.substr(tag.size()));
    }
  }
}

static bool RccListReadQrcContent(std::string const& qrcFile,
                                  std::string& qrcContents)
{
  cmsys::ifstream ifs(qrcFile.c_str());
  if (!ifs) {
    return false;
  }
  std::ostringstream osst;
  osst << ifs.rdbuf();
  qrcContents = osst.str();
  return true;
}

static bool RccListReadQrc(std::string const& qrcFile,
                           std::vector<std::string>& files)
{
  std::string qrcContents;
  if (!RccListReadQrcContent(qrcFile, qrcContents)) {
    return false;
  }
  RccListParseContent(qrcContents, files);
  return true;
}

static bool RccListParseOutput(std::string const& rccStdOut,
                               std::string const& rccStdErr,
                               std::vector<std::string>& files,
                               std::string& error)
{
  // Lambda to strip CR characters
  auto StripCR = [](std::string& line) {
    std::string::size_type cr = line.find('\r');
    if (cr != std::string::npos) {
      line.resize(cr);
    }
  };

  // Parse rcc std output
  {
    std::istringstream ostr(rccStdOut);
    std::string oline;
    while (std::getline(ostr, oline)) {
      StripCR(oline);
      if (!oline.empty()) {
        files.push_back(oline);
      }
    }
  }
  // Parse rcc error output
  {
    std::istringstream estr(rccStdErr);
    std::string eline;
    while (std::getline(estr, eline)) {
      StripCR(eline);
      if (cmHasLiteralPrefix(eline, "RCC: Error in")) {
        static std::string const searchString = "Cannot find file '";

        std::string::size_type pos = eline.find(searchString);
        if (pos == std::string::npos) {
          error = cmStrCat("rcc lists unparsable output:\n",
                           cmQtAutoGen::Quoted(eline), '\n');
          return false;
        }
        pos += searchString.length();
        std::string::size_type sz = eline.size() - pos - 1;
        files.push_back(eline.substr(pos, sz));
      }
    }
  }

  return true;
}

// The header of a cached rcc listing identifies the rcc executable, its
// options, and the qrc file.  The qrc file is identified by a hash of its
// content, because its modification time may not change if it is modified
// again within the time resolution of the file system.
static std::string RccListCacheHeader(std::string const& rccExecutable,
                                      std::vector<std::string> const& options,
                                      std::string const& qrcFile,
                                      std::string const& qrcContent)
{
  cmFileTime rccTime;
  if (!rccTime.Load(rccExecutable)) {
    return std::string();
  }
  cmCryptoHash sha256(cmCryptoHash::AlgoSHA256);
  return cmStrCat(
    "rcc=", rccExecutable, "\nrcc_time=", std::to_string(rccTime.GetTime()),
    "\nrcc_size=", std::to_string(cmSystemTools::FileLength(rccExecutable)),
    "\noptions=", cmJoin(options, ";"), "\nqrc=", qrcFile,
    "\nqrc_sha256=", sha256.HashString(qrcContent), '\n');
}

static bool RccListReadCache(std::string const& cacheFile,
                             std::string const& header,
                             std::vector<std::string>& files)
{
  cmsys::ifstream fin(cacheFile.c_str(), std::ios::in | std::ios::binary);
  if (!fin) {
    return false;
  }
  std::ostringstream content;
  content << fin.rdbuf();
  std::string const cache = content.str();
  if (!cmHasPrefix(cache, header)) {
    return false;
  }
  std::vector<std::string> cachedFiles;
  std::istringstream lines(cache.substr(header.size()));
  std::string line;
  while (std::getline(lines, line)) {
    // Tolerate line endings converted to CRLF.
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) {
      continue;
    }
    // rcc lists the content of directories, so the listing is outdated
    // if a listed path became a directory.
    if (cmSystemTools::FileIsDirectory(line)) {
      return false;
    }
    cachedFiles.push_back(std::move(line));
  }
  cm::append(files, cachedFiles);
  return true;
}

static void RccListWriteCache(std::string const& cacheFile,
                              std::string const& header,
                              std::string const& qrcContent,
                              std::string const& qrcDir,
                              std::vector<std::string>::const_iterator first,
                              std::vector<std::string>::const_iterator last)
{
  // rcc lists the content of directories referred to by the qrc file, which
  // may change without the qrc file changing.  Do not cache such listings.
  // Rather than interpreting the qrc file the way rcc does, cache only
  // listings in which rcc listed each entry of the qrc file as given.
  // Directories, and entries rcc resolves differently, e.g. names with
  // XML character references, are not listed as given.
  {
    std::vector<std::string> entries;
    RccListParseContent(qrcContent, entries);
    std::unordered_set<std::string> const listed(first, last);
    for (std::string const& entry : entries) {
      if (!listed.count(cmSystemTools::CollapseFullPath(
            cmTrimWhitespace(entry), qrcDir))) {
        cmSystemTools::RemoveFile(cacheFile);
        return;
      }
    }
  }

  std::string content = header;
  for (; first != last; ++first) {
    content += *first;
    content += '\n';
  }
  cmGeneratedFileStream::WriteIfDifferent(cacheFile, content, true);
}

cmQtAutoGen::RccLister::RccLister() = default;

cmQtAutoGen::RccLister::RccLister(std::string rccExecutable,
                                  std::vector<std::string> listOptions)
  : RccExecutable_(std::move(rccExecutable))
  , ListOptions_(std::move(listOptions))
{
}

bool cmQtAutoGen::RccLister::list(std::string const& qrcFile,
                                  std::vector<std::string>& files,
                                  std::string& error, bool verbose,
                                  std::string const& cacheFile) const
{
  error.clear();

  if (!cmSystemTools::FileExists(qrcFile, true)) {
    error =
      cmStrCat("The resource file ", Quoted(qrcFile), " does not exist.");
    return false;
  }

  // Run rcc list command in the directory of the qrc file with the pathless
  // qrc file name argument.  This way rcc prints relative paths.
  // This avoids issues on Windows when the qrc file is in a path that
  // contains non-ASCII characters.
  std::string const fileDir = cmSystemTools::GetFilenamePath(qrcFile);

  std::size_t const firstFile = files.size();
  std::string cacheHeader;
  std::string qrcContent;
  if (!this->RccExecutable_.empty() &&
      cmSystemTools::FileExists(this->RccExecutable_, true) &&
      !this->ListOptions_.empty()) {

    // Running rcc is expensive.  Reuse the listing of a previous run if
    // neither rcc nor the qrc file changed since.
    if (!cacheFile.empty() && RccListReadQrcContent(qrcFile, qrcContent)) {
      cacheHeader = RccListCacheHeader(
        this->RccExecutable_, this->ListOptions_, qrcFile, qrcContent);
      if (!cacheHeader.empty() &&
          RccListReadCache(cacheFile, cacheHeader, files)) {
        return true;
      }
    }

    bool result = false;
    int retVal = 0;
    std::string rccStdOut;
    std::string rccStdErr;
    {
      std::vector<std::string> cmd;
      cmd.emplace_back(this->RccExecutable_);
      cm::append(cmd, this->ListOptions_);
      cmd.emplace_back(cmSystemTools::GetFilenameName(qrcFile));

      // Log command
      if (verbose) {
        cmSystemTools::Stdout(
          cmStrCat("Running command:\n", QuotedCommand(cmd), '\n'));
      }

      result = cmSystemTools::RunSingleCommand(
        cmd, &rccStdOut, &rccStdErr, &retVal, fileDir.c_str(),
        cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
    }
    if (!result || retVal) {
      error =
        cmStrCat("The rcc list process failed for ", Quoted(qrcFile), '\n');
      if (!rccStdOut.empty()) {
        error = cmStrCat(std::move(error), rccStdOut, '\n');
      }
      if (!rccStdErr.empty()) {
        error = cmStrCat(std::move(error), rccStdErr, '\n');
      }
      return false;
    }
    if (!RccListParseOutput(rccStdOut, rccStdErr, files, error)) {
      return false;
    }
  } else {
    // We can't use rcc for the file listing.
    // Read the qrc file content into string and parse it.
    if (!RccListReadQrc(qrcFile, files)) {
      error =
        cmStrCat("The resource file ", Quoted(qrcFile), " is not readable\n");
      return false;
    }
  }

  // Convert relative paths to absolute paths
  for (std::string& entry : files) {
    entry = cmSystemTools::CollapseFullPath(entry, fileDir);
  }

  if (!cacheHeader.empty()) {
    RccListWriteCache(cacheFile, cacheHeader, qrcContent, fileDir,
                      files.cbegin() + firstFile, files.cend());
  }
  return true;
}
