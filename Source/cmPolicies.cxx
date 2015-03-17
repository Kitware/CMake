#include "cmPolicies.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include "cmVersionMacros.h"
#include "cmAlgorithms.h"
#include <map>
#include <set>
#include <queue>
#include <assert.h>

const char* cmPolicies::PolicyStatusNames[] = {
  "OLD", "WARN", "NEW", "REQUIRED_IF_USED", "REQUIRED_ALWAYS"
};

class cmPolicy
{
public:
  cmPolicy(cmPolicies::PolicyID iD,
            const char *idString,
            const char *shortDescription,
            unsigned int majorVersionIntroduced,
            unsigned int minorVersionIntroduced,
            unsigned int patchVersionIntroduced,
            cmPolicies::PolicyStatus status)
  {
    if (!idString || !shortDescription)
      {
      cmSystemTools::Error("Attempt to define a policy without "
        "all parameters being specified!");
      return;
      }
    this->ID = iD;
    this->IDString = idString;
    this->ShortDescription = shortDescription;
    this->MajorVersionIntroduced = majorVersionIntroduced;
    this->MinorVersionIntroduced = minorVersionIntroduced;
    this->PatchVersionIntroduced = patchVersionIntroduced;
    this->Status = status;
  }

  std::string GetVersionString()
  {
    std::ostringstream v;
    v << this->MajorVersionIntroduced << "." << this->MinorVersionIntroduced;
    if(this->PatchVersionIntroduced > 0)
      {
      v << "." << this->PatchVersionIntroduced;
      }
    return v.str();
  }

  bool IsPolicyNewerThan(unsigned int majorV,
                         unsigned int minorV,
                         unsigned int patchV)
  {
    if (majorV < this->MajorVersionIntroduced)
      {
      return true;
      }
    if (majorV > this->MajorVersionIntroduced)
      {
      return false;
      }
    if (minorV < this->MinorVersionIntroduced)
      {
      return true;
      }
    if (minorV > this->MinorVersionIntroduced)
      {
      return false;
      }
    return (patchV < this->PatchVersionIntroduced);
  }

  cmPolicies::PolicyID ID;
  std::string IDString;
  std::string ShortDescription;
  unsigned int MajorVersionIntroduced;
  unsigned int MinorVersionIntroduced;
  unsigned int PatchVersionIntroduced;
  cmPolicies::PolicyStatus Status;
};

cmPolicies::cmPolicies()
{
  // define all the policies
  this->DefinePolicy(
    CMP0000, "CMP0000",
    "A minimum required CMake version must be specified.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0001, "CMP0001",
    "CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0002, "CMP0002",
    "Logical target names must be globally unique.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0003, "CMP0003",
    "Libraries linked via full path no longer produce linker search paths.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0004, "CMP0004",
    "Libraries linked may not have leading or trailing whitespace.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0005, "CMP0005",
    "Preprocessor definition values are now escaped automatically.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0006, "CMP0006",
    "Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0007, "CMP0007",
    "list command no longer ignores empty elements.",
    2,6,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0008, "CMP0008",
    "Libraries linked by full-path must have a valid library file name.",
    2,6,1, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0009, "CMP0009",
    "FILE GLOB_RECURSE calls should not follow symlinks by default.",
    2,6,2, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0010, "CMP0010",
    "Bad variable reference syntax is an error.",
    2,6,3, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0011, "CMP0011",
    "Included scripts do automatic cmake_policy PUSH and POP.",
    2,6,3, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0012, "CMP0012",
    "if() recognizes numbers and boolean constants.",
    2,8,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0013, "CMP0013",
    "Duplicate binary directories are not allowed.",
    2,8,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0014, "CMP0014",
    "Input directories must have CMakeLists.txt.",
    2,8,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0015, "CMP0015",
    "link_directories() treats paths relative to the source dir.",
    2,8,1, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0016, "CMP0016",
    "target_link_libraries() reports error if its only argument "
    "is not a target.",
    2,8,3, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0017, "CMP0017",
    "Prefer files from the CMake module directory when including from there.",
    2,8,4, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0018, "CMP0018",
    "Ignore CMAKE_SHARED_LIBRARY_<Lang>_FLAGS variable.",
    2,8,9, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0019, "CMP0019",
    "Do not re-expand variables in include and link information.",
    2,8,11, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0020, "CMP0020",
    "Automatically link Qt executables to qtmain target on Windows.",
    2,8,11, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0021, "CMP0021",
    "Fatal error on relative paths in INCLUDE_DIRECTORIES target property.",
    2,8,12, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0022, "CMP0022",
    "INTERFACE_LINK_LIBRARIES defines the link interface.",
    2,8,12, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0023, "CMP0023",
    "Plain and keyword target_link_libraries signatures cannot be mixed.",
    2,8,12, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0024, "CMP0024",
    "Disallow include export result.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0025, "CMP0025",
    "Compiler id for Apple Clang is now AppleClang.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0026, "CMP0026",
    "Disallow use of the LOCATION target property.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0027, "CMP0027",
    "Conditionally linked imported targets with missing include directories.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0028, "CMP0028",
    "Double colon in target name means ALIAS or IMPORTED target.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0029, "CMP0029",
    "The subdir_depends command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0030, "CMP0030",
    "The use_mangled_mesa command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0031, "CMP0031",
    "The load_command command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0032, "CMP0032",
    "The output_required_files command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0033, "CMP0033",
    "The export_library_dependencies command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0034, "CMP0034",
    "The utility_source command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0035, "CMP0035",
    "The variable_requires command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0036, "CMP0036",
    "The build_name command should not be called.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0037, "CMP0037",
    "Target names should not be reserved and should match a validity pattern.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0038, "CMP0038",
    "Targets may not link directly to themselves.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0039, "CMP0039",
    "Utility targets may not have link dependencies.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0040, "CMP0040",
    "The target in the TARGET signature of add_custom_command() must exist.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0041, "CMP0041",
    "Error on relative include with generator expression.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0042, "CMP0042",
    "MACOSX_RPATH is enabled by default.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0043, "CMP0043",
    "Ignore COMPILE_DEFINITIONS_<Config> properties.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0044, "CMP0044",
    "Case sensitive <LANG>_COMPILER_ID generator expressions.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0045, "CMP0045",
    "Error on non-existent target in get_target_property.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0046, "CMP0046",
    "Error on non-existent dependency in add_dependencies.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0047, "CMP0047",
    "Use QCC compiler id for the qcc drivers on QNX.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0048, "CMP0048",
    "project() command manages VERSION variables.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0049, "CMP0049",
    "Do not expand variables in target source entries.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0050, "CMP0050",
    "Disallow add_custom_command SOURCE signatures.",
    3,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0051, "CMP0051",
    "List TARGET_OBJECTS in SOURCES target property.",
    3,1,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0052, "CMP0052",
    "Reject source and build dirs in installed "
    "INTERFACE_INCLUDE_DIRECTORIES.",
    3,1,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0053, "CMP0053",
    "Simplify variable reference and escape sequence evaluation.",
    3,1,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0054, "CMP0054",
    "Only interpret if() arguments as variables or keywords when unquoted.",
    3,1,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0055, "CMP0055",
    "Strict checking for break() command.",
    3,2,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0056, "CMP0056",
    "Honor link flags in try_compile() source-file signature.",
    3,2,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0057, "CMP0057",
    "Disallow multiple MAIN_DEPENDENCY specifications for the same file.",
    3,3,0, cmPolicies::WARN);
}

cmPolicies::~cmPolicies()
{
  cmDeleteAll(this->Policies);
}

void cmPolicies::DefinePolicy(cmPolicies::PolicyID iD,
                              const char *idString,
                              const char *shortDescription,
                              unsigned int majorVersionIntroduced,
                              unsigned int minorVersionIntroduced,
                              unsigned int patchVersionIntroduced,
                              cmPolicies::PolicyStatus status)
{
  // a policy must be unique and can only be defined once
  if (this->Policies.find(iD) != this->Policies.end())
    {
    cmSystemTools::Error("Attempt to redefine a CMake policy for policy "
      "ID ", this->GetPolicyIDString(iD).c_str());
    return;
    }

  this->Policies[iD] = new cmPolicy(iD, idString,
                                    shortDescription,
                                    majorVersionIntroduced,
                                    minorVersionIntroduced,
                                    patchVersionIntroduced,
                                    status);
  this->PolicyStringMap[idString] = iD;
}

//----------------------------------------------------------------------------
bool cmPolicies::ApplyPolicyVersion(cmMakefile *mf,
                                    const char *version)
{
  std::string ver = "2.4.0";

  if (version && strlen(version) > 0)
    {
    ver = version;
    }

  unsigned int majorVer = 2;
  unsigned int minorVer = 0;
  unsigned int patchVer = 0;
  unsigned int tweakVer = 0;

  // parse the string
  if(sscanf(ver.c_str(), "%u.%u.%u.%u",
            &majorVer, &minorVer, &patchVer, &tweakVer) < 2)
    {
    std::ostringstream e;
    e << "Invalid policy version value \"" << ver << "\".  "
      << "A numeric major.minor[.patch[.tweak]] must be given.";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  // it is an error if the policy version is less than 2.4
  if (majorVer < 2 || (majorVer == 2 && minorVer < 4))
    {
    mf->IssueMessage(cmake::FATAL_ERROR,
      "Compatibility with CMake < 2.4 is not supported by CMake >= 3.0.  "
      "For compatibility with older versions please use any CMake 2.8.x "
      "release or lower.");
    return false;
    }

  // It is an error if the policy version is greater than the running
  // CMake.
  if (majorVer > cmVersion::GetMajorVersion() ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer > cmVersion::GetMinorVersion()) ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer == cmVersion::GetMinorVersion() &&
       patchVer > cmVersion::GetPatchVersion()) ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer == cmVersion::GetMinorVersion() &&
       patchVer == cmVersion::GetPatchVersion() &&
       tweakVer > cmVersion::GetTweakVersion()))
    {
    std::ostringstream e;
    e << "An attempt was made to set the policy version of CMake to \""
      << version << "\" which is greater than this version of CMake.  "
      << "This is not allowed because the greater version may have new "
      << "policies not known to this CMake.  "
      << "You may need a newer CMake version to build this project.";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  // now loop over all the policies and set them as appropriate
  std::vector<cmPolicies::PolicyID> ancientPolicies;
  for(std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i
                     = this->Policies.begin(); i != this->Policies.end(); ++i)
    {
    if (i->second->IsPolicyNewerThan(majorVer,minorVer,patchVer))
      {
      if(i->second->Status == cmPolicies::REQUIRED_ALWAYS)
        {
        ancientPolicies.push_back(i->first);
        }
      else
        {
        cmPolicies::PolicyStatus status = cmPolicies::WARN;
        if(!this->GetPolicyDefault(mf, i->second->IDString, &status) ||
           !mf->SetPolicy(i->second->ID, status))
          {
          return false;
          }
        }
      }
    else
      {
      if (!mf->SetPolicy(i->second->ID, cmPolicies::NEW))
        {
        return false;
        }
      }
    }

  // Make sure the project does not use any ancient policies.
  if(!ancientPolicies.empty())
    {
    this->DiagnoseAncientPolicies(ancientPolicies,
                                  majorVer, minorVer, patchVer, mf);
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmPolicies::GetPolicyDefault(cmMakefile* mf, std::string const& policy,
                                  cmPolicies::PolicyStatus* defaultSetting)
{
  std::string defaultVar = "CMAKE_POLICY_DEFAULT_" + policy;
  std::string defaultValue = mf->GetSafeDefinition(defaultVar);
  if(defaultValue == "NEW")
    {
    *defaultSetting = cmPolicies::NEW;
    }
  else if(defaultValue == "OLD")
    {
    *defaultSetting = cmPolicies::OLD;
    }
  else if(defaultValue == "")
    {
    *defaultSetting = cmPolicies::WARN;
    }
  else
    {
    std::ostringstream e;
    e << defaultVar << " has value \"" << defaultValue
      << "\" but must be \"OLD\", \"NEW\", or \"\" (empty).";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  return true;
}

bool cmPolicies::GetPolicyID(const char *id, cmPolicies::PolicyID &pid)
{
  if (!id || strlen(id) < 1)
    {
    return false;
    }
  std::map<std::string,cmPolicies::PolicyID>::iterator pos =
    this->PolicyStringMap.find(id);
  if (pos == this->PolicyStringMap.end())
    {
    return false;
    }
  pid = pos->second;
  return true;
}

std::string cmPolicies::GetPolicyIDString(cmPolicies::PolicyID pid)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(pid);
  if (pos == this->Policies.end())
    {
    return "";
    }
  return pos->second->IDString;
}


///! return a warning string for a given policy
std::string cmPolicies::GetPolicyWarning(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
    {
    cmSystemTools::Error(
      "Request for warning text for undefined policy!");
    return "Request for warning text for undefined policy!";
    }

  std::ostringstream msg;
  msg <<
    "Policy " << pos->second->IDString << " is not set: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "Use the cmake_policy command to set the policy "
    "and suppress this warning.";
  return msg.str();
}


///! return an error string for when a required policy is unspecified
std::string cmPolicies::GetRequiredPolicyError(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
    {
    cmSystemTools::Error(
      "Request for error text for undefined policy!");
    return "Request for error text for undefined policy!";
    }

  std::ostringstream error;
  error <<
    "Policy " << pos->second->IDString << " is not set to NEW: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "CMake now requires this policy to be set to NEW by the project.  "
    "The policy may be set explicitly using the code\n"
    "  cmake_policy(SET " << pos->second->IDString << " NEW)\n"
    "or by upgrading all policies with the code\n"
    "  cmake_policy(VERSION " << pos->second->GetVersionString() <<
    ") # or later\n"
    "Run \"cmake --help-command cmake_policy\" for more information.";
  return error.str();
}

///! Get the default status for a policy
cmPolicies::PolicyStatus
cmPolicies::GetPolicyStatus(cmPolicies::PolicyID id)
{
  // if the policy is not know then what?
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
    {
    // TODO is this right?
    return cmPolicies::WARN;
    }

  return pos->second->Status;
}

//----------------------------------------------------------------------------
std::string
cmPolicies::GetRequiredAlwaysPolicyError(cmPolicies::PolicyID id)
{
  std::string pid = this->GetPolicyIDString(id);
  std::ostringstream e;
  e << "Policy " << pid << " may not be set to OLD behavior because this "
    << "version of CMake no longer supports it.  "
    << "The policy was introduced in "
    << "CMake version " << this->Policies[id]->GetVersionString()
    << ", and use of NEW behavior is now required."
    << "\n"
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.  "
    << "Run cmake --help-policy " << pid << " for more information.";
  return e.str();
}

//----------------------------------------------------------------------------
void
cmPolicies::DiagnoseAncientPolicies(std::vector<PolicyID> const& ancient,
                                    unsigned int majorVer,
                                    unsigned int minorVer,
                                    unsigned int patchVer,
                                    cmMakefile* mf)
{
  std::ostringstream e;
  e << "The project requests behavior compatible with CMake version \""
    << majorVer << "." << minorVer << "." << patchVer
    << "\", which requires the OLD behavior for some policies:\n";
  for(std::vector<PolicyID>::const_iterator
        i = ancient.begin(); i != ancient.end(); ++i)
    {
    cmPolicy const* policy = this->Policies[*i];
    e << "  " << policy->IDString << ": " << policy->ShortDescription << "\n";
    }
  e << "However, this version of CMake no longer supports the OLD "
    << "behavior for these policies.  "
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.";
  mf->IssueMessage(cmake::FATAL_ERROR, e.str());
}
