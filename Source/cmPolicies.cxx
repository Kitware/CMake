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
  this->DefinePolicy(CMP_0000, "CMP_0000",
    "Missing a CMake version specification. You must have a cmake_policy "
    "or cmake_minimum_required call.",
    "CMake requires that projects specify what version of CMake they have "
    "been written to. The easiest way to do this is by placing a call to "
    "cmake_policy such as the following cmake_policy(VERSION 2.6) Replace "
    "2.6 in that example with the verison of CMake you are writing to. "
    "This policy is being put in place because it aids us in detecting "
    "and maintaining backwards compatibility.",
    2,6,0, cmPolicies::WARN);
  this->PolicyStringMap["CMP_POLICY_SPECIFICATION"] = CMP_0000;
  
  this->DefinePolicy(CMP_0001, "CMP_0001",
    "CMake does not allow target names to include slash characters.",
    "CMake requires that target names not include any / or \\ characters "
    "please change the name of any targets to not use such characters."
    ,
    2,4,0, cmPolicies::REQUIRED_IF_USED); 
  this->PolicyStringMap["CMP_TARGET_NAMES_WITH_SLASHES"] = CMP_0001;
    
  this->DefinePolicy(CMP_0002, "CMP_0002",
    "CMake requires that target names be globaly unique.",
    "CMake requires that target names not include any / or \\ characters "
    "please change the name of any targets to not use such characters."
    ,
    2,6,0, cmPolicies::WARN);
  this->PolicyStringMap["CMP_REQUIRE_UNIQUE_TARGET_NAMES"] = CMP_0002;

}

cmPolicies::~cmPolicies()
{
  // free the policies
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->PolicyMap.begin();
  for (;i != this->PolicyMap.end(); ++i)
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
  if (this->PolicyMap.find(iD) != this->PolicyMap.end())
  {
    cmSystemTools::Error("Attempt to redefine a CMake policy for policy "
      "ID ", this->GetPolicyIDString(iD).c_str());
    return;
  }
  
  this->PolicyMap[iD] = new cmPolicy(iD, idString,
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
  std::string major = ver.substr(0,ver.find('.'));
  std::string patch = ver.substr(ver.find('.'));
  std::string minor = patch.substr(0,patch.find('.'));
  patch = patch.substr(patch.find('.'));
  
  if (major.size())
  {
    majorVer = atoi(major.c_str());
  }
  if (minor.size())
  {
    minorVer = atoi(minor.c_str());
  }
  if (patch.size())
  {
    patchVer = atoi(patch.c_str());
  }
 
  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i 
    = this->PolicyMap.begin();
  for (;i != this->PolicyMap.end(); ++i)
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
  if (this->PolicyMap.find(id) == this->PolicyMap.end())
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
      this->PolicyMap[id]->Status == cmPolicies::REQUIRED_ALWAYS)
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->PolicyMap[id]->GetVersionString() <<
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
  if (this->PolicyMap.find(id) == this->PolicyMap.end())
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
      (this->PolicyMap[id]->Status == cmPolicies::REQUIRED_ALWAYS ||
       this->PolicyMap[id]->Status == cmPolicies::REQUIRED_IF_USED))
  {
    cmOStringStream error;
    error << 
      "Error: an attempt was made to enable the old behavior for " <<
      "a feature that is no longer supported. The feature in " <<
      "question is feature " <<
      id <<
      " which had new behavior introduced in CMake version " <<
      this->PolicyMap[id]->GetVersionString() <<
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
    this->PolicyMap.find(pid);
  if (pos == this->PolicyMap.end())
  {
    return "";
  }
  return pos->second->IDString;
}


///! return a warning string for a given policy
std::string cmPolicies::GetPolicyWarning(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->PolicyMap.find(id);
  if (pos == this->PolicyMap.end())
  {
    cmSystemTools::Error(
      "Request for warning text for undefined policy!");
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream error;
  error << 
    "Warning " <<
    pos->second->IDString << ": " <<
    pos->second->ShortDescription <<
    " You can suppress this warning by adding either\n" <<
    "cmake_policy (OLD " <<
    pos->second->IDString << ") for the old behavior or " <<
    "cmake_policy(NEW " <<
    pos->second->IDString << ") for the new behavior. " <<
    "Run cmake --help-policy " <<
    pos->second->IDString << " for more information.";
  return error.str();
}
  
  
///! return an error string for when a required policy is unspecified
std::string cmPolicies::GetRequiredPolicyError(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->PolicyMap.find(id);
  if (pos == this->PolicyMap.end())
  {
    cmSystemTools::Error(
      "Request for error text for undefined policy!");
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream error;
  error << 
    "Error " <<
    pos->second->IDString << ": " <<
    pos->second->ShortDescription <<
    " This behavior is required now. You can suppress this message by "
    "specifying that your listfile is written to handle this new "
    "behavior by adding either\n" <<
    "cmake_policy (NEW " <<
    pos->second->IDString << ")\n or \n. " <<
    "cmake_policy (VERSION " << 
    pos->second->GetVersionString() << " ) or later."
    "Run cmake --help-policy " <<
    pos->second->IDString << " for more information.";
  return error.str();
  
}

///! Get the default status for a policy
cmPolicies::PolicyStatus 
cmPolicies::GetPolicyStatus(cmPolicies::PolicyID id)
{
  // if the policy is not know then what?
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos = 
    this->PolicyMap.find(id);
  if (pos == this->PolicyMap.end())
  {
    // TODO is this right?
    return cmPolicies::WARN;
  }
  
  return pos->second->Status;
}

