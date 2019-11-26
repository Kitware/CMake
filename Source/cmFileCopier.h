/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileCopier_h
#define cmFileCopier_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmFileTimeCache.h"

class cmExecutionStatus;
class cmMakefile;

// File installation helper class.
struct cmFileCopier
{
  cmFileCopier(cmExecutionStatus& status, const char* name = "COPY");
  virtual ~cmFileCopier();

  bool Run(std::vector<std::string> const& args);

protected:
  cmExecutionStatus& Status;
  cmMakefile* Makefile;
  const char* Name;
  bool Always;
  cmFileTimeCache FileTimes;

  // Whether to install a file not matching any expression.
  bool MatchlessFiles;

  // Permissions for files and directories installed by this object.
  mode_t FilePermissions;
  mode_t DirPermissions;

  // Properties set by pattern and regex match rules.
  struct MatchProperties
  {
    bool Exclude = false;
    mode_t Permissions = 0;
  };
  struct MatchRule
  {
    cmsys::RegularExpression Regex;
    MatchProperties Properties;
    std::string RegexString;
    MatchRule(std::string const& regex)
      : Regex(regex)
      , RegexString(regex)
    {
    }
  };
  std::vector<MatchRule> MatchRules;

  // Get the properties from rules matching this input file.
  MatchProperties CollectMatchProperties(const std::string& file);

  bool SetPermissions(const std::string& toFile, mode_t permissions);

  // Translate an argument to a permissions bit.
  bool CheckPermissions(std::string const& arg, mode_t& permissions);

  bool InstallSymlinkChain(std::string& fromFile, std::string& toFile);
  bool InstallSymlink(const std::string& fromFile, const std::string& toFile);
  bool InstallFile(const std::string& fromFile, const std::string& toFile,
                   MatchProperties match_properties);
  bool InstallDirectory(const std::string& source,
                        const std::string& destination,
                        MatchProperties match_properties);
  virtual bool Install(const std::string& fromFile, const std::string& toFile);
  virtual std::string const& ToName(std::string const& fromName);

  enum Type
  {
    TypeFile,
    TypeDir,
    TypeLink
  };
  virtual void ReportCopy(const std::string&, Type, bool) {}
  virtual bool ReportMissing(const std::string& fromFile);

  MatchRule* CurrentMatchRule;
  bool UseGivenPermissionsFile;
  bool UseGivenPermissionsDir;
  bool UseSourcePermissions;
  bool FollowSymlinkChain;
  std::string Destination;
  std::string FilesFromDir;
  std::vector<std::string> Files;
  int Doing;

  virtual bool Parse(std::vector<std::string> const& args);
  enum
  {
    DoingNone,
    DoingError,
    DoingDestination,
    DoingFilesFromDir,
    DoingFiles,
    DoingPattern,
    DoingRegex,
    DoingPermissionsFile,
    DoingPermissionsDir,
    DoingPermissionsMatch,
    DoingLast1
  };
  virtual bool CheckKeyword(std::string const& arg);
  virtual bool CheckValue(std::string const& arg);

  void NotBeforeMatch(std::string const& arg);
  void NotAfterMatch(std::string const& arg);
  virtual void DefaultFilePermissions();
  virtual void DefaultDirectoryPermissions();

  bool GetDefaultDirectoryPermissions(mode_t** mode);
};

#endif
