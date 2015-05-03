#include "cmPolicies.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmVersion.h"
#include "cmVersionMacros.h"
#include "cmAlgorithms.h"
#include <map>
#include <set>
#include <queue>
#include <assert.h>

static bool stringToId(const char* input, cmPolicies::PolicyID& pid)
{
  assert(input);
  if (strlen(input) != 7)
    {
    return false;
    }
  if (!cmHasLiteralPrefix(input, "CMP"))
    {
    return false;
    }
  if (cmHasLiteralSuffix(input, "0000"))
    {
    pid = cmPolicies::CMP0000;
    return true;
    }
  for (int i = 3; i < 7; ++i)
    {
    if (!isdigit(*(input + i)))
      {
      return false;
      }
    }
  long id;
  if (!cmSystemTools::StringToLong(input + 3, &id))
    {
    return false;
    }
  if (id >= cmPolicies::CMPCOUNT)
    {
    return false;
    }
  pid = cmPolicies::PolicyID(id);
  return true;
}

#define CM_SELECT_ID_VERSION(F, A1, A2, A3, A4, A5, A6) F(A1, A3, A4, A5)
#define CM_FOR_EACH_POLICY_ID_VERSION(POLICY) \
  CM_FOR_EACH_POLICY_TABLE(POLICY, CM_SELECT_ID_VERSION)

#define CM_SELECT_ID_DOC(F, A1, A2, A3, A4, A5, A6) F(A1, A2)
#define CM_FOR_EACH_POLICY_ID_DOC(POLICY) \
  CM_FOR_EACH_POLICY_TABLE(POLICY, CM_SELECT_ID_DOC)

static const char* idToString(cmPolicies::PolicyID id)
{
  switch(id)
    {
#define POLICY_CASE(ID) \
    case cmPolicies::ID: \
      return #ID;
  CM_FOR_EACH_POLICY_ID(POLICY_CASE)
#undef POLICY_CASE
    case cmPolicies::CMPCOUNT:
      return 0;
    }
  return 0;
}

static const char* idToVersion(cmPolicies::PolicyID id)
{
  switch(id)
    {
#define POLICY_CASE(ID, V_MAJOR, V_MINOR, V_PATCH) \
    case cmPolicies::ID: \
      return #V_MAJOR "." #V_MINOR "." #V_PATCH;
  CM_FOR_EACH_POLICY_ID_VERSION(POLICY_CASE)
#undef POLICY_CASE
    case cmPolicies::CMPCOUNT:
      return 0;
    }
  return 0;
}

static bool isPolicyNewerThan(cmPolicies::PolicyID id,
                       unsigned int majorV,
                       unsigned int minorV,
                       unsigned int patchV)
{
  switch(id)
    {
#define POLICY_CASE(ID, V_MAJOR, V_MINOR, V_PATCH) \
    case cmPolicies::ID: \
      return (majorV < V_MAJOR || \
             (majorV == V_MAJOR && \
              minorV + 1 < V_MINOR + 1) || \
             (majorV == V_MAJOR && \
              minorV == V_MINOR && \
              patchV + 1 < V_PATCH + 1));
  CM_FOR_EACH_POLICY_ID_VERSION(POLICY_CASE)
#undef POLICY_CASE
    case cmPolicies::CMPCOUNT:
      return false;
    }
  return false;
}

const char* idToShortDescription(cmPolicies::PolicyID id)
{
  switch(id)
    {
#define POLICY_CASE(ID, SHORT_DESCRIPTION) \
    case cmPolicies::ID: \
      return SHORT_DESCRIPTION;
  CM_FOR_EACH_POLICY_ID_DOC(POLICY_CASE)
#undef POLICY_CASE
    case cmPolicies::CMPCOUNT:
      return 0;
    }
  return 0;
}

class cmPolicy
{
public:
  cmPolicy(cmPolicies::PolicyID iD,
            cmPolicies::PolicyStatus status)
  {
    this->ID = iD;
    this->Status = status;
  }

  cmPolicies::PolicyID ID;
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
    CMP0058, "CMP0058",
    "Ninja requires custom command byproducts to be explicit.",
    3,3,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0059, "CMP0059",
    "Do no treat DEFINITIONS as a built-in directory property.",
    3,3,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0060, "CMP0060",
    "Link libraries by full path even in implicit directories.",
    3,3,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0057, "CMP0057",
    "Support new IN_LIST if() operator.",
    3,3,0, cmPolicies::WARN);
}

cmPolicies::~cmPolicies()
{
  cmDeleteAll(this->Policies);
}

void cmPolicies::DefinePolicy(cmPolicies::PolicyID iD,
                              const char *,
                              const char *,
                              unsigned int,
                              unsigned int,
                              unsigned int,
                              cmPolicies::PolicyStatus status)
{
  this->Policies[iD] = new cmPolicy(iD,
                                    status);
}

//----------------------------------------------------------------------------
static void DiagnoseAncientPolicies(
    std::vector<cmPolicies::PolicyID> const& ancient,
    unsigned int majorVer,
    unsigned int minorVer,
    unsigned int patchVer,
    cmMakefile* mf)
{
  std::ostringstream e;
  e << "The project requests behavior compatible with CMake version \""
    << majorVer << "." << minorVer << "." << patchVer
    << "\", which requires the OLD behavior for some policies:\n";
  for(std::vector<cmPolicies::PolicyID>::const_iterator
        i = ancient.begin(); i != ancient.end(); ++i)
    {
    e << "  " << idToString(*i) << ": " << idToShortDescription(*i) << "\n";
    }
  e << "However, this version of CMake no longer supports the OLD "
    << "behavior for these policies.  "
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.";
  mf->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
static bool GetPolicyDefault(cmMakefile* mf, std::string const& policy,
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
    if (isPolicyNewerThan(i->first, majorVer, minorVer, patchVer))
      {
      if(this->GetPolicyStatus(i->first) == cmPolicies::REQUIRED_ALWAYS)
        {
        ancientPolicies.push_back(i->first);
        }
      else
        {
        cmPolicies::PolicyStatus status = cmPolicies::WARN;
        if(!GetPolicyDefault(mf, idToString(i->first), &status) ||
           !mf->SetPolicy(i->first, status))
          {
          return false;
          }
        }
      }
    else
      {
      if (!mf->SetPolicy(i->first, cmPolicies::NEW))
        {
        return false;
        }
      }
    }

  // Make sure the project does not use any ancient policies.
  if(!ancientPolicies.empty())
    {
    DiagnoseAncientPolicies(ancientPolicies,
                            majorVer, minorVer, patchVer, mf);
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  return true;
}

bool cmPolicies::GetPolicyID(const char *id, cmPolicies::PolicyID &pid)
{
  return stringToId(id, pid);
}

///! return a warning string for a given policy
std::string cmPolicies::GetPolicyWarning(cmPolicies::PolicyID id)
{
  std::ostringstream msg;
  msg <<
    "Policy " << idToString(id) << " is not set: "
    "" << idToShortDescription(id) << "  "
    "Run \"cmake --help-policy " << idToString(id) << "\" for "
    "policy details.  "
    "Use the cmake_policy command to set the policy "
    "and suppress this warning.";
  return msg.str();
}


///! return an error string for when a required policy is unspecified
std::string cmPolicies::GetRequiredPolicyError(cmPolicies::PolicyID id)
{
  std::ostringstream error;
  error <<
    "Policy " << idToString(id) << " is not set to NEW: "
    "" << idToShortDescription(id) << "  "
    "Run \"cmake --help-policy " << idToString(id) << "\" for "
    "policy details.  "
    "CMake now requires this policy to be set to NEW by the project.  "
    "The policy may be set explicitly using the code\n"
    "  cmake_policy(SET " << idToString(id) << " NEW)\n"
    "or by upgrading all policies with the code\n"
    "  cmake_policy(VERSION " << idToVersion(id) <<
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
  std::string pid = idToString(id);
  std::ostringstream e;
  e << "Policy " << pid << " may not be set to OLD behavior because this "
    << "version of CMake no longer supports it.  "
    << "The policy was introduced in "
    << "CMake version " << idToVersion(id)
    << ", and use of NEW behavior is now required."
    << "\n"
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.  "
    << "Run cmake --help-policy " << pid << " for more information.";
  return e.str();
}
