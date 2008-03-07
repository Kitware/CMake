#include "cmPolicies.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
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
            const char *longDescription,
            unsigned int majorVersionIntroduced,
            unsigned int minorVersionIntroduced,
            unsigned int patchVersionIntroduced,
            cmPolicies::PolicyStatus status)
  {
    if (!idString || !shortDescription || ! longDescription)
    {
      cmSystemTools::Error("Attempt to define a policy without "
        "all parameters being specified!");
      return;
    }
    this->ID = iD;
    this->IDString = idString;
    this->ShortDescription = shortDescription;
    this->LongDescription = longDescription;
    this->MajorVersionIntroduced = majorVersionIntroduced;
    this->MinorVersionIntroduced = minorVersionIntroduced;
    this->PatchVersionIntroduced = patchVersionIntroduced;
    this->Status = status;
  }

  std::string GetVersionString()
  {
    cmOStringStream error;
    error << this->MajorVersionIntroduced << "." <<
      this->MinorVersionIntroduced << "." <<
      this->PatchVersionIntroduced; 
    return error.str();
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
  std::string LongDescription;
  unsigned int MajorVersionIntroduced;
  unsigned int MinorVersionIntroduced;
  unsigned int PatchVersionIntroduced;
  cmPolicies::PolicyStatus Status;
};

cmPolicies::cmPolicies()
{
  // define all the policies
  this->DefinePolicy(
    CMP_0000, "CMP_0000",
    "Missing a CMake version specification. You must have a cmake_policy "
    "call.",
    "CMake requires that projects specify what version of CMake they have "
    "been written to. The easiest way to do this is by placing a call to "
    "cmake_policy at the top of your CMakeLists file. For example: "
    "cmake_policy(VERSION 2.6) Replace "
    "2.6 in that example with the verison of CMake you are writing to. "
    "This policy is being put in place because it aids us in detecting "
    "and maintaining backwards compatibility.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP_0001, "CMP_0001",
    "CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.",
    "The OLD behavior is to check CMAKE_BACKWARDS_COMPATIBILITY and present "
    "it to the user.  "
    "The NEW behavior is to ignore CMAKE_BACKWARDS_COMPATIBILITY "
    "completely.\n"
    "In CMake 2.4 and below the variable CMAKE_BACKWARDS_COMPATIBILITY was "
    "used to request compatibility with earlier versions of CMake.  "
    "In CMake 2.6 and above all compatibility issues are handled by policies "
    "and the cmake_policy command.  "
    "However, CMake must still check CMAKE_BACKWARDS_COMPATIBILITY for "
    "projects written for CMake 2.4 and below.",
    2,6,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP_0002, "CMP_0002",
    "CMake requires that target names be globaly unique.",
    "....",
    2,6,0, cmPolicies::WARN
    );
}

cmPolicies::~cmPolicies()
{
  // free the policies
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    delete i->second;
  }
}

void cmPolicies::DefinePolicy(cmPolicies::PolicyID iD,
                              const char *idString, 
                              const char *shortDescription,
                              const char *longDescription,
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
                                    longDescription,
                                    majorVersionIntroduced,
                                    minorVersionIntroduced,
                                    patchVersionIntroduced,
                                    status);
  this->PolicyStringMap[idString] = iD;
}

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

  // parse the string
  if(sscanf(ver.c_str(), "%u.%u.%u",
            &majorVer, &minorVer, &patchVer) < 2)
    {
    return false;
    }
  
  // it is an error if the policy version is less than 2.4
  if (majorVer < 2 || majorVer == 2 && minorVer < 4)
  {
    mf->IssueError(
      "An attempt was made to set the policy version of CMake to something "
      "earlier than \"2.4\".  "
      "In CMake 2.4 and below backwards compatibility was handled with the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "In order to get compatibility features supporting versions earlier "
      "than 2.4 set policy CMP_0001 to OLD to tell CMake to check the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "One way to so this is to set the policy version to 2.4 exactly."
      );
  }

  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    if (i->second->IsPolicyNewerThan(majorVer,minorVer,patchVer))
    {
      if (!mf->SetPolicy(i->second->ID, cmPolicies::WARN))
      {
        return false;
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
  return true;
}

// is this a valid status the listfile can set this policy to?
bool cmPolicies::IsValidPolicyStatus(cmPolicies::PolicyID id, 
                                     cmPolicies::PolicyStatus status)
{
  // if they are setting a feature to anything other than OLD or WARN and the
  // feature is not known about then that is an error
  if (this->Policies.find(id) == this->Policies.end())
  {
    if (status == cmPolicies::WARN ||
        status == cmPolicies::OLD)
    {
      return true;
    }
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the new behavior for " <<
      "a new feature that is in a later version of CMake than "
      "what you are runing, please upgrade to a newer version "
      "of CMake.";
    cmSystemTools::Error(error.str().c_str());
    return false;  
  }

  // now we know the feature is defined, so the only issue is if someone is
  // setting it to WARN or OLD when the feature is REQUIRED_ALWAYS
  if ((status == cmPolicies::WARN || 
      status == cmPolicies::OLD) && 
      this->Policies[id]->Status == cmPolicies::REQUIRED_ALWAYS)
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->Policies[id]->GetVersionString() <<
      " please either update your CMakeLists files to conform to " <<
      "the new behavior " <<
      "or use an older version of CMake that still supports " <<
      "the old behavior. Run cmake --help-policies " <<
      id << " for more information.";
    cmSystemTools::Error(error.str().c_str());
    return false;
  }
  
  return true;
}

// is this a valid status the listfile can set this policy to?
bool cmPolicies::IsValidUsedPolicyStatus(cmPolicies::PolicyID id, 
                                         cmPolicies::PolicyStatus status)
{
  // if they are setting a feature to anything other than OLD or WARN and the
  // feature is not known about then that is an error
  if (this->Policies.find(id) == this->Policies.end())
  {
    if (status == cmPolicies::WARN ||
        status == cmPolicies::OLD)
    {
      return true;
    }
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the new behavior for " <<
      "a new feature that is in a later version of CMake than "
      "what you are runing, please upgrade to a newer version "
      "of CMake.";
    cmSystemTools::Error(error.str().c_str());
    return false;  
  }

  // now we know the feature is defined, so the only issue is if someone is
  // setting it to WARN or OLD when the feature is REQUIRED_ALWAYS
  if ((status == cmPolicies::WARN || 
      status == cmPolicies::OLD) && 
      (this->Policies[id]->Status == cmPolicies::REQUIRED_ALWAYS ||
       this->Policies[id]->Status == cmPolicies::REQUIRED_IF_USED))
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->Policies[id]->GetVersionString() <<
      " please either update your CMakeLists files to conform to " <<
      "the new behavior " <<
      "or use an older version of CMake that still supports " <<
      "the old behavior. Run cmake --help-policies " <<
      id << " for more information.";
    cmSystemTools::Error(error.str().c_str());
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

  cmOStringStream msg;
  msg <<
    "Policy " << pos->second->IDString << " is not set: "
    "" << pos->second->ShortDescription << "\n"
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.\n"
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
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream error;
  error <<
    "Policy " << pos->second->IDString << " is not set to NEW: "
    "" << pos->second->ShortDescription << "\n"
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.\n"
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

void cmPolicies::GetDocumentation(std::vector<cmDocumentationEntry>& v)
{
  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    std::string full;
    full += i->second->LongDescription;
    full += "\nThis policy was introduced in CMake version ";
    full += i->second->GetVersionString();
    full += ". The version of CMake you are running ";
    // add in some more text here based on status
    switch (i->second->Status)
    {
      case cmPolicies::WARN:
        full += "defaults to warning about this policy. You can either "
        "suppress the warning without fixing the issue by adding a "
        "cmake_policy(SET ";
        full += i->second->IDString;
        full += " OLD) command to the top of your CMakeLists file or "
        "you can change your code to use the new behavior and add "
        "cmake_policy(SET ";
        full += i->second->IDString;
        full += " NEW) to your CMakeList file. If you are fixing all "
        "issues with a new version of CMake you can add "
        "cmake_policy(VERSION #.#) where #.# is the verison of CMake "
        "you are updating to. This will tell CMake that you have fixed "
        "all issues to use the new behavior.";
      case cmPolicies::OLD:
        full += "defaults to the old behavior for this policy.";
        break;
      case cmPolicies::NEW:
        full += "defaults to the new behavior for this policy.";
        break;
      case cmPolicies::REQUIRED_IF_USED:
        full += "requires the new behavior for this policy."
          "if you usee it.";
        break;
      case cmPolicies::REQUIRED_ALWAYS:
        full += "requires the new behavior for this policy.";
        break;
    }
          
    cmDocumentationEntry e(i->second->IDString.c_str(),
                           i->second->ShortDescription.c_str(),
                           full.c_str());
    v.push_back(e);
  }
}
