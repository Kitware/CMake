/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDependsC.h"

#include <utility>

#include "cmsys/FStream.hxx"

#include "cmFileTime.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmList.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

#define INCLUDE_REGEX_LINE                                                    \
  "^[ \t]*[#%][ \t]*(include|import)[ \t]*[<\"]([^\">]+)([\">])"

#define INCLUDE_REGEX_LINE_MARKER "#IncludeRegexLine: "
#define INCLUDE_REGEX_SCAN_MARKER "#IncludeRegexScan: "
#define INCLUDE_REGEX_COMPLAIN_MARKER "#IncludeRegexComplain: "
#define INCLUDE_REGEX_TRANSFORM_MARKER "#IncludeRegexTransform: "

cmDependsC::cmDependsC() = default;

cmDependsC::cmDependsC(cmLocalUnixMakefileGenerator3* lg,
                       const std::string& targetDir, const std::string& lang,
                       const DependencyMap* validDeps)
  : cmDepends(lg, targetDir)
  , ValidDeps(validDeps)
{
  cmMakefile* mf = lg->GetMakefile();

  // Configure the include file search path.
  this->SetIncludePathFromLanguage(lang);

  // Configure regular expressions.
  std::string scanRegex = "^.*$";
  std::string complainRegex = "^$";
  {
    std::string scanRegexVar = cmStrCat("CMAKE_", lang, "_INCLUDE_REGEX_SCAN");
    if (cmValue sr = mf->GetDefinition(scanRegexVar)) {
      scanRegex = *sr;
    }
    std::string complainRegexVar =
      cmStrCat("CMAKE_", lang, "_INCLUDE_REGEX_COMPLAIN");
    if (cmValue cr = mf->GetDefinition(complainRegexVar)) {
      complainRegex = *cr;
    }
  }

  this->IncludeRegexLine.compile(INCLUDE_REGEX_LINE);
  this->IncludeRegexScan.compile(scanRegex);
  this->IncludeRegexComplain.compile(complainRegex);
  this->IncludeRegexLineString = INCLUDE_REGEX_LINE_MARKER INCLUDE_REGEX_LINE;
  this->IncludeRegexScanString =
    cmStrCat(INCLUDE_REGEX_SCAN_MARKER, scanRegex);
  this->IncludeRegexComplainString =
    cmStrCat(INCLUDE_REGEX_COMPLAIN_MARKER, complainRegex);

  this->SetupTransforms();

  this->CacheFileName =
    cmStrCat(this->TargetDirectory, '/', lang, ".includecache");

  this->ReadCacheFile();
}

cmDependsC::~cmDependsC()
{
  this->WriteCacheFile();
}

bool cmDependsC::WriteDependencies(const std::set<std::string>& sources,
                                   const std::string& obj,
                                   std::ostream& makeDepends,
                                   std::ostream& internalDepends)
{
  // Make sure this is a scanning instance.
  if (sources.empty() || sources.begin()->empty()) {
    cmSystemTools::Error("Cannot scan dependencies without a source file.");
    return false;
  }
  if (obj.empty()) {
    cmSystemTools::Error("Cannot scan dependencies without an object file.");
    return false;
  }

  std::set<std::string> dependencies;
  bool haveDeps = false;

  // Compute a path to the object file to write to the internal depend file.
  // Any existing content of the internal depend file has already been
  // loaded in ValidDeps with this path as a key.
  std::string obj_i = this->LocalGenerator->MaybeRelativeToTopBinDir(obj);

  if (this->ValidDeps != nullptr) {
    auto const tmpIt = this->ValidDeps->find(obj_i);
    if (tmpIt != this->ValidDeps->end()) {
      dependencies.insert(tmpIt->second.begin(), tmpIt->second.end());
      haveDeps = true;
    }
  }

  if (!haveDeps) {
    // Walk the dependency graph starting with the source file.
    int srcFiles = static_cast<int>(sources.size());
    this->Encountered.clear();

    for (std::string const& src : sources) {
      UnscannedEntry root;
      root.FileName = src;
      this->Unscanned.push(root);
      this->Encountered.insert(src);
    }

    std::set<std::string> scanned;
    while (!this->Unscanned.empty()) {
      // Get the next file to scan.
      UnscannedEntry current = this->Unscanned.front();
      this->Unscanned.pop();

      // If not a full path, find the file in the include path.
      std::string fullName;
      if ((srcFiles > 0) || cmSystemTools::FileIsFullPath(current.FileName)) {
        if (cmSystemTools::FileExists(current.FileName, true)) {
          fullName = current.FileName;
        }
      } else if (!current.QuotedLocation.empty() &&
                 cmSystemTools::FileExists(current.QuotedLocation, true)) {
        // The include statement producing this entry was a double-quote
        // include and the included file is present in the directory of
        // the source containing the include statement.
        fullName = current.QuotedLocation;
      } else {
        auto headerLocationIt =
          this->HeaderLocationCache.find(current.FileName);
        if (headerLocationIt != this->HeaderLocationCache.end()) {
          fullName = headerLocationIt->second;
        } else {
          for (std::string const& iPath : this->IncludePath) {
            // Construct the name of the file as if it were in the current
            // include directory.  Avoid using a leading "./".
            std::string tmpPath =
              cmSystemTools::CollapseFullPath(current.FileName, iPath);

            // Look for the file in this location.
            if (cmSystemTools::FileExists(tmpPath, true)) {
              fullName = tmpPath;
              this->HeaderLocationCache[current.FileName] = std::move(tmpPath);
              break;
            }
          }
        }
      }

      // Complain if the file cannot be found and matches the complain
      // regex.
      if (fullName.empty() &&
          this->IncludeRegexComplain.find(current.FileName)) {
        cmSystemTools::Error("Cannot find file \"" + current.FileName + "\".");
        return false;
      }

      // Scan the file if it was found and has not been scanned already.
      if (!fullName.empty() && (scanned.find(fullName) == scanned.end())) {
        // Record scanned files.
        scanned.insert(fullName);

        // Check whether this file is already in the cache
        auto fileIt = this->FileCache.find(fullName);
        if (fileIt != this->FileCache.end()) {
          fileIt->second.Used = true;
          dependencies.insert(fullName);
          for (UnscannedEntry const& inc : fileIt->second.UnscannedEntries) {
            if (this->Encountered.find(inc.FileName) ==
                this->Encountered.end()) {
              this->Encountered.insert(inc.FileName);
              this->Unscanned.push(inc);
            }
          }
        } else {

          // Try to scan the file.  Just leave it out if we cannot find
          // it.
          cmsys::ifstream fin(fullName.c_str());
          if (fin) {
            cmsys::FStream::BOM bom = cmsys::FStream::ReadBOM(fin);
            if (bom == cmsys::FStream::BOM_None ||
                bom == cmsys::FStream::BOM_UTF8) {
              // Add this file as a dependency.
              dependencies.insert(fullName);

              // Scan this file for new dependencies.  Pass the directory
              // containing the file to handle double-quote includes.
              std::string dir = cmSystemTools::GetFilenamePath(fullName);
              this->Scan(fin, dir, fullName);
            } else {
              // Skip file with encoding we do not implement.
            }
          }
        }
      }

      srcFiles--;
    }
  }

  // Write the dependencies to the output stream.  Makefile rules
  // written by the original local generator for this directory
  // convert the dependencies to paths relative to the home output
  // directory.  We must do the same here.
  std::string obj_m = this->LocalGenerator->ConvertToMakefilePath(obj_i);
  internalDepends << obj_i << '\n';
  if (!dependencies.empty()) {
    const auto& lineContinue = static_cast<cmGlobalUnixMakefileGenerator3*>(
                                 this->LocalGenerator->GetGlobalGenerator())
                                 ->LineContinueDirective;
    bool supportLongLineDepend = static_cast<cmGlobalUnixMakefileGenerator3*>(
                                   this->LocalGenerator->GetGlobalGenerator())
                                   ->SupportsLongLineDependencies();
    if (supportLongLineDepend) {
      makeDepends << obj_m << ':';
    }
    for (std::string const& dep : dependencies) {
      std::string dependee = this->LocalGenerator->ConvertToMakefilePath(
        this->LocalGenerator->MaybeRelativeToTopBinDir(dep));
      if (supportLongLineDepend) {
        makeDepends << ' ' << lineContinue << ' ' << dependee;
      } else {
        makeDepends << obj_m << ": " << dependee << '\n';
      }
      internalDepends << ' ' << dep << '\n';
    }
    makeDepends << '\n';
  }

  return true;
}

void cmDependsC::ReadCacheFile()
{
  if (this->CacheFileName.empty()) {
    return;
  }
  cmsys::ifstream fin(this->CacheFileName.c_str());
  if (!fin) {
    return;
  }

  std::string line;
  cmIncludeLines* cacheEntry = nullptr;
  bool haveFileName = false;

  cmFileTime cacheFileTime;
  bool const cacheFileTimeGood = cacheFileTime.Load(this->CacheFileName);
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    if (line.empty()) {
      cacheEntry = nullptr;
      haveFileName = false;
      continue;
    }
    // the first line after an empty line is the name of the parsed file
    if (!haveFileName) {
      haveFileName = true;

      cmFileTime fileTime;
      bool const res = cacheFileTimeGood && fileTime.Load(line);
      bool const newer = res && cacheFileTime.Newer(fileTime);

      if (res && newer) // cache is newer than the parsed file
      {
        cacheEntry = &this->FileCache[line];
      }
      // file doesn't exist, check that the regular expressions
      // haven't changed
      else if (!res) {
        if (cmHasLiteralPrefix(line, INCLUDE_REGEX_LINE_MARKER)) {
          if (line != this->IncludeRegexLineString) {
            return;
          }
        } else if (cmHasLiteralPrefix(line, INCLUDE_REGEX_SCAN_MARKER)) {
          if (line != this->IncludeRegexScanString) {
            return;
          }
        } else if (cmHasLiteralPrefix(line, INCLUDE_REGEX_COMPLAIN_MARKER)) {
          if (line != this->IncludeRegexComplainString) {
            return;
          }
        } else if (cmHasLiteralPrefix(line, INCLUDE_REGEX_TRANSFORM_MARKER)) {
          if (line != this->IncludeRegexTransformString) {
            return;
          }
        }
      }
    } else if (cacheEntry != nullptr) {
      UnscannedEntry entry;
      entry.FileName = line;
      if (cmSystemTools::GetLineFromStream(fin, line)) {
        if (line != "-") {
          entry.QuotedLocation = line;
        }
        cacheEntry->UnscannedEntries.push_back(std::move(entry));
      }
    }
  }
}

void cmDependsC::WriteCacheFile() const
{
  if (this->CacheFileName.empty()) {
    return;
  }
  cmsys::ofstream cacheOut(this->CacheFileName.c_str());
  if (!cacheOut) {
    return;
  }

  cacheOut << this->IncludeRegexLineString << "\n\n";
  cacheOut << this->IncludeRegexScanString << "\n\n";
  cacheOut << this->IncludeRegexComplainString << "\n\n";
  cacheOut << this->IncludeRegexTransformString << "\n\n";

  for (auto const& fileIt : this->FileCache) {
    if (fileIt.second.Used) {
      cacheOut << fileIt.first << '\n';

      for (UnscannedEntry const& inc : fileIt.second.UnscannedEntries) {
        cacheOut << inc.FileName << '\n';
        if (inc.QuotedLocation.empty()) {
          cacheOut << '-' << '\n';
        } else {
          cacheOut << inc.QuotedLocation << '\n';
        }
      }
      cacheOut << '\n';
    }
  }
}

void cmDependsC::Scan(std::istream& is, const std::string& directory,
                      const std::string& fullName)
{
  cmIncludeLines& newCacheEntry = this->FileCache[fullName];
  newCacheEntry.Used = true;

  // Read one line at a time.
  std::string line;
  while (cmSystemTools::GetLineFromStream(is, line)) {
    // Transform the line content first.
    if (!this->TransformRules.empty()) {
      this->TransformLine(line);
    }

    // Match include directives.
    if (this->IncludeRegexLine.find(line)) {
      // Get the file being included.
      UnscannedEntry entry;
      entry.FileName = this->IncludeRegexLine.match(2);
      cmSystemTools::ConvertToUnixSlashes(entry.FileName);
      if (this->IncludeRegexLine.match(3) == "\"" &&
          !cmSystemTools::FileIsFullPath(entry.FileName)) {
        // This was a double-quoted include with a relative path.  We
        // must check for the file in the directory containing the
        // file we are scanning.
        entry.QuotedLocation =
          cmSystemTools::CollapseFullPath(entry.FileName, directory);
      }

      // Queue the file if it has not yet been encountered and it
      // matches the regular expression for recursive scanning.  Note
      // that this check does not account for the possibility of two
      // headers with the same name in different directories when one
      // is included by double-quotes and the other by angle brackets.
      // It also does not work properly if two header files with the same
      // name exist in different directories, and both are included from a
      // file their own directory by simply using "filename.h" (#12619)
      // This kind of problem will be fixed when a more
      // preprocessor-like implementation of this scanner is created.
      if (this->IncludeRegexScan.find(entry.FileName)) {
        newCacheEntry.UnscannedEntries.push_back(entry);
        if (this->Encountered.find(entry.FileName) ==
            this->Encountered.end()) {
          this->Encountered.insert(entry.FileName);
          this->Unscanned.push(entry);
        }
      }
    }
  }
}

void cmDependsC::SetupTransforms()
{
  // Get the transformation rules.
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  cmList transformRules{ mf->GetDefinition("CMAKE_INCLUDE_TRANSFORMS"),
                         cmList::EmptyElements::Yes };
  for (auto const& tr : transformRules) {
    this->ParseTransform(tr);
  }

  this->IncludeRegexTransformString = INCLUDE_REGEX_TRANSFORM_MARKER;
  if (!this->TransformRules.empty()) {
    // Construct the regular expression to match lines to be
    // transformed.
    std::string xform = "^([ \t]*[#%][ \t]*(include|import)[ \t]*)(";
    const char* sep = "";
    for (auto const& tr : this->TransformRules) {
      xform += sep;
      xform += tr.first;
      sep = "|";
    }
    xform += ")[ \t]*\\(([^),]*)\\)";
    this->IncludeRegexTransform.compile(xform);

    // Build a string that encodes all transformation rules and will
    // change when rules are changed.
    this->IncludeRegexTransformString += xform;
    for (auto const& tr : this->TransformRules) {
      this->IncludeRegexTransformString += " ";
      this->IncludeRegexTransformString += tr.first;
      this->IncludeRegexTransformString += "(%)=";
      this->IncludeRegexTransformString += tr.second;
    }
  }
}

void cmDependsC::ParseTransform(std::string const& xform)
{
  // A transform rule is of the form SOME_MACRO(%)=value-with-%
  // We can simply separate with "(%)=".
  std::string::size_type pos = xform.find("(%)=");
  if (pos == std::string::npos || pos == 0) {
    return;
  }
  std::string name = xform.substr(0, pos);
  std::string value = xform.substr(pos + 4);
  this->TransformRules[name] = value;
}

void cmDependsC::TransformLine(std::string& line)
{
  // Check for a transform rule match.  Return if none.
  if (!this->IncludeRegexTransform.find(line)) {
    return;
  }
  auto tri = this->TransformRules.find(this->IncludeRegexTransform.match(3));
  if (tri == this->TransformRules.end()) {
    return;
  }

  // Construct the transformed line.
  std::string newline = this->IncludeRegexTransform.match(1);
  std::string arg = this->IncludeRegexTransform.match(4);
  for (char c : tri->second) {
    if (c == '%') {
      newline += arg;
    } else {
      newline += c;
    }
  }

  // Return the transformed line.
  line = newline;
}
