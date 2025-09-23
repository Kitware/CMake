/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"

class cmMakefile;

class cmInstallCommandArguments : public cmArgumentParser<void>
{
public:
  cmInstallCommandArguments(std::string defaultComponent,
                            cmMakefile& makefile);
  void SetGenericArguments(cmInstallCommandArguments* args)
  {
    this->GenericArguments = args;
  }

  // Compute destination path.and check permissions
  bool Finalize();

  std::string const& GetDestination() const;
  std::string const& GetComponent() const;
  std::string const& GetNamelinkComponent() const;
  bool GetExcludeFromAll() const;
  std::string const& GetRename() const;
  std::string const& GetPermissions() const;
  std::vector<std::string> const& GetConfigurations() const;
  bool GetOptional() const;
  bool GetNamelinkOnly() const;
  bool GetNamelinkSkip() const;
  bool HasNamelinkComponent() const;
  std::string const& GetType() const;

  std::string const& GetDefaultComponent() const;

  static bool CheckPermissions(std::string const& onePerm, std::string& perm);

private:
  std::string Destination;
  std::string Component;
  std::string NamelinkComponent;
  bool ExcludeFromAll = false;
  std::string Rename;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Permissions;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Configurations;
  bool Optional = false;
  bool NamelinkOnly = false;
  bool NamelinkSkip = false;
  std::string Type;

  std::string DestinationString;
  std::string PermissionsString;

  cmInstallCommandArguments* GenericArguments = nullptr;
  static char const* PermissionsTable[];
  static std::string const EmptyString;
  std::string DefaultComponentName;
  bool CheckPermissions();
};

class cmInstallCommandIncludesArgument
{
public:
  cmInstallCommandIncludesArgument();
  void Parse(std::vector<std::string> const* args,
             std::vector<std::string>* unconsumedArgs);

  std::vector<std::string> const& GetIncludeDirs() const;

private:
  std::vector<std::string> IncludeDirs;
};

class cmInstallCommandFileSetArguments : public cmInstallCommandArguments
{
public:
  cmInstallCommandFileSetArguments(std::string defaultComponent,
                                   cmMakefile& makefile);

  void Parse(std::vector<std::string> args,
             std::vector<std::string>* unconsumedArgs);

  std::string const& GetFileSet() const { return this->FileSet; }

private:
  std::string FileSet;
};
