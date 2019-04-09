/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallCommandArguments_h
#define cmInstallCommandArguments_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmArgumentParser.h"

class cmInstallCommandArguments : public cmArgumentParser<void>
{
public:
  cmInstallCommandArguments(std::string defaultComponent);
  void SetGenericArguments(cmInstallCommandArguments* args)
  {
    this->GenericArguments = args;
  }

  // Compute destination path.and check permissions
  bool Finalize();

  const std::string& GetDestination() const;
  const std::string& GetComponent() const;
  const std::string& GetNamelinkComponent() const;
  bool GetExcludeFromAll() const;
  const std::string& GetRename() const;
  const std::string& GetPermissions() const;
  const std::vector<std::string>& GetConfigurations() const;
  bool GetOptional() const;
  bool GetNamelinkOnly() const;
  bool GetNamelinkSkip() const;
  bool HasNamelinkComponent() const;
  const std::string& GetType() const;

  static bool CheckPermissions(const std::string& onePerm, std::string& perm);

private:
  std::string Destination;
  std::string Component;
  std::string NamelinkComponent;
  bool ExcludeFromAll = false;
  std::string Rename;
  std::vector<std::string> Permissions;
  std::vector<std::string> Configurations;
  bool Optional = false;
  bool NamelinkOnly = false;
  bool NamelinkSkip = false;
  std::string Type;

  std::string DestinationString;
  std::string PermissionsString;

  cmInstallCommandArguments* GenericArguments = nullptr;
  static const char* PermissionsTable[];
  static const std::string EmptyString;
  std::string DefaultComponentName;
  bool CheckPermissions();
};

class cmInstallCommandIncludesArgument
{
public:
  cmInstallCommandIncludesArgument();
  void Parse(const std::vector<std::string>* args,
             std::vector<std::string>* unconsumedArgs);

  const std::vector<std::string>& GetIncludeDirs() const;

private:
  std::vector<std::string> IncludeDirs;
};

#endif
