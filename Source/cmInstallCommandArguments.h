/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallCommandArguments_h
#define cmInstallCommandArguments_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCommandArgumentsHelper.h"

class cmInstallCommandArguments
{
public:
  cmInstallCommandArguments(std::string defaultComponent);
  void SetGenericArguments(cmInstallCommandArguments* args)
  {
    this->GenericArguments = args;
  }
  void Parse(const std::vector<std::string>* args,
             std::vector<std::string>* unconsumedArgs);

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

  // once HandleDirectoryMode() is also switched to using
  // cmInstallCommandArguments then these two functions can become non-static
  // private member functions without arguments
  static bool CheckPermissions(const std::string& onePerm, std::string& perm);
  cmCommandArgumentsHelper Parser;
  cmCommandArgumentGroup ArgumentGroup;

private:
  cmCAString Destination;
  cmCAString Component;
  cmCAString NamelinkComponent;
  cmCAEnabler ExcludeFromAll;
  cmCAString Rename;
  cmCAStringVector Permissions;
  cmCAStringVector Configurations;
  cmCAEnabler Optional;
  cmCAEnabler NamelinkOnly;
  cmCAEnabler NamelinkSkip;
  cmCAString Type;

  std::string DestinationString;
  std::string PermissionsString;

  cmInstallCommandArguments* GenericArguments;
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
