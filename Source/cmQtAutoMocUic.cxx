/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoMocUic.h"

#include <algorithm>
#include <array>
#include <deque>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmCryptoHash.h"
#include "cmMakefile.h"
#include "cmQtAutoGen.h"
#include "cmSystemTools.h"
#include "cmake.h"

#if defined(__APPLE__)
#  include <unistd.h>
#endif

// -- Class methods

std::string cmQtAutoMocUic::BaseSettingsT::AbsoluteBuildPath(
  std::string const& relativePath) const
{
  return FileSys->CollapseFullPath(relativePath, AutogenBuildDir);
}

/**
 * @brief Tries to find the header file to the given file base path by
 * appending different header extensions
 * @return True on success
 */
bool cmQtAutoMocUic::BaseSettingsT::FindHeader(
  std::string& header, std::string const& testBasePath) const
{
  for (std::string const& ext : HeaderExtensions) {
    std::string testFilePath(testBasePath);
    testFilePath.push_back('.');
    testFilePath += ext;
    if (FileSys->FileExists(testFilePath)) {
      header = testFilePath;
      return true;
    }
  }
  return false;
}

bool cmQtAutoMocUic::MocSettingsT::skipped(std::string const& fileName) const
{
  return (!Enabled || (SkipList.find(fileName) != SkipList.end()));
}

/**
 * @brief Returns the first relevant Qt macro name found in the given C++ code
 * @return The name of the Qt macro or an empty string
 */
std::string cmQtAutoMocUic::MocSettingsT::FindMacro(
  std::string const& content) const
{
  for (KeyExpT const& filter : MacroFilters) {
    // Run a simple find string operation before the expensive
    // regular expression check
    if (content.find(filter.Key) != std::string::npos) {
      cmsys::RegularExpressionMatch match;
      if (filter.Exp.find(content.c_str(), match)) {
        // Return macro name on demand
        return filter.Key;
      }
    }
  }
  return std::string();
}

std::string cmQtAutoMocUic::MocSettingsT::MacrosString() const
{
  std::string res;
  const auto itB = MacroFilters.cbegin();
  const auto itE = MacroFilters.cend();
  const auto itL = itE - 1;
  auto itC = itB;
  for (; itC != itE; ++itC) {
    // Separator
    if (itC != itB) {
      if (itC != itL) {
        res += ", ";
      } else {
        res += " or ";
      }
    }
    // Key
    res += itC->Key;
  }
  return res;
}

std::string cmQtAutoMocUic::MocSettingsT::FindIncludedFile(
  std::string const& sourcePath, std::string const& includeString) const
{
  // Search in vicinity of the source
  {
    std::string testPath = sourcePath;
    testPath += includeString;
    if (FileSys->FileExists(testPath)) {
      return FileSys->GetRealPath(testPath);
    }
  }
  // Search in include directories
  for (std::string const& path : IncludePaths) {
    std::string fullPath = path;
    fullPath.push_back('/');
    fullPath += includeString;
    if (FileSys->FileExists(fullPath)) {
      return FileSys->GetRealPath(fullPath);
    }
  }
  // Return empty string
  return std::string();
}

void cmQtAutoMocUic::MocSettingsT::FindDependencies(
  std::string const& content, std::set<std::string>& depends) const
{
  if (!DependFilters.empty() && !content.empty()) {
    for (KeyExpT const& filter : DependFilters) {
      // Run a simple find string check
      if (content.find(filter.Key) != std::string::npos) {
        // Run the expensive regular expression check loop
        const char* contentChars = content.c_str();
        cmsys::RegularExpressionMatch match;
        while (filter.Exp.find(contentChars, match)) {
          {
            std::string dep = match.match(1);
            if (!dep.empty()) {
              depends.emplace(std::move(dep));
            }
          }
          contentChars += match.end();
        }
      }
    }
  }
}

bool cmQtAutoMocUic::UicSettingsT::skipped(std::string const& fileName) const
{
  return (!Enabled || (SkipList.find(fileName) != SkipList.end()));
}

void cmQtAutoMocUic::JobT::LogError(GenT genType,
                                    std::string const& message) const
{
  Gen()->AbortError();
  Gen()->Log().Error(genType, message);
}

void cmQtAutoMocUic::JobT::LogFileError(GenT genType,
                                        std::string const& filename,
                                        std::string const& message) const
{
  Gen()->AbortError();
  Gen()->Log().ErrorFile(genType, filename, message);
}

void cmQtAutoMocUic::JobT::LogCommandError(
  GenT genType, std::string const& message,
  std::vector<std::string> const& command, std::string const& output) const
{
  Gen()->AbortError();
  Gen()->Log().ErrorCommand(genType, message, command, output);
}

bool cmQtAutoMocUic::JobT::RunProcess(GenT genType,
                                      cmWorkerPool::ProcessResultT& result,
                                      std::vector<std::string> const& command)
{
  // Log command
  if (Log().Verbose()) {
    std::string msg = "Running command:\n";
    msg += QuotedCommand(command);
    msg += '\n';
    Log().Info(genType, msg);
  }
  return cmWorkerPool::JobT::RunProcess(result, command,
                                        Gen()->Base().AutogenBuildDir);
}

void cmQtAutoMocUic::JobMocPredefsT::Process()
{
  // (Re)generate moc_predefs.h on demand
  bool generate(false);
  bool fileExists(FileSys().FileExists(Gen()->Moc().PredefsFileAbs));
  if (!fileExists) {
    if (Log().Verbose()) {
      std::string reason = "Generating ";
      reason += Quoted(Gen()->Moc().PredefsFileRel);
      reason += " because it doesn't exist";
      Log().Info(GenT::MOC, reason);
    }
    generate = true;
  } else if (Gen()->Moc().SettingsChanged) {
    if (Log().Verbose()) {
      std::string reason = "Generating ";
      reason += Quoted(Gen()->Moc().PredefsFileRel);
      reason += " because the settings changed.";
      Log().Info(GenT::MOC, reason);
    }
    generate = true;
  }
  if (generate) {
    cmWorkerPool::ProcessResultT result;
    {
      // Compose command
      std::vector<std::string> cmd = Gen()->Moc().PredefsCmd;
      // Add includes
      cmd.insert(cmd.end(), Gen()->Moc().Includes.begin(),
                 Gen()->Moc().Includes.end());
      // Add definitions
      for (std::string const& def : Gen()->Moc().Definitions) {
        cmd.push_back("-D" + def);
      }
      // Execute command
      if (!RunProcess(GenT::MOC, result, cmd)) {
        std::string emsg = "The content generation command for ";
        emsg += Quoted(Gen()->Moc().PredefsFileRel);
        emsg += " failed.\n";
        emsg += result.ErrorMessage;
        LogCommandError(GenT::MOC, emsg, cmd, result.StdOut);
      }
    }

    // (Re)write predefs file only on demand
    if (!result.error()) {
      if (!fileExists ||
          FileSys().FileDiffers(Gen()->Moc().PredefsFileAbs, result.StdOut)) {
        if (FileSys().FileWrite(Gen()->Moc().PredefsFileAbs, result.StdOut)) {
          // Success
        } else {
          std::string emsg = "Writing ";
          emsg += Quoted(Gen()->Moc().PredefsFileRel);
          emsg += " failed.";
          LogFileError(GenT::MOC, Gen()->Moc().PredefsFileAbs, emsg);
        }
      } else {
        // Touch to update the time stamp
        if (Log().Verbose()) {
          std::string msg = "Touching ";
          msg += Quoted(Gen()->Moc().PredefsFileRel);
          msg += ".";
          Log().Info(GenT::MOC, msg);
        }
        FileSys().Touch(Gen()->Moc().PredefsFileAbs);
      }
    }
  }
}

void cmQtAutoMocUic::JobParseT::Process()
{
  if (AutoMoc && Header) {
    // Don't parse header for moc if the file is included by a source already
    if (Gen()->ParallelMocIncluded(FileName)) {
      AutoMoc = false;
    }
  }

  if (AutoMoc || AutoUic) {
    std::string error;
    MetaT meta;
    if (FileSys().FileRead(meta.Content, FileName, &error)) {
      if (!meta.Content.empty()) {
        meta.FileDir = FileSys().SubDirPrefix(FileName);
        meta.FileBase = FileSys().GetFilenameWithoutLastExtension(FileName);

        bool success = true;
        if (AutoMoc) {
          if (Header) {
            success = ParseMocHeader(meta);
          } else {
            success = ParseMocSource(meta);
          }
        }
        if (AutoUic && success) {
          ParseUic(meta);
        }
      } else {
        Log().WarningFile(GenT::GEN, FileName, "The source file is empty");
      }
    } else {
      LogFileError(GenT::GEN, FileName, "Could not read the file: " + error);
    }
  }
}

bool cmQtAutoMocUic::JobParseT::ParseMocSource(MetaT const& meta)
{
  struct JobPre
  {
    bool self;       // source file is self
    bool underscore; // "moc_" style include
    std::string SourceFile;
    std::string IncludeString;
  };

  struct MocInclude
  {
    std::string Inc;  // full include string
    std::string Dir;  // include string directory
    std::string Base; // include string file base
  };

  // Check if this source file contains a relevant macro
  std::string const ownMacro = Gen()->Moc().FindMacro(meta.Content);

  // Extract moc includes from file
  std::deque<MocInclude> mocIncsUsc;
  std::deque<MocInclude> mocIncsDot;
  {
    if (meta.Content.find("moc") != std::string::npos) {
      const char* contentChars = meta.Content.c_str();
      cmsys::RegularExpressionMatch match;
      while (Gen()->Moc().RegExpInclude.find(contentChars, match)) {
        std::string incString = match.match(2);
        std::string incDir(FileSys().SubDirPrefix(incString));
        std::string incBase =
          FileSys().GetFilenameWithoutLastExtension(incString);
        if (cmHasLiteralPrefix(incBase, "moc_")) {
          // moc_<BASE>.cxx
          // Remove the moc_ part from the base name
          mocIncsUsc.emplace_back(MocInclude{
            std::move(incString), std::move(incDir), incBase.substr(4) });
        } else {
          // <BASE>.moc
          mocIncsDot.emplace_back(MocInclude{
            std::move(incString), std::move(incDir), std::move(incBase) });
        }
        // Forward content pointer
        contentChars += match.end();
      }
    }
  }

  // Check if there is anything to do
  if (ownMacro.empty() && mocIncsUsc.empty() && mocIncsDot.empty()) {
    return true;
  }

  bool ownDotMocIncluded = false;
  bool ownMocUscIncluded = false;
  std::deque<JobPre> jobs;

  // Process moc_<BASE>.cxx includes
  for (const MocInclude& mocInc : mocIncsUsc) {
    std::string const header =
      MocFindIncludedHeader(meta.FileDir, mocInc.Dir + mocInc.Base);
    if (!header.empty()) {
      // Check if header is skipped
      if (Gen()->Moc().skipped(header)) {
        continue;
      }
      // Register moc job
      const bool ownMoc = (mocInc.Base == meta.FileBase);
      jobs.emplace_back(JobPre{ ownMoc, true, header, mocInc.Inc });
      // Store meta information for relaxed mode
      if (ownMoc) {
        ownMocUscIncluded = true;
      }
    } else {
      {
        std::string emsg = "The file includes the moc file ";
        emsg += Quoted(mocInc.Inc);
        emsg += ", but the header ";
        emsg += Quoted(MocStringHeaders(mocInc.Base));
        emsg += " could not be found.";
        LogFileError(GenT::MOC, FileName, emsg);
      }
      return false;
    }
  }

  // Process <BASE>.moc includes
  for (const MocInclude& mocInc : mocIncsDot) {
    const bool ownMoc = (mocInc.Base == meta.FileBase);
    if (Gen()->Moc().RelaxedMode) {
      // Relaxed mode
      if (!ownMacro.empty() && ownMoc) {
        // Add self
        jobs.emplace_back(JobPre{ ownMoc, false, FileName, mocInc.Inc });
        ownDotMocIncluded = true;
      } else {
        // In relaxed mode try to find a header instead but issue a warning.
        // This is for KDE4 compatibility
        std::string const header =
          MocFindIncludedHeader(meta.FileDir, mocInc.Dir + mocInc.Base);
        if (!header.empty()) {
          // Check if header is skipped
          if (Gen()->Moc().skipped(header)) {
            continue;
          }
          // Register moc job
          jobs.emplace_back(JobPre{ ownMoc, false, header, mocInc.Inc });
          if (ownMacro.empty()) {
            if (ownMoc) {
              std::string emsg = "The file includes the moc file ";
              emsg += Quoted(mocInc.Inc);
              emsg += ", but does not contain a ";
              emsg += Gen()->Moc().MacrosString();
              emsg += " macro.\nRunning moc on\n  ";
              emsg += Quoted(header);
              emsg += "!\nBetter include ";
              emsg += Quoted("moc_" + mocInc.Base + ".cpp");
              emsg += " for a compatibility with strict mode.\n"
                      "(CMAKE_AUTOMOC_RELAXED_MODE warning)\n";
              Log().WarningFile(GenT::MOC, FileName, emsg);
            } else {
              std::string emsg = "The file includes the moc file ";
              emsg += Quoted(mocInc.Inc);
              emsg += " instead of ";
              emsg += Quoted("moc_" + mocInc.Base + ".cpp");
              emsg += ".\nRunning moc on\n  ";
              emsg += Quoted(header);
              emsg += "!\nBetter include ";
              emsg += Quoted("moc_" + mocInc.Base + ".cpp");
              emsg += " for compatibility with strict mode.\n"
                      "(CMAKE_AUTOMOC_RELAXED_MODE warning)\n";
              Log().WarningFile(GenT::MOC, FileName, emsg);
            }
          }
        } else {
          {
            std::string emsg = "The file includes the moc file ";
            emsg += Quoted(mocInc.Inc);
            emsg += ", which seems to be the moc file from a different "
                    "source file.\nCMAKE_AUTOMOC_RELAXED_MODE: Also a "
                    "matching header ";
            emsg += Quoted(MocStringHeaders(mocInc.Base));
            emsg += " could not be found.";
            LogFileError(GenT::MOC, FileName, emsg);
          }
          return false;
        }
      }
    } else {
      // Strict mode
      if (ownMoc) {
        // Include self
        jobs.emplace_back(JobPre{ ownMoc, false, FileName, mocInc.Inc });
        ownDotMocIncluded = true;
        // Accept but issue a warning if moc isn't required
        if (ownMacro.empty()) {
          std::string emsg = "The file includes the moc file ";
          emsg += Quoted(mocInc.Inc);
          emsg += ", but does not contain a ";
          emsg += Gen()->Moc().MacrosString();
          emsg += " macro.";
          Log().WarningFile(GenT::MOC, FileName, emsg);
        }
      } else {
        // Don't allow <BASE>.moc include other than self in strict mode
        {
          std::string emsg = "The file includes the moc file ";
          emsg += Quoted(mocInc.Inc);
          emsg += ", which seems to be the moc file from a different "
                  "source file.\nThis is not supported. Include ";
          emsg += Quoted(meta.FileBase + ".moc");
          emsg += " to run moc on this source file.";
          LogFileError(GenT::MOC, FileName, emsg);
        }
        return false;
      }
    }
  }

  if (!ownMacro.empty() && !ownDotMocIncluded) {
    // In this case, check whether the scanned file itself contains a
    // Q_OBJECT.
    // If this is the case, the moc_foo.cpp should probably be generated from
    // foo.cpp instead of foo.h, because otherwise it won't build.
    // But warn, since this is not how it is supposed to be used.
    // This is for KDE4 compatibility.
    if (Gen()->Moc().RelaxedMode && ownMocUscIncluded) {
      JobPre uscJobPre;
      // Remove underscore job request
      {
        auto itC = jobs.begin();
        auto itE = jobs.end();
        for (; itC != itE; ++itC) {
          JobPre& job(*itC);
          if (job.self && job.underscore) {
            uscJobPre = std::move(job);
            jobs.erase(itC);
            break;
          }
        }
      }
      // Issue a warning
      {
        std::string emsg = "The file contains a ";
        emsg += ownMacro;
        emsg += " macro, but does not include ";
        emsg += Quoted(meta.FileBase + ".moc");
        emsg += ". Instead it includes ";
        emsg += Quoted(uscJobPre.IncludeString);
        emsg += ".\nRunning moc on\n  ";
        emsg += Quoted(FileName);
        emsg += "!\nBetter include ";
        emsg += Quoted(meta.FileBase + ".moc");
        emsg += " for compatibility with strict mode.\n"
                "(CMAKE_AUTOMOC_RELAXED_MODE warning)";
        Log().WarningFile(GenT::MOC, FileName, emsg);
      }
      // Add own source job
      jobs.emplace_back(
        JobPre{ true, false, FileName, uscJobPre.IncludeString });
    } else {
      // Otherwise always error out since it will not compile.
      {
        std::string emsg = "The file contains a ";
        emsg += ownMacro;
        emsg += " macro, but does not include ";
        emsg += Quoted(meta.FileBase + ".moc");
        emsg += "!\nConsider to\n - add #include \"";
        emsg += meta.FileBase;
        emsg += ".moc\"\n - enable SKIP_AUTOMOC for this file";
        LogFileError(GenT::MOC, FileName, emsg);
      }
      return false;
    }
  }

  // Convert pre jobs to actual jobs
  for (JobPre& jobPre : jobs) {
    cmWorkerPool::JobHandleT jobHandle = cm::make_unique<JobMocT>(
      std::move(jobPre.SourceFile), FileName, std::move(jobPre.IncludeString));
    if (jobPre.self) {
      // Read dependencies from this source
      JobMocT& jobMoc = static_cast<JobMocT&>(*jobHandle);
      Gen()->Moc().FindDependencies(meta.Content, jobMoc.Depends);
      jobMoc.DependsValid = true;
    }
    if (!Gen()->ParallelJobPushMoc(std::move(jobHandle))) {
      return false;
    }
  }
  return true;
}

bool cmQtAutoMocUic::JobParseT::ParseMocHeader(MetaT const& meta)
{
  bool success = true;
  std::string const macroName = Gen()->Moc().FindMacro(meta.Content);
  if (!macroName.empty()) {
    cmWorkerPool::JobHandleT jobHandle = cm::make_unique<JobMocT>(
      std::string(FileName), std::string(), std::string());
    // Read dependencies from this source
    {
      JobMocT& jobMoc = static_cast<JobMocT&>(*jobHandle);
      Gen()->Moc().FindDependencies(meta.Content, jobMoc.Depends);
      jobMoc.DependsValid = true;
    }
    success = Gen()->ParallelJobPushMoc(std::move(jobHandle));
  }
  return success;
}

std::string cmQtAutoMocUic::JobParseT::MocStringHeaders(
  std::string const& fileBase) const
{
  std::string res = fileBase;
  res += ".{";
  res += cmJoin(Gen()->Base().HeaderExtensions, ",");
  res += "}";
  return res;
}

std::string cmQtAutoMocUic::JobParseT::MocFindIncludedHeader(
  std::string const& includerDir, std::string const& includeBase)
{
  std::string header;
  // Search in vicinity of the source
  if (!Gen()->Base().FindHeader(header, includerDir + includeBase)) {
    // Search in include directories
    for (std::string const& path : Gen()->Moc().IncludePaths) {
      std::string fullPath = path;
      fullPath.push_back('/');
      fullPath += includeBase;
      if (Gen()->Base().FindHeader(header, fullPath)) {
        break;
      }
    }
  }
  // Sanitize
  if (!header.empty()) {
    header = FileSys().GetRealPath(header);
  }
  return header;
}

bool cmQtAutoMocUic::JobParseT::ParseUic(MetaT const& meta)
{
  bool success = true;
  if (meta.Content.find("ui_") != std::string::npos) {
    const char* contentChars = meta.Content.c_str();
    cmsys::RegularExpressionMatch match;
    while (Gen()->Uic().RegExpInclude.find(contentChars, match)) {
      if (!ParseUicInclude(meta, match.match(2))) {
        success = false;
        break;
      }
      contentChars += match.end();
    }
  }
  return success;
}

bool cmQtAutoMocUic::JobParseT::ParseUicInclude(MetaT const& meta,
                                                std::string&& includeString)
{
  bool success = false;
  std::string uiInputFile = UicFindIncludedFile(meta, includeString);
  if (!uiInputFile.empty()) {
    if (!Gen()->Uic().skipped(uiInputFile)) {
      cmWorkerPool::JobHandleT jobHandle = cm::make_unique<JobUicT>(
        std::move(uiInputFile), FileName, std::move(includeString));
      success = Gen()->ParallelJobPushUic(std::move(jobHandle));
    } else {
      // A skipped file is successful
      success = true;
    }
  }
  return success;
}

std::string cmQtAutoMocUic::JobParseT::UicFindIncludedFile(
  MetaT const& meta, std::string const& includeString)
{
  std::string res;
  std::string searchFile =
    FileSys().GetFilenameWithoutLastExtension(includeString).substr(3);
  searchFile += ".ui";
  // Collect search paths list
  std::deque<std::string> testFiles;
  {
    std::string const searchPath = FileSys().SubDirPrefix(includeString);

    std::string searchFileFull;
    if (!searchPath.empty()) {
      searchFileFull = searchPath;
      searchFileFull += searchFile;
    }
    // Vicinity of the source
    {
      std::string const sourcePath = meta.FileDir;
      testFiles.push_back(sourcePath + searchFile);
      if (!searchPath.empty()) {
        testFiles.push_back(sourcePath + searchFileFull);
      }
    }
    // AUTOUIC search paths
    if (!Gen()->Uic().SearchPaths.empty()) {
      for (std::string const& sPath : Gen()->Uic().SearchPaths) {
        testFiles.push_back((sPath + "/").append(searchFile));
      }
      if (!searchPath.empty()) {
        for (std::string const& sPath : Gen()->Uic().SearchPaths) {
          testFiles.push_back((sPath + "/").append(searchFileFull));
        }
      }
    }
  }

  // Search for the .ui file!
  for (std::string const& testFile : testFiles) {
    if (FileSys().FileExists(testFile)) {
      res = FileSys().GetRealPath(testFile);
      break;
    }
  }

  // Log error
  if (res.empty()) {
    std::string emsg = "Could not find ";
    emsg += Quoted(searchFile);
    emsg += " in\n";
    for (std::string const& testFile : testFiles) {
      emsg += "  ";
      emsg += Quoted(testFile);
      emsg += "\n";
    }
    LogFileError(GenT::UIC, FileName, emsg);
  }

  return res;
}

void cmQtAutoMocUic::JobPostParseT::Process()
{
  if (Gen()->Moc().Enabled) {
    // Add mocs compilations fence job
    Gen()->WorkerPool().EmplaceJob<JobMocsCompilationT>();
  }
  // Add finish job
  Gen()->WorkerPool().EmplaceJob<JobFinishT>();
}

void cmQtAutoMocUic::JobMocsCompilationT::Process()
{
  // Compose mocs compilation file content
  std::string content =
    "// This file is autogenerated. Changes will be overwritten.\n";
  if (Gen()->MocAutoFiles().empty()) {
    // Placeholder content
    content += "// No files found that require moc or the moc files are "
               "included\n";
    content += "enum some_compilers { need_more_than_nothing };\n";
  } else {
    // Valid content
    char const sbeg = Gen()->Base().MultiConfig ? '<' : '"';
    char const send = Gen()->Base().MultiConfig ? '>' : '"';
    for (std::string const& mocfile : Gen()->MocAutoFiles()) {
      content += "#include ";
      content += sbeg;
      content += mocfile;
      content += send;
      content += '\n';
    }
  }

  std::string const& compAbs = Gen()->Moc().CompFileAbs;
  if (FileSys().FileDiffers(compAbs, content)) {
    // Actually write mocs compilation file
    if (Log().Verbose()) {
      Log().Info(GenT::MOC, "Generating MOC compilation " + compAbs);
    }
    if (!FileSys().FileWrite(compAbs, content)) {
      LogFileError(GenT::MOC, compAbs,
                   "mocs compilation file writing failed.");
    }
  } else if (Gen()->MocAutoFileUpdated()) {
    // Only touch mocs compilation file
    if (Log().Verbose()) {
      Log().Info(GenT::MOC, "Touching mocs compilation " + compAbs);
    }
    FileSys().Touch(compAbs);
  }
}

void cmQtAutoMocUic::JobMocT::FindDependencies(std::string const& content)
{
  Gen()->Moc().FindDependencies(content, Depends);
  DependsValid = true;
}

void cmQtAutoMocUic::JobMocT::Process()
{
  // Compute build file name
  if (!IncludeString.empty()) {
    BuildFile = Gen()->Base().AutogenIncludeDir;
    BuildFile += '/';
    BuildFile += IncludeString;
  } else {
    // Relative build path
    std::string relPath = FileSys().GetFilePathChecksum(SourceFile);
    relPath += "/moc_";
    relPath += FileSys().GetFilenameWithoutLastExtension(SourceFile);

    // Register relative file path with duplication check
    relPath = Gen()->ParallelMocAutoRegister(relPath);

    // Absolute build path
    if (Gen()->Base().MultiConfig) {
      BuildFile = Gen()->Base().AutogenIncludeDir;
      BuildFile += '/';
      BuildFile += relPath;
    } else {
      BuildFile = Gen()->Base().AbsoluteBuildPath(relPath);
    }
  }

  if (UpdateRequired()) {
    GenerateMoc();
  }
}

bool cmQtAutoMocUic::JobMocT::UpdateRequired()
{
  bool const verbose = Log().Verbose();

  // Test if the build file exists
  if (!FileSys().FileExists(BuildFile)) {
    if (verbose) {
      std::string reason = "Generating ";
      reason += Quoted(BuildFile);
      reason += " from its source file ";
      reason += Quoted(SourceFile);
      reason += " because it doesn't exist";
      Log().Info(GenT::MOC, reason);
    }
    return true;
  }

  // Test if any setting changed
  if (Gen()->Moc().SettingsChanged) {
    if (verbose) {
      std::string reason = "Generating ";
      reason += Quoted(BuildFile);
      reason += " from ";
      reason += Quoted(SourceFile);
      reason += " because the MOC settings changed";
      Log().Info(GenT::MOC, reason);
    }
    return true;
  }

  // Test if the moc_predefs file is newer
  if (!Gen()->Moc().PredefsFileAbs.empty()) {
    bool isOlder = false;
    {
      std::string error;
      isOlder = FileSys().FileIsOlderThan(BuildFile,
                                          Gen()->Moc().PredefsFileAbs, &error);
      if (!isOlder && !error.empty()) {
        LogError(GenT::MOC, error);
        return false;
      }
    }
    if (isOlder) {
      if (verbose) {
        std::string reason = "Generating ";
        reason += Quoted(BuildFile);
        reason += " because it's older than: ";
        reason += Quoted(Gen()->Moc().PredefsFileAbs);
        Log().Info(GenT::MOC, reason);
      }
      return true;
    }
  }

  // Test if the source file is newer
  {
    bool isOlder = false;
    {
      std::string error;
      isOlder = FileSys().FileIsOlderThan(BuildFile, SourceFile, &error);
      if (!isOlder && !error.empty()) {
        LogError(GenT::MOC, error);
        return false;
      }
    }
    if (isOlder) {
      if (verbose) {
        std::string reason = "Generating ";
        reason += Quoted(BuildFile);
        reason += " because it's older than its source file ";
        reason += Quoted(SourceFile);
        Log().Info(GenT::MOC, reason);
      }
      return true;
    }
  }

  // Test if a dependency file is newer
  {
    // Read dependencies on demand
    if (!DependsValid) {
      std::string content;
      {
        std::string error;
        if (!FileSys().FileRead(content, SourceFile, &error)) {
          std::string emsg = "Could not read file\n  ";
          emsg += Quoted(SourceFile);
          emsg += "\nrequired by moc include ";
          emsg += Quoted(IncludeString);
          emsg += " in\n  ";
          emsg += Quoted(IncluderFile);
          emsg += ".\n";
          emsg += error;
          LogError(GenT::MOC, emsg);
          return false;
        }
      }
      FindDependencies(content);
    }
    // Check dependency timestamps
    std::string error;
    std::string sourceDir = FileSys().SubDirPrefix(SourceFile);
    for (std::string const& depFileRel : Depends) {
      std::string depFileAbs =
        Gen()->Moc().FindIncludedFile(sourceDir, depFileRel);
      if (!depFileAbs.empty()) {
        if (FileSys().FileIsOlderThan(BuildFile, depFileAbs, &error)) {
          if (verbose) {
            std::string reason = "Generating ";
            reason += Quoted(BuildFile);
            reason += " from ";
            reason += Quoted(SourceFile);
            reason += " because it is older than it's dependency file ";
            reason += Quoted(depFileAbs);
            Log().Info(GenT::MOC, reason);
          }
          return true;
        }
        if (!error.empty()) {
          LogError(GenT::MOC, error);
          return false;
        }
      } else {
        std::string message = "Could not find dependency file ";
        message += Quoted(depFileRel);
        Log().WarningFile(GenT::MOC, SourceFile, message);
      }
    }
  }

  return false;
}

void cmQtAutoMocUic::JobMocT::GenerateMoc()
{
  // Make sure the parent directory exists
  if (!FileSys().MakeParentDirectory(BuildFile)) {
    LogFileError(GenT::MOC, BuildFile, "Could not create parent directory.");
    return;
  }
  {
    // Compose moc command
    std::vector<std::string> cmd;
    cmd.push_back(Gen()->Moc().Executable);
    // Add options
    cmd.insert(cmd.end(), Gen()->Moc().AllOptions.begin(),
               Gen()->Moc().AllOptions.end());
    // Add predefs include
    if (!Gen()->Moc().PredefsFileAbs.empty()) {
      cmd.emplace_back("--include");
      cmd.push_back(Gen()->Moc().PredefsFileAbs);
    }
    cmd.emplace_back("-o");
    cmd.push_back(BuildFile);
    cmd.push_back(SourceFile);

    // Execute moc command
    cmWorkerPool::ProcessResultT result;
    if (RunProcess(GenT::MOC, result, cmd)) {
      // Moc command success
      // Print moc output
      if (!result.StdOut.empty()) {
        Log().Info(GenT::MOC, result.StdOut);
      }
      // Notify the generator that a not included file changed (on demand)
      if (IncludeString.empty()) {
        Gen()->ParallelMocAutoUpdated();
      }
    } else {
      // Moc command failed
      {
        std::string emsg = "The moc process failed to compile\n  ";
        emsg += Quoted(SourceFile);
        emsg += "\ninto\n  ";
        emsg += Quoted(BuildFile);
        emsg += ".\n";
        emsg += result.ErrorMessage;
        LogCommandError(GenT::MOC, emsg, cmd, result.StdOut);
      }
      FileSys().FileRemove(BuildFile);
    }
  }
}

void cmQtAutoMocUic::JobUicT::Process()
{
  // Compute build file name
  BuildFile = Gen()->Base().AutogenIncludeDir;
  BuildFile += '/';
  BuildFile += IncludeString;

  if (UpdateRequired()) {
    GenerateUic();
  }
}

bool cmQtAutoMocUic::JobUicT::UpdateRequired()
{
  bool const verbose = Log().Verbose();

  // Test if the build file exists
  if (!FileSys().FileExists(BuildFile)) {
    if (verbose) {
      std::string reason = "Generating ";
      reason += Quoted(BuildFile);
      reason += " from its source file ";
      reason += Quoted(SourceFile);
      reason += " because it doesn't exist";
      Log().Info(GenT::UIC, reason);
    }
    return true;
  }

  // Test if the uic settings changed
  if (Gen()->Uic().SettingsChanged) {
    if (verbose) {
      std::string reason = "Generating ";
      reason += Quoted(BuildFile);
      reason += " from ";
      reason += Quoted(SourceFile);
      reason += " because the UIC settings changed";
      Log().Info(GenT::UIC, reason);
    }
    return true;
  }

  // Test if the source file is newer
  {
    bool isOlder = false;
    {
      std::string error;
      isOlder = FileSys().FileIsOlderThan(BuildFile, SourceFile, &error);
      if (!isOlder && !error.empty()) {
        LogError(GenT::UIC, error);
        return false;
      }
    }
    if (isOlder) {
      if (verbose) {
        std::string reason = "Generating ";
        reason += Quoted(BuildFile);
        reason += " because it's older than its source file ";
        reason += Quoted(SourceFile);
        Log().Info(GenT::UIC, reason);
      }
      return true;
    }
  }

  return false;
}

void cmQtAutoMocUic::JobUicT::GenerateUic()
{
  // Make sure the parent directory exists
  if (!FileSys().MakeParentDirectory(BuildFile)) {
    LogFileError(GenT::UIC, BuildFile, "Could not create parent directory.");
    return;
  }
  {
    // Compose uic command
    std::vector<std::string> cmd;
    cmd.push_back(Gen()->Uic().Executable);
    {
      std::vector<std::string> allOpts = Gen()->Uic().TargetOptions;
      auto optionIt = Gen()->Uic().Options.find(SourceFile);
      if (optionIt != Gen()->Uic().Options.end()) {
        UicMergeOptions(allOpts, optionIt->second,
                        (Gen()->Base().QtVersionMajor == 5));
      }
      cmd.insert(cmd.end(), allOpts.begin(), allOpts.end());
    }
    cmd.emplace_back("-o");
    cmd.emplace_back(BuildFile);
    cmd.emplace_back(SourceFile);

    cmWorkerPool::ProcessResultT result;
    if (RunProcess(GenT::UIC, result, cmd)) {
      // Uic command success
      // Print uic output
      if (!result.StdOut.empty()) {
        Log().Info(GenT::UIC, result.StdOut);
      }
    } else {
      // Uic command failed
      {
        std::string emsg = "The uic process failed to compile\n  ";
        emsg += Quoted(SourceFile);
        emsg += "\ninto\n  ";
        emsg += Quoted(BuildFile);
        emsg += "\nincluded by\n  ";
        emsg += Quoted(IncluderFile);
        emsg += ".\n";
        emsg += result.ErrorMessage;
        LogCommandError(GenT::UIC, emsg, cmd, result.StdOut);
      }
      FileSys().FileRemove(BuildFile);
    }
  }
}

void cmQtAutoMocUic::JobFinishT::Process()
{
  Gen()->AbortSuccess();
}

cmQtAutoMocUic::cmQtAutoMocUic()
  : Base_(&FileSys())
  , Moc_(&FileSys())
{
  // Precompile regular expressions
  Moc_.RegExpInclude.compile(
    "(^|\n)[ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
  Uic_.RegExpInclude.compile("(^|\n)[ \t]*#[ \t]*include[ \t]+"
                             "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

cmQtAutoMocUic::~cmQtAutoMocUic() = default;

bool cmQtAutoMocUic::Init(cmMakefile* makefile)
{
  // -- Meta
  Base_.HeaderExtensions = makefile->GetCMakeInstance()->GetHeaderExtensions();

  // Utility lambdas
  auto InfoGet = [makefile](const char* key) {
    return makefile->GetSafeDefinition(key);
  };
  auto InfoGetBool = [makefile](const char* key) {
    return makefile->IsOn(key);
  };
  auto InfoGetList = [makefile](const char* key) -> std::vector<std::string> {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition(key), list);
    return list;
  };
  auto InfoGetLists =
    [makefile](const char* key) -> std::vector<std::vector<std::string>> {
    std::vector<std::vector<std::string>> lists;
    {
      std::string const value = makefile->GetSafeDefinition(key);
      std::string::size_type pos = 0;
      while (pos < value.size()) {
        std::string::size_type next = value.find(ListSep, pos);
        std::string::size_type length =
          (next != std::string::npos) ? next - pos : value.size() - pos;
        // Remove enclosing braces
        if (length >= 2) {
          std::string::const_iterator itBeg = value.begin() + (pos + 1);
          std::string::const_iterator itEnd = itBeg + (length - 2);
          {
            std::string subValue(itBeg, itEnd);
            std::vector<std::string> list;
            cmSystemTools::ExpandListArgument(subValue, list);
            lists.push_back(std::move(list));
          }
        }
        pos += length;
        pos += ListSep.size();
      }
    }
    return lists;
  };
  auto InfoGetConfig = [makefile, this](const char* key) -> std::string {
    const char* valueConf = nullptr;
    {
      std::string keyConf = key;
      keyConf += '_';
      keyConf += InfoConfig();
      valueConf = makefile->GetDefinition(keyConf);
    }
    if (valueConf == nullptr) {
      return makefile->GetSafeDefinition(key);
    }
    return std::string(valueConf);
  };
  auto InfoGetConfigList =
    [&InfoGetConfig](const char* key) -> std::vector<std::string> {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(InfoGetConfig(key), list);
    return list;
  };

  // -- Read info file
  if (!makefile->ReadListFile(InfoFile())) {
    Log().ErrorFile(GenT::GEN, InfoFile(), "File processing failed");
    return false;
  }

  // -- Meta
  Log().RaiseVerbosity(InfoGet("AM_VERBOSITY"));
  Base_.MultiConfig = InfoGetBool("AM_MULTI_CONFIG");
  {
    unsigned long num = Base_.NumThreads;
    if (cmSystemTools::StringToULong(InfoGet("AM_PARALLEL").c_str(), &num)) {
      num = std::max<unsigned long>(num, 1);
      num = std::min<unsigned long>(num, ParallelMax);
      Base_.NumThreads = static_cast<unsigned int>(num);
    }
    WorkerPool_.SetThreadCount(Base_.NumThreads);
  }

  // - Files and directories
  Base_.ProjectSourceDir = InfoGet("AM_CMAKE_SOURCE_DIR");
  Base_.ProjectBinaryDir = InfoGet("AM_CMAKE_BINARY_DIR");
  Base_.CurrentSourceDir = InfoGet("AM_CMAKE_CURRENT_SOURCE_DIR");
  Base_.CurrentBinaryDir = InfoGet("AM_CMAKE_CURRENT_BINARY_DIR");
  Base_.IncludeProjectDirsBefore =
    InfoGetBool("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  Base_.AutogenBuildDir = InfoGet("AM_BUILD_DIR");
  if (Base_.AutogenBuildDir.empty()) {
    Log().ErrorFile(GenT::GEN, InfoFile(), "Autogen build directory missing");
    return false;
  }
  // include directory
  Base_.AutogenIncludeDir = InfoGetConfig("AM_INCLUDE_DIR");
  if (Base_.AutogenIncludeDir.empty()) {
    Log().ErrorFile(GenT::GEN, InfoFile(),
                    "Autogen include directory missing");
    return false;
  }

  // - Files
  SettingsFile_ = InfoGetConfig("AM_SETTINGS_FILE");
  if (SettingsFile_.empty()) {
    Log().ErrorFile(GenT::GEN, InfoFile(), "Settings file name missing");
    return false;
  }

  // - Qt environment
  {
    unsigned long qtv = Base_.QtVersionMajor;
    if (cmSystemTools::StringToULong(InfoGet("AM_QT_VERSION_MAJOR").c_str(),
                                     &qtv)) {
      Base_.QtVersionMajor = static_cast<unsigned int>(qtv);
    }
  }

  // - Moc
  Moc_.Executable = InfoGet("AM_QT_MOC_EXECUTABLE");
  Moc_.Enabled = !Moc().Executable.empty();
  if (Moc().Enabled) {
    for (std::string& sfl : InfoGetList("AM_MOC_SKIP")) {
      Moc_.SkipList.insert(std::move(sfl));
    }
    Moc_.Definitions = InfoGetConfigList("AM_MOC_DEFINITIONS");
    Moc_.IncludePaths = InfoGetConfigList("AM_MOC_INCLUDES");
    Moc_.Options = InfoGetList("AM_MOC_OPTIONS");
    Moc_.RelaxedMode = InfoGetBool("AM_MOC_RELAXED_MODE");
    for (std::string const& item : InfoGetList("AM_MOC_MACRO_NAMES")) {
      Moc_.MacroFilters.emplace_back(
        item, ("[\n][ \t]*{?[ \t]*" + item).append("[^a-zA-Z0-9_]"));
    }
    {
      auto pushFilter = [this](std::string const& key, std::string const& exp,
                               std::string& error) {
        if (!key.empty()) {
          if (!exp.empty()) {
            Moc_.DependFilters.emplace_back();
            KeyExpT& filter(Moc_.DependFilters.back());
            if (filter.Exp.compile(exp)) {
              filter.Key = key;
            } else {
              error = "Regular expression compiling failed";
            }
          } else {
            error = "Regular expression is empty";
          }
        } else {
          error = "Key is empty";
        }
        if (!error.empty()) {
          error = ("AUTOMOC_DEPEND_FILTERS: " + error);
          error += "\n";
          error += "  Key: ";
          error += Quoted(key);
          error += "\n";
          error += "  Exp: ";
          error += Quoted(exp);
          error += "\n";
        }
      };

      std::string error;
      // Insert default filter for Q_PLUGIN_METADATA
      if (Base().QtVersionMajor != 4) {
        pushFilter("Q_PLUGIN_METADATA",
                   "[\n][ \t]*Q_PLUGIN_METADATA[ \t]*\\("
                   "[^\\)]*FILE[ \t]*\"([^\"]+)\"",
                   error);
      }
      // Insert user defined dependency filters
      {
        std::vector<std::string> flts = InfoGetList("AM_MOC_DEPEND_FILTERS");
        if ((flts.size() % 2) == 0) {
          for (std::vector<std::string>::iterator itC = flts.begin(),
                                                  itE = flts.end();
               itC != itE; itC += 2) {
            pushFilter(*itC, *(itC + 1), error);
            if (!error.empty()) {
              break;
            }
          }
        } else {
          Log().ErrorFile(
            GenT::MOC, InfoFile(),
            "AUTOMOC_DEPEND_FILTERS list size is not a multiple of 2");
          return false;
        }
      }
      if (!error.empty()) {
        Log().ErrorFile(GenT::MOC, InfoFile(), error);
        return false;
      }
    }
    Moc_.PredefsCmd = InfoGetList("AM_MOC_PREDEFS_CMD");
    // Install moc predefs job
    if (!Moc().PredefsCmd.empty()) {
      WorkerPool().EmplaceJob<JobMocPredefsT>();
    }
  }

  // - Uic
  Uic_.Executable = InfoGet("AM_QT_UIC_EXECUTABLE");
  Uic_.Enabled = !Uic().Executable.empty();
  if (Uic().Enabled) {
    for (std::string& sfl : InfoGetList("AM_UIC_SKIP")) {
      Uic_.SkipList.insert(std::move(sfl));
    }
    Uic_.SearchPaths = InfoGetList("AM_UIC_SEARCH_PATHS");
    Uic_.TargetOptions = InfoGetConfigList("AM_UIC_TARGET_OPTIONS");
    {
      auto sources = InfoGetList("AM_UIC_OPTIONS_FILES");
      auto options = InfoGetLists("AM_UIC_OPTIONS_OPTIONS");
      // Compare list sizes
      if (sources.size() != options.size()) {
        std::ostringstream ost;
        ost << "files/options lists sizes mismatch (" << sources.size() << "/"
            << options.size() << ")";
        Log().ErrorFile(GenT::UIC, InfoFile(), ost.str());
        return false;
      }
      auto fitEnd = sources.cend();
      auto fit = sources.begin();
      auto oit = options.begin();
      while (fit != fitEnd) {
        Uic_.Options[*fit] = std::move(*oit);
        ++fit;
        ++oit;
      }
    }
  }

  // - Headers and sources
  // Add sources
  {
    auto addSource = [this](std::string&& src, bool moc, bool uic) {
      WorkerPool().EmplaceJob<JobParseT>(std::move(src), moc, uic, false);
    };
    for (std::string& src : InfoGetList("AM_SOURCES")) {
      addSource(std::move(src), true, true);
    }
    if (Moc().Enabled) {
      for (std::string& src : InfoGetList("AM_MOC_SOURCES")) {
        addSource(std::move(src), true, false);
      }
    }
    if (Uic().Enabled) {
      for (std::string& src : InfoGetList("AM_UIC_SOURCES")) {
        addSource(std::move(src), false, true);
      }
    }
  }
  // Add Fence job
  WorkerPool().EmplaceJob<JobFenceT>();
  // Add headers
  {
    auto addHeader = [this](std::string&& hdr, bool moc, bool uic) {
      WorkerPool().EmplaceJob<JobParseT>(std::move(hdr), moc, uic, true);
    };
    for (std::string& hdr : InfoGetList("AM_HEADERS")) {
      addHeader(std::move(hdr), true, true);
    }
    if (Moc().Enabled) {
      for (std::string& hdr : InfoGetList("AM_MOC_HEADERS")) {
        addHeader(std::move(hdr), true, false);
      }
    }
    if (Uic().Enabled) {
      for (std::string& hdr : InfoGetList("AM_UIC_HEADERS")) {
        addHeader(std::move(hdr), false, true);
      }
    }
  }
  // Addpost parse fence job
  WorkerPool().EmplaceJob<JobPostParseT>();

  // Init derived information
  // ------------------------

  // Init file path checksum generator
  FileSys().setupFilePathChecksum(
    Base().CurrentSourceDir, Base().CurrentBinaryDir, Base().ProjectSourceDir,
    Base().ProjectBinaryDir);

  // Moc variables
  if (Moc().Enabled) {
    // Mocs compilation file
    Moc_.CompFileAbs = Base().AbsoluteBuildPath("mocs_compilation.cpp");

    // Moc predefs file
    if (!Moc_.PredefsCmd.empty()) {
      Moc_.PredefsFileRel = "moc_predefs";
      if (Base_.MultiConfig) {
        Moc_.PredefsFileRel += '_';
        Moc_.PredefsFileRel += InfoConfig();
      }
      Moc_.PredefsFileRel += ".h";
      Moc_.PredefsFileAbs = Base_.AbsoluteBuildPath(Moc().PredefsFileRel);
    }

    // Sort include directories on demand
    if (Base().IncludeProjectDirsBefore) {
      // Move strings to temporary list
      std::list<std::string> includes;
      includes.insert(includes.end(), Moc().IncludePaths.begin(),
                      Moc().IncludePaths.end());
      Moc_.IncludePaths.clear();
      Moc_.IncludePaths.reserve(includes.size());
      // Append project directories only
      {
        std::array<std::string const*, 2> const movePaths = {
          { &Base().ProjectBinaryDir, &Base().ProjectSourceDir }
        };
        for (std::string const* ppath : movePaths) {
          std::list<std::string>::iterator it = includes.begin();
          while (it != includes.end()) {
            std::string const& path = *it;
            if (cmSystemTools::StringStartsWith(path, ppath->c_str())) {
              Moc_.IncludePaths.push_back(path);
              it = includes.erase(it);
            } else {
              ++it;
            }
          }
        }
      }
      // Append remaining directories
      Moc_.IncludePaths.insert(Moc_.IncludePaths.end(), includes.begin(),
                               includes.end());
    }
    // Compose moc includes list
    {
      std::set<std::string> frameworkPaths;
      for (std::string const& path : Moc().IncludePaths) {
        Moc_.Includes.push_back("-I" + path);
        // Extract framework path
        if (cmHasLiteralSuffix(path, ".framework/Headers")) {
          // Go up twice to get to the framework root
          std::vector<std::string> pathComponents;
          FileSys().SplitPath(path, pathComponents);
          std::string frameworkPath = FileSys().JoinPath(
            pathComponents.begin(), pathComponents.end() - 2);
          frameworkPaths.insert(frameworkPath);
        }
      }
      // Append framework includes
      for (std::string const& path : frameworkPaths) {
        Moc_.Includes.emplace_back("-F");
        Moc_.Includes.push_back(path);
      }
    }
    // Setup single list with all options
    {
      // Add includes
      Moc_.AllOptions.insert(Moc_.AllOptions.end(), Moc().Includes.begin(),
                             Moc().Includes.end());
      // Add definitions
      for (std::string const& def : Moc().Definitions) {
        Moc_.AllOptions.push_back("-D" + def);
      }
      // Add options
      Moc_.AllOptions.insert(Moc_.AllOptions.end(), Moc().Options.begin(),
                             Moc().Options.end());
    }
  }

  return true;
}

bool cmQtAutoMocUic::Process()
{
  SettingsFileRead();
  if (!CreateDirectories()) {
    return false;
  }
  if (!WorkerPool_.Process(this)) {
    return false;
  }
  if (JobError_) {
    return false;
  }
  return SettingsFileWrite();
}

void cmQtAutoMocUic::SettingsFileRead()
{
  // Compose current settings strings
  {
    cmCryptoHash crypt(cmCryptoHash::AlgoSHA256);
    std::string const sep(" ~~~ ");
    if (Moc_.Enabled) {
      std::string str;
      str += Moc().Executable;
      str += sep;
      str += cmJoin(Moc().AllOptions, ";");
      str += sep;
      str += Base().IncludeProjectDirsBefore ? "TRUE" : "FALSE";
      str += sep;
      str += cmJoin(Moc().PredefsCmd, ";");
      str += sep;
      SettingsStringMoc_ = crypt.HashString(str);
    }
    if (Uic().Enabled) {
      std::string str;
      str += Uic().Executable;
      str += sep;
      str += cmJoin(Uic().TargetOptions, ";");
      for (const auto& item : Uic().Options) {
        str += sep;
        str += item.first;
        str += sep;
        str += cmJoin(item.second, ";");
      }
      str += sep;
      SettingsStringUic_ = crypt.HashString(str);
    }
  }

  // Read old settings and compare
  {
    std::string content;
    if (FileSys().FileRead(content, SettingsFile_)) {
      if (Moc().Enabled) {
        if (SettingsStringMoc_ != SettingsFind(content, "moc")) {
          Moc_.SettingsChanged = true;
        }
      }
      if (Uic().Enabled) {
        if (SettingsStringUic_ != SettingsFind(content, "uic")) {
          Uic_.SettingsChanged = true;
        }
      }
      // In case any setting changed remove the old settings file.
      // This triggers a full rebuild on the next run if the current
      // build is aborted before writing the current settings in the end.
      if (Moc().SettingsChanged || Uic().SettingsChanged) {
        FileSys().FileRemove(SettingsFile_);
      }
    } else {
      // Settings file read failed
      if (Moc().Enabled) {
        Moc_.SettingsChanged = true;
      }
      if (Uic().Enabled) {
        Uic_.SettingsChanged = true;
      }
    }
  }
}

bool cmQtAutoMocUic::SettingsFileWrite()
{
  // Only write if any setting changed
  if (Moc().SettingsChanged || Uic().SettingsChanged) {
    if (Log().Verbose()) {
      Log().Info(GenT::GEN, "Writing settings file " + Quoted(SettingsFile_));
    }
    // Compose settings file content
    std::string content;
    {
      auto SettingAppend = [&content](const char* key,
                                      std::string const& value) {
        if (!value.empty()) {
          content += key;
          content += ':';
          content += value;
          content += '\n';
        }
      };
      SettingAppend("moc", SettingsStringMoc_);
      SettingAppend("uic", SettingsStringUic_);
    }
    // Write settings file
    std::string error;
    if (!FileSys().FileWrite(SettingsFile_, content, &error)) {
      Log().ErrorFile(GenT::GEN, SettingsFile_,
                      "Settings file writing failed. " + error);
      // Remove old settings file to trigger a full rebuild on the next run
      FileSys().FileRemove(SettingsFile_);
      return false;
    }
  }
  return true;
}

bool cmQtAutoMocUic::CreateDirectories()
{
  // Create AUTOGEN include directory
  if (!FileSys().MakeDirectory(Base().AutogenIncludeDir)) {
    Log().ErrorFile(GenT::GEN, Base().AutogenIncludeDir,
                    "Could not create directory.");
    return false;
  }
  return true;
}

// Private method that requires cmQtAutoMocUic::JobsMutex_ to be
// locked
void cmQtAutoMocUic::Abort(bool error)
{
  if (error) {
    JobError_.store(true);
  }
  WorkerPool_.Abort();
}

bool cmQtAutoMocUic::ParallelJobPushMoc(cmWorkerPool::JobHandleT&& jobHandle)
{
  JobMocT const& mocJob(static_cast<JobMocT&>(*jobHandle));
  // Do additional tests if this is an included moc job
  if (!mocJob.IncludeString.empty()) {
    std::lock_guard<std::mutex> guard(MocMetaMutex_);
    // Register included moc file
    MocIncludedFiles_.emplace(mocJob.SourceFile);

    // Check if the same moc file would be generated from a different
    // source file.
    auto const range = MocIncludes_.equal_range(mocJob.IncludeString);
    for (auto it = range.first; it != range.second; ++it) {
      if (it->second[0] == mocJob.SourceFile) {
        // The output file already gets generated
        return true;
      }
      {
        // The output file already gets generated - from a different source
        // file!
        std::string error = "The two source files\n  ";
        error += Quoted(mocJob.IncluderFile);
        error += " and\n  ";
        error += Quoted(it->second[1]);
        error += "\ncontain the same moc include string ";
        error += Quoted(mocJob.IncludeString);
        error += "\nbut the moc file would be generated from different "
                 "source files\n  ";
        error += Quoted(mocJob.SourceFile);
        error += " and\n  ";
        error += Quoted(it->second[0]);
        error += ".\nConsider to\n"
                 "- not include the \"moc_<NAME>.cpp\" file\n"
                 "- add a directory prefix to a \"<NAME>.moc\" include "
                 "(e.g \"sub/<NAME>.moc\")\n"
                 "- rename the source file(s)\n";
        Log().Error(GenT::MOC, error);
        AbortError();
        return false;
      }
    }

    // We're still here so register this job
    MocIncludes_.emplace_hint(range.first, mocJob.IncludeString,
                              std::array<std::string, 2>{
                                { mocJob.SourceFile, mocJob.IncluderFile } });
  }
  return WorkerPool_.PushJob(std::move(jobHandle));
}

bool cmQtAutoMocUic::ParallelJobPushUic(cmWorkerPool::JobHandleT&& jobHandle)
{
  const JobUicT& uicJob(static_cast<JobUicT&>(*jobHandle));
  {
    std::lock_guard<std::mutex> guard(UicMetaMutex_);
    // Check if the same uic file would be generated from a different
    // source file.
    auto const range = UicIncludes_.equal_range(uicJob.IncludeString);
    for (auto it = range.first; it != range.second; ++it) {
      if (it->second[0] == uicJob.SourceFile) {
        // The output file already gets generated
        return true;
      }
      {
        // The output file already gets generated - from a different .ui
        // file!
        std::string error = "The two source files\n  ";
        error += Quoted(uicJob.IncluderFile);
        error += " and\n  ";
        error += Quoted(it->second[1]);
        error += "\ncontain the same uic include string ";
        error += Quoted(uicJob.IncludeString);
        error += "\nbut the uic file would be generated from different "
                 "source files\n  ";
        error += Quoted(uicJob.SourceFile);
        error += " and\n  ";
        error += Quoted(it->second[0]);
        error +=
          ".\nConsider to\n"
          "- add a directory prefix to a \"ui_<NAME>.h\" include "
          "(e.g \"sub/ui_<NAME>.h\")\n"
          "- rename the <NAME>.ui file(s) and adjust the \"ui_<NAME>.h\" "
          "include(s)\n";
        Log().Error(GenT::UIC, error);
        AbortError();
        return false;
      }
    }

    // We're still here so register this job
    UicIncludes_.emplace_hint(range.first, uicJob.IncludeString,
                              std::array<std::string, 2>{
                                { uicJob.SourceFile, uicJob.IncluderFile } });
  }
  return WorkerPool_.PushJob(std::move(jobHandle));
}

bool cmQtAutoMocUic::ParallelMocIncluded(std::string const& sourceFile)
{
  std::lock_guard<std::mutex> guard(MocMetaMutex_);
  return (MocIncludedFiles_.find(sourceFile) != MocIncludedFiles_.end());
}

std::string cmQtAutoMocUic::ParallelMocAutoRegister(
  std::string const& baseName)
{
  std::string res;
  {
    std::lock_guard<std::mutex> mocLock(MocMetaMutex_);
    res = baseName;
    res += ".cpp";
    if (MocAutoFiles_.find(res) == MocAutoFiles_.end()) {
      MocAutoFiles_.emplace(res);
    } else {
      // Append number suffix to the file name
      for (unsigned int ii = 2; ii != 1024; ++ii) {
        res = baseName;
        res += '_';
        res += std::to_string(ii);
        res += ".cpp";
        if (MocAutoFiles_.find(res) == MocAutoFiles_.end()) {
          MocAutoFiles_.emplace(res);
          break;
        }
      }
    }
  }
  return res;
}
