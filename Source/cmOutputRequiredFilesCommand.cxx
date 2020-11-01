/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOutputRequiredFilesCommand.h"

#include <cstdio>
#include <map>
#include <set>
#include <utility>

#include <cm/memory>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

namespace {
/** \class cmDependInformation
 * \brief Store dependency information for a single source file.
 *
 * This structure stores the depend information for a single source file.
 */
class cmDependInformation
{
public:
  /**
   * Construct with dependency generation marked not done; instance
   * not placed in cmMakefile's list.
   */
  cmDependInformation() = default;

  /**
   * The set of files on which this one depends.
   */
  using DependencySetType = std::set<cmDependInformation*>;
  DependencySetType DependencySet;

  /**
   * This flag indicates whether dependency checking has been
   * performed for this file.
   */
  bool DependDone = false;

  /**
   * If this object corresponds to a cmSourceFile instance, this points
   * to it.
   */
  const cmSourceFile* SourceFile = nullptr;

  /**
   * Full path to this file.
   */
  std::string FullPath;

  /**
   * Full path not including file name.
   */
  std::string PathOnly;

  /**
   * Name used to #include this file.
   */
  std::string IncludeName;

  /**
   * This method adds the dependencies of another file to this one.
   */
  void AddDependencies(cmDependInformation* info)
  {
    if (this != info) {
      this->DependencySet.insert(info);
    }
  }
};

class cmLBDepend
{
public:
  /**
   * Construct the object with verbose turned off.
   */
  cmLBDepend()
  {
    this->Verbose = false;
    this->IncludeFileRegularExpression.compile("^.*$");
    this->ComplainFileRegularExpression.compile("^$");
  }

  /**
   * Destructor.
   */
  ~cmLBDepend() = default;

  cmLBDepend(const cmLBDepend&) = delete;
  cmLBDepend& operator=(const cmLBDepend&) = delete;

  /**
   * Set the makefile that is used as a source of classes.
   */
  void SetMakefile(cmMakefile* makefile)
  {
    this->Makefile = makefile;

    // Now extract the include file regular expression from the makefile.
    this->IncludeFileRegularExpression.compile(
      this->Makefile->GetIncludeRegularExpression());
    this->ComplainFileRegularExpression.compile(
      this->Makefile->GetComplainRegularExpression());

    // Now extract any include paths from the targets
    std::set<std::string> uniqueIncludes;
    std::vector<std::string> orderedAndUniqueIncludes;
    for (auto const& target : this->Makefile->GetTargets()) {
      cmProp incDirProp = target.second.GetProperty("INCLUDE_DIRECTORIES");
      if (!incDirProp) {
        continue;
      }

      std::string incDirs = cmGeneratorExpression::Preprocess(
        *incDirProp, cmGeneratorExpression::StripAllGeneratorExpressions);

      std::vector<std::string> includes = cmExpandedList(incDirs);

      for (std::string& path : includes) {
        this->Makefile->ExpandVariablesInString(path);
        if (uniqueIncludes.insert(path).second) {
          orderedAndUniqueIncludes.push_back(path);
        }
      }
    }

    for (std::string const& inc : orderedAndUniqueIncludes) {
      this->AddSearchPath(inc);
    }
  }

  /**
   * Add a directory to the search path for include files.
   */
  void AddSearchPath(const std::string& path)
  {
    this->IncludeDirectories.push_back(path);
  }

  /**
   * Generate dependencies for the file given.  Returns a pointer to
   * the cmDependInformation object for the file.
   */
  const cmDependInformation* FindDependencies(const std::string& file)
  {
    cmDependInformation* info = this->GetDependInformation(file, "");
    this->GenerateDependInformation(info);
    return info;
  }

protected:
  /**
   * Compute the depend information for this class.
   */

  void DependWalk(cmDependInformation* info)
  {
    cmsys::ifstream fin(info->FullPath.c_str());
    if (!fin) {
      cmSystemTools::Error("error can not open " + info->FullPath);
      return;
    }

    std::string line;
    while (cmSystemTools::GetLineFromStream(fin, line)) {
      if (cmHasLiteralPrefix(line, "#include")) {
        // if it is an include line then create a string class
        size_t qstart = line.find('\"', 8);
        size_t qend;
        // if a quote is not found look for a <
        if (qstart == std::string::npos) {
          qstart = line.find('<', 8);
          // if a < is not found then move on
          if (qstart == std::string::npos) {
            cmSystemTools::Error("unknown include directive " + line);
            continue;
          }
          qend = line.find('>', qstart + 1);
        } else {
          qend = line.find('\"', qstart + 1);
        }
        // extract the file being included
        std::string includeFile = line.substr(qstart + 1, qend - qstart - 1);
        // see if the include matches the regular expression
        if (!this->IncludeFileRegularExpression.find(includeFile)) {
          if (this->Verbose) {
            std::string message =
              cmStrCat("Skipping ", includeFile, " for file ", info->FullPath);
            cmSystemTools::Error(message);
          }
          continue;
        }

        // Add this file and all its dependencies.
        this->AddDependency(info, includeFile);
        /// add the cxx file if it exists
        std::string cxxFile = includeFile;
        std::string::size_type pos = cxxFile.rfind('.');
        if (pos != std::string::npos) {
          std::string root = cxxFile.substr(0, pos);
          cxxFile = root + ".cxx";
          bool found = false;
          // try jumping to .cxx .cpp and .c in order
          if (cmSystemTools::FileExists(cxxFile)) {
            found = true;
          }
          for (std::string const& path : this->IncludeDirectories) {
            if (cmSystemTools::FileExists(cmStrCat(path, "/", cxxFile))) {
              found = true;
            }
          }
          if (!found) {
            cxxFile = root + ".cpp";
            if (cmSystemTools::FileExists(cxxFile)) {
              found = true;
            }
            for (std::string const& path : this->IncludeDirectories) {
              if (cmSystemTools::FileExists(cmStrCat(path, "/", cxxFile))) {
                found = true;
              }
            }
          }
          if (!found) {
            cxxFile = root + ".c";
            if (cmSystemTools::FileExists(cxxFile)) {
              found = true;
            }
            for (std::string const& path : this->IncludeDirectories) {
              if (cmSystemTools::FileExists(cmStrCat(path, "/", cxxFile))) {
                found = true;
              }
            }
          }
          if (!found) {
            cxxFile = root + ".txx";
            if (cmSystemTools::FileExists(cxxFile)) {
              found = true;
            }
            for (std::string const& path : this->IncludeDirectories) {
              if (cmSystemTools::FileExists(cmStrCat(path, "/", cxxFile))) {
                found = true;
              }
            }
          }
          if (found) {
            this->AddDependency(info, cxxFile);
          }
        }
      }
    }
  }

  /**
   * Add a dependency.  Possibly walk it for more dependencies.
   */
  void AddDependency(cmDependInformation* info, const std::string& file)
  {
    cmDependInformation* dependInfo =
      this->GetDependInformation(file, info->PathOnly);
    this->GenerateDependInformation(dependInfo);
    info->AddDependencies(dependInfo);
  }

  /**
   * Fill in the given object with dependency information.  If the
   * information is already complete, nothing is done.
   */
  void GenerateDependInformation(cmDependInformation* info)
  {
    // If dependencies are already done, stop now.
    if (info->DependDone) {
      return;
    }
    // Make sure we don't visit the same file more than once.
    info->DependDone = true;

    const std::string& path = info->FullPath;
    if (path.empty()) {
      cmSystemTools::Error(
        "Attempt to find dependencies for file without path!");
      return;
    }

    bool found = false;

    // If the file exists, use it to find dependency information.
    if (cmSystemTools::FileExists(path, true)) {
      // Use the real file to find its dependencies.
      this->DependWalk(info);
      found = true;
    }

    // See if the cmSourceFile for it has any files specified as
    // dependency hints.
    if (info->SourceFile != nullptr) {

      // Get the cmSourceFile corresponding to this.
      const cmSourceFile& cFile = *(info->SourceFile);
      // See if there are any hints for finding dependencies for the missing
      // file.
      if (!cFile.GetDepends().empty()) {
        // Dependency hints have been given.  Use them to begin the
        // recursion.
        for (std::string const& file : cFile.GetDepends()) {
          this->AddDependency(info, file);
        }

        // Found dependency information.  We are done.
        found = true;
      }
    }

    if (!found) {
      // Try to find the file amongst the sources
      cmSourceFile* srcFile = this->Makefile->GetSource(
        cmSystemTools::GetFilenameWithoutExtension(path));
      if (srcFile) {
        if (srcFile->ResolveFullPath() == path) {
          found = true;
        } else {
          // try to guess which include path to use
          for (std::string incpath : this->IncludeDirectories) {
            if (!incpath.empty() && incpath.back() != '/') {
              incpath += "/";
            }
            incpath += path;
            if (srcFile->ResolveFullPath() == incpath) {
              // set the path to the guessed path
              info->FullPath = incpath;
              found = true;
            }
          }
        }
      }
    }

    if (!found) {
      // Couldn't find any dependency information.
      if (this->ComplainFileRegularExpression.find(info->IncludeName)) {
        cmSystemTools::Error("error cannot find dependencies for " + path);
      } else {
        // Destroy the name of the file so that it won't be output as a
        // dependency.
        info->FullPath.clear();
      }
    }
  }

  /**
   * Get an instance of cmDependInformation corresponding to the given file
   * name.
   */
  cmDependInformation* GetDependInformation(const std::string& file,
                                            const std::string& extraPath)
  {
    // Get the full path for the file so that lookup is unambiguous.
    std::string fullPath = this->FullPath(file, extraPath);

    // Try to find the file's instance of cmDependInformation.
    auto result = this->DependInformationMap.find(fullPath);
    if (result != this->DependInformationMap.end()) {
      // Found an instance, return it.
      return result->second.get();
    }
    // Didn't find an instance.  Create a new one and save it.
    auto info = cm::make_unique<cmDependInformation>();
    auto ptr = info.get();
    info->FullPath = fullPath;
    info->PathOnly = cmSystemTools::GetFilenamePath(fullPath);
    info->IncludeName = file;
    this->DependInformationMap[fullPath] = std::move(info);
    return ptr;
  }

  /**
   * Find the full path name for the given file name.
   * This uses the include directories.
   * TODO: Cache path conversions to reduce FileExists calls.
   */
  std::string FullPath(const std::string& fname, const std::string& extraPath)
  {
    auto m = this->DirectoryToFileToPathMap.find(extraPath);

    if (m != this->DirectoryToFileToPathMap.end()) {
      FileToPathMapType& map = m->second;
      auto p = map.find(fname);
      if (p != map.end()) {
        return p->second;
      }
    }

    if (cmSystemTools::FileExists(fname, true)) {
      std::string fp = cmSystemTools::CollapseFullPath(fname);
      this->DirectoryToFileToPathMap[extraPath][fname] = fp;
      return fp;
    }

    for (std::string path : this->IncludeDirectories) {
      if (!path.empty() && path.back() != '/') {
        path += "/";
      }
      path += fname;
      if (cmSystemTools::FileExists(path, true) &&
          !cmSystemTools::FileIsDirectory(path)) {
        std::string fp = cmSystemTools::CollapseFullPath(path);
        this->DirectoryToFileToPathMap[extraPath][fname] = fp;
        return fp;
      }
    }

    if (!extraPath.empty()) {
      std::string path = extraPath;
      if (!path.empty() && path.back() != '/') {
        path = path + "/";
      }
      path = path + fname;
      if (cmSystemTools::FileExists(path, true) &&
          !cmSystemTools::FileIsDirectory(path)) {
        std::string fp = cmSystemTools::CollapseFullPath(path);
        this->DirectoryToFileToPathMap[extraPath][fname] = fp;
        return fp;
      }
    }

    // Couldn't find the file.
    return fname;
  }

  cmMakefile* Makefile;
  bool Verbose;
  cmsys::RegularExpression IncludeFileRegularExpression;
  cmsys::RegularExpression ComplainFileRegularExpression;
  std::vector<std::string> IncludeDirectories;
  using FileToPathMapType = std::map<std::string, std::string>;
  using DirectoryToFileToPathMapType =
    std::map<std::string, FileToPathMapType>;
  using DependInformationMapType =
    std::map<std::string, std::unique_ptr<cmDependInformation>>;
  DependInformationMapType DependInformationMap;
  DirectoryToFileToPathMapType DirectoryToFileToPathMap;
};

void ListDependencies(cmDependInformation const* info, FILE* fout,
                      std::set<cmDependInformation const*>* visited);
}

// cmOutputRequiredFilesCommand
bool cmOutputRequiredFilesCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  if (args.size() != 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // store the arg for final pass
  const std::string& file = args[0];
  const std::string& outputFile = args[1];

  // compute the list of files
  cmLBDepend md;
  md.SetMakefile(&status.GetMakefile());
  md.AddSearchPath(status.GetMakefile().GetCurrentSourceDirectory());
  // find the depends for a file
  const cmDependInformation* info = md.FindDependencies(file);
  if (info) {
    // write them out
    FILE* fout = cmsys::SystemTools::Fopen(outputFile, "w");
    if (!fout) {
      status.SetError(cmStrCat("Can not open output file: ", outputFile));
      return false;
    }
    std::set<cmDependInformation const*> visited;
    ListDependencies(info, fout, &visited);
    fclose(fout);
  }

  return true;
}

namespace {
void ListDependencies(cmDependInformation const* info, FILE* fout,
                      std::set<cmDependInformation const*>* visited)
{
  // add info to the visited set
  visited->insert(info);
  // now recurse with info's dependencies
  for (cmDependInformation* d : info->DependencySet) {
    if (visited->find(d) == visited->end()) {
      if (!info->FullPath.empty()) {
        std::string tmp = d->FullPath;
        std::string::size_type pos = tmp.rfind('.');
        if (pos != std::string::npos && (tmp.substr(pos) != ".h")) {
          tmp = tmp.substr(0, pos);
          fprintf(fout, "%s\n", d->FullPath.c_str());
        }
      }
      ListDependencies(d, fout, visited);
    }
  }
}
}
