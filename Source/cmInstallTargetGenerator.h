/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"

class cmGeneratorTarget;
class cmLocalGenerator;

/** \class cmInstallTargetGenerator
 * \brief Generate target installation rules.
 */
class cmInstallTargetGenerator : public cmInstallGenerator
{
public:
  cmInstallTargetGenerator(
    std::string targetName, std::string const& dest, bool implib,
    std::string file_permissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool exclude_from_all,
    bool optional, cmListFileBacktrace backtrace = cmListFileBacktrace());
  ~cmInstallTargetGenerator() override;

  /** Select the policy for installing shared library linkable name
      symlinks.  */
  enum NamelinkModeType
  {
    NamelinkModeNone,
    NamelinkModeOnly,
    NamelinkModeSkip
  };
  void SetNamelinkMode(NamelinkModeType mode) { this->NamelinkMode = mode; }
  void SetImportlinkMode(NamelinkModeType mode)
  {
    this->ImportlinkMode = mode;
  }

  std::string GetInstallFilename(const std::string& config) const;

  void GetInstallObjectNames(std::string const& config,
                             std::vector<std::string>& objects) const;

  enum NameType
  {
    NameNormal,
    NameImplib,
    NameSO,
    NameReal,
    NameImplibReal
  };

  static std::string GetInstallFilename(const cmGeneratorTarget* target,
                                        const std::string& config,
                                        NameType nameType = NameNormal);

  bool Compute(cmLocalGenerator* lg) override;

  cmGeneratorTarget* GetTarget() const { return this->Target; }

  bool IsImportLibrary() const { return this->ImportLibrary; }

  std::string GetDestination(std::string const& config) const;

  struct Files
  {
    // Names or paths of files to be read from the source or build tree.
    // The paths may be computed as [FromDir/] + From[i].
    std::vector<std::string> From;

    // Corresponding names of files to be written in the install directory.
    // The paths may be computed as Destination/ + [ToDir/] + To[i].
    std::vector<std::string> To;

    // Prefix for all files in From.
    std::string FromDir;

    // Prefix for all files in To.
    std::string ToDir;

    NamelinkModeType NamelinkMode = NamelinkModeNone;
    bool NoTweak = false;
    bool UseSourcePermissions = false;
    cmInstallType Type = cmInstallType();
  };
  Files GetFiles(std::string const& config) const;

  bool GetOptional() const { return this->Optional; }

protected:
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void PreReplacementTweaks(std::ostream& os, Indent indent,
                            const std::string& config,
                            std::string const& file);
  void PostReplacementTweaks(std::ostream& os, Indent indent,
                             const std::string& config,
                             std::string const& file);
  void AddInstallNamePatchRule(std::ostream& os, Indent indent,
                               const std::string& config,
                               const std::string& toDestDirPath);
  void AddChrpathPatchRule(std::ostream& os, Indent indent,
                           const std::string& config,
                           std::string const& toDestDirPath);
  void AddRPathCheckRule(std::ostream& os, Indent indent,
                         const std::string& config,
                         std::string const& toDestDirPath);

  void AddStripRule(std::ostream& os, Indent indent,
                    const std::string& toDestDirPath);
  void AddRanlibRule(std::ostream& os, Indent indent,
                     const std::string& toDestDirPath);
  void AddUniversalInstallRule(std::ostream& os, Indent indent,
                               const std::string& toDestDirPath);
  void IssueCMP0095Warning(const std::string& unescapedRpath);

  std::string const TargetName;
  cmGeneratorTarget* Target = nullptr;
  std::string const FilePermissions;
  NamelinkModeType NamelinkMode;
  NamelinkModeType ImportlinkMode;
  bool const ImportLibrary;
  bool const Optional;
};
