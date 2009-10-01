/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudioGenerator.h"

#include "cmCallVisualStudioMacro.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::cmGlobalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::~cmGlobalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetRegistryBase()
{
  std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\";
  key += this->GetIDEVersion();
  return key;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::Generate()
{
  // Add a special target that depends on ALL projects for easy build
  // of one configuration only.
  const char* no_working_dir = 0;
  std::vector<std::string> no_depends;
  cmCustomCommandLines no_commands;
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& gen = it->second;
    // add the ALL_BUILD to the first local generator of each project
    if(gen.size())
      {
      // Use no actual command lines so that the target itself is not
      // considered always out of date.
      cmTarget* allBuild = 
        gen[0]->GetMakefile()->
        AddUtilityCommand("ALL_BUILD", true, no_working_dir,
                          no_depends, no_commands, false,
                          "Build all projects");
      // Now make all targets depend on the ALL_BUILD target
      cmTargets targets;
      for(std::vector<cmLocalGenerator*>::iterator i = gen.begin();
          i != gen.end(); ++i)
        {
        cmTargets& targets = (*i)->GetMakefile()->GetTargets();
        for(cmTargets::iterator t = targets.begin();
            t != targets.end(); ++t)
          {
          if(!this->IsExcluded(gen[0], t->second))
            {
            allBuild->AddUtility(t->second.GetName());
            }
          }
        }
      }
    }

  // Fix utility dependencies to avoid linking to libraries.
  this->FixUtilityDepends();

  // Configure CMake Visual Studio macros, for this user on this version
  // of Visual Studio.
  this->ConfigureCMakeVisualStudioMacros();

  // Run all the local generators.
  this->cmGlobalGenerator::Generate();
}

//----------------------------------------------------------------------------
bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
  const std::string& regKeyBase,
  std::string& nextAvailableSubKeyName);

void RegisterVisualStudioMacros(const std::string& macrosFile,
  const std::string& regKeyBase);

//----------------------------------------------------------------------------
#define CMAKE_VSMACROS_FILENAME \
  "CMakeVSMacros2.vsmacros"

#define CMAKE_VSMACROS_RELOAD_MACRONAME \
  "Macros.CMakeVSMacros2.Macros.ReloadProjects"

#define CMAKE_VSMACROS_STOP_MACRONAME \
  "Macros.CMakeVSMacros2.Macros.StopBuild"

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::ConfigureCMakeVisualStudioMacros()
{
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();
  std::string dir = this->GetUserMacrosDirectory();

  if (mf != 0 && dir != "")
    {
    std::string src = mf->GetRequiredDefinition("CMAKE_ROOT");
    src += "/Templates/" CMAKE_VSMACROS_FILENAME;

    std::string dst = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;

    // Copy the macros file to the user directory only if the
    // destination does not exist or the source location is newer.
    // This will allow the user to edit the macros for development
    // purposes but newer versions distributed with CMake will replace
    // older versions in user directories.
    int res;
    if(!cmSystemTools::FileTimeCompare(src.c_str(), dst.c_str(), &res) ||
       res > 0)
      {
      if (!cmSystemTools::CopyFileAlways(src.c_str(), dst.c_str()))
        {
        std::ostringstream oss;
        oss << "Could not copy from: " << src << std::endl;
        oss << "                 to: " << dst << std::endl;
        cmSystemTools::Message(oss.str().c_str(), "Warning");
        }
      }

    RegisterVisualStudioMacros(dst, this->GetUserMacrosRegKeyBase());
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator
::CallVisualStudioMacro(MacroName m,
                        const char* vsSolutionFile)
{
  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();
  std::string dir = this->GetUserMacrosDirectory();

  // Only really try to call the macro if:
  //  - mf is non-NULL
  //  - there is a UserMacrosDirectory
  //  - the CMake vsmacros file exists
  //  - the CMake vsmacros file is registered
  //  - there were .sln/.vcproj files changed during generation
  //
  if (mf != 0 && dir != "")
    {
    std::string macrosFile = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;
    std::string nextSubkeyName;
    if (cmSystemTools::FileExists(macrosFile.c_str()) &&
      IsVisualStudioMacrosFileRegistered(macrosFile,
        this->GetUserMacrosRegKeyBase(), nextSubkeyName)
      )
      {
      std::string topLevelSlnName;
      if(vsSolutionFile)
        {
        topLevelSlnName = vsSolutionFile;
        }
      else
        {
        topLevelSlnName = mf->GetStartOutputDirectory();
        topLevelSlnName += "/";
        topLevelSlnName += mf->GetProjectName();
        topLevelSlnName += ".sln";
        }

      if(m == MacroReload)
        {
        std::vector<std::string> filenames;
        this->GetFilesReplacedDuringGenerate(filenames);
        if (filenames.size() > 0)
          {
          // Convert vector to semi-colon delimited string of filenames:
          std::string projects;
          std::vector<std::string>::iterator it = filenames.begin();
          if (it != filenames.end())
            {
            projects = *it;
            ++it;
            }
          for (; it != filenames.end(); ++it)
            {
            projects += ";";
            projects += *it;
            }
          cmCallVisualStudioMacro::CallMacro(topLevelSlnName,
            CMAKE_VSMACROS_RELOAD_MACRONAME, projects,
            this->GetCMakeInstance()->GetDebugOutput());
          }
        }
      else if(m == MacroStop)
        {
        cmCallVisualStudioMacro::CallMacro(topLevelSlnName,
          CMAKE_VSMACROS_STOP_MACRONAME, "",
          this->GetCMakeInstance()->GetDebugOutput());
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetUserMacrosDirectory()
{
  return "";
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetUserMacrosRegKeyBase()
{
  return "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::FixUtilityDepends()
{
  // Skip for VS versions 8 and above.
  if(!this->VSLinksDependencies())
    {
    return;
    }

  // For VS versions before 8:
  //
  // When a target that links contains a project-level dependency on a
  // library target that library is automatically linked.  In order to
  // allow utility-style project-level dependencies that do not
  // actually link we need to automatically insert an intermediate
  // custom target.
  //
  // Here we edit the utility dependencies of a target to add the
  // intermediate custom target when necessary.
  for(unsigned i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets* targets =
      &(this->LocalGenerators[i]->GetMakefile()->GetTargets());
    for(cmTargets::iterator tarIt = targets->begin();
        tarIt != targets->end(); ++tarIt)
      {
      this->FixUtilityDependsForTarget(tarIt->second);
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator::FixUtilityDependsForTarget(cmTarget& target)
{
  // Only targets that link need to be fixed.
  if(target.GetType() != cmTarget::STATIC_LIBRARY &&
     target.GetType() != cmTarget::SHARED_LIBRARY &&
     target.GetType() != cmTarget::MODULE_LIBRARY &&
     target.GetType() != cmTarget::EXECUTABLE)
    {
    return;
    }

#if 0
  // This feature makes a mess in SLN files for VS 7.1 and below.  It
  // creates an extra target for every target that is "linked" by a
  // static library.  Without this feature static libraries do not
  // wait until their "link" dependencies are built to build.  This is
  // not a problem 99.9% of the time, and projects that do have the
  // problem can enable this work-around by using add_dependencies.

  // Static libraries cannot depend directly on the targets to which
  // they link because VS will copy those targets into the library
  // (for VS < 8).  To work around the problem we copy the
  // dependencies to be utility dependencies so that the work-around
  // below is used.
  if(target.GetType() == cmTarget::STATIC_LIBRARY)
    {
    cmTarget::LinkLibraryVectorType const& libs = target.GetLinkLibraries();
    for(cmTarget::LinkLibraryVectorType::const_iterator i = libs.begin();
        i != libs.end(); ++i)
      {
      if(cmTarget* depTarget = this->FindTarget(0, i->first.c_str(), false))
        {
        target.AddUtility(depTarget->GetName());
        }
      }
    }
#endif

  // Look at each utility dependency.
  for(std::set<cmStdString>::const_iterator ui =
        target.GetUtilities().begin();
      ui != target.GetUtilities().end(); ++ui)
    {
    if(cmTarget* depTarget = this->FindTarget(0, ui->c_str()))
      {
      if(depTarget->GetType() == cmTarget::STATIC_LIBRARY ||
         depTarget->GetType() == cmTarget::SHARED_LIBRARY ||
         depTarget->GetType() == cmTarget::MODULE_LIBRARY)
        {
        // This utility dependency will cause an attempt to link.  If
        // the depender does not already link the dependee we need an
        // intermediate target.
        if(!this->CheckTargetLinks(target, ui->c_str()))
          {
          this->CreateUtilityDependTarget(*depTarget);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator::CreateUtilityDependTarget(cmTarget& target)
{
  // This target is a library on which a utility dependency exists.
  // We need to create an intermediate custom target to hook up the
  // dependency without causing a link.
  const char* altName = target.GetProperty("ALTERNATIVE_DEPENDENCY_NAME");
  if(!altName)
    {
    // Create the intermediate utility target.
    std::string altNameStr = target.GetName();
    altNameStr += "_UTILITY";
    const std::vector<std::string> no_depends;
    cmCustomCommandLines no_commands;
    const char* no_working_dir = 0;
    const char* no_comment = 0;
    target.GetMakefile()->AddUtilityCommand(altNameStr.c_str(), true,
                                            no_working_dir, no_depends,
                                            no_commands, false, no_comment);
    target.SetProperty("ALTERNATIVE_DEPENDENCY_NAME", altNameStr.c_str());

    // Most targets have a GUID created in ConfigureFinalPass.  Since
    // that has already been called, create one for this target now.
    this->CreateGUID(altNameStr.c_str());

    // The intermediate target should depend on the original target.
    if(cmTarget* alt = this->FindTarget(0, altNameStr.c_str()))
      {
      alt->AddUtility(target.GetName());
      }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudioGenerator::CheckTargetLinks(cmTarget& target,
                                                     const char* name)
{
  // Return whether the given target links to a target with the given name.
  if(target.GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Static libraries never link to anything.
    return false;
    }
  cmTarget::LinkLibraryVectorType const& libs = target.GetLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator i = libs.begin();
      i != libs.end(); ++i)
    {
    if(i->first == name)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
const char*
cmGlobalVisualStudioGenerator::GetUtilityForTarget(cmTarget& target,
                                                   const char* name)
{
  // Possibly depend on an intermediate utility target to avoid
  // linking.
  if(target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY ||
     target.GetType() == cmTarget::EXECUTABLE)
    {
    // The depender is a target that links.  Lookup the dependee to
    // see if it provides an alternative dependency name.
    if(cmTarget* depTarget = this->FindTarget(0, name))
      {
      // Check for an alternative name created by FixUtilityDepends.
      if(const char* altName =
         depTarget->GetProperty("ALTERNATIVE_DEPENDENCY_NAME"))
        {
        // The alternative name is needed only if the depender does
        // not really link to the dependee.
        if(!this->CheckTargetLinks(target, name))
          {
          return altName;
          }
        }
      }
    }

  // No special case.  Just use the original dependency name.
  return name;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::GetTargetSets(
  TargetDependSet& projectTargets, TargetDependSet& originalTargets,
  cmLocalGenerator* root, GeneratorVector const& generators
  )
{
  this->cmGlobalGenerator::GetTargetSets(projectTargets, originalTargets,
                                         root, generators);

  // Add alternative dependency targets created by FixUtilityDepends.
  for(TargetDependSet::iterator ti = projectTargets.begin();
      ti != projectTargets.end(); ++ti)
    {
    cmTarget* tgt = *ti;
    if(const char* altName = tgt->GetProperty("ALTERNATIVE_DEPENDENCY_NAME"))
      {
      if(cmTarget* alt = tgt->GetMakefile()->FindTarget(altName))
        {
        projectTargets.insert(alt);
        }
      }
    }
}

//----------------------------------------------------------------------------
#include <windows.h>

//----------------------------------------------------------------------------
bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
  const std::string& regKeyBase,
  std::string& nextAvailableSubKeyName)
{
  bool macrosRegistered = false;

  std::string s1;
  std::string s2;

  // Make lowercase local copies, convert to Unix slashes, and
  // see if the resulting strings are the same:
  s1 = cmSystemTools::LowerCase(macrosFile);
  cmSystemTools::ConvertToUnixSlashes(s1);

  std::string keyname;
  HKEY hkey = NULL;
  LONG result = ERROR_SUCCESS;
  DWORD index = 0;

  keyname = regKeyBase + "\\OtherProjects7";
  hkey = NULL;
  result = RegOpenKeyEx(HKEY_CURRENT_USER, keyname.c_str(),
                        0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result)
    {
    // Iterate the subkeys and look for the values of interest in each subkey:
    CHAR subkeyname[256];
    DWORD cch_subkeyname = sizeof(subkeyname)/sizeof(subkeyname[0]);
    CHAR keyclass[256];
    DWORD cch_keyclass = sizeof(keyclass)/sizeof(keyclass[0]);
    FILETIME lastWriteTime;
    lastWriteTime.dwHighDateTime = 0;
    lastWriteTime.dwLowDateTime = 0;

    while (ERROR_SUCCESS == RegEnumKeyEx(hkey, index, subkeyname,
                                         &cch_subkeyname,
      0, keyclass, &cch_keyclass, &lastWriteTime))
      {
      // Open the subkey and query the values of interest:
      HKEY hsubkey = NULL;
      result = RegOpenKeyEx(hkey, subkeyname, 0, KEY_READ, &hsubkey);
      if (ERROR_SUCCESS == result)
        {
        DWORD valueType = REG_SZ;
        CHAR data1[256];
        DWORD cch_data1 = sizeof(data1)/sizeof(data1[0]);
        RegQueryValueEx(hsubkey, "Path", 0, &valueType,
                        (LPBYTE) &data1[0], &cch_data1);

        DWORD data2 = 0;
        DWORD cch_data2 = sizeof(data2);
        RegQueryValueEx(hsubkey, "Security", 0, &valueType,
                        (LPBYTE) &data2, &cch_data2);

        DWORD data3 = 0;
        DWORD cch_data3 = sizeof(data3);
        RegQueryValueEx(hsubkey, "StorageFormat", 0, &valueType,
                        (LPBYTE) &data3, &cch_data3);

        s2 = cmSystemTools::LowerCase(data1);
        cmSystemTools::ConvertToUnixSlashes(s2);
        if (s2 == s1)
          {
          macrosRegistered = true;
          }

        std::string fullname(data1);
        std::string filename;
        std::string filepath;
        std::string filepathname;
        std::string filepathpath;
        if (cmSystemTools::FileExists(fullname.c_str()))
          {
          filename = cmSystemTools::GetFilenameName(fullname);
          filepath = cmSystemTools::GetFilenamePath(fullname);
          filepathname = cmSystemTools::GetFilenameName(filepath);
          filepathpath = cmSystemTools::GetFilenamePath(filepath);
          }

        //std::cout << keyname << "\\" << subkeyname << ":" << std::endl;
        //std::cout << "  Path: " << data1 << std::endl;
        //std::cout << "  Security: " << data2 << std::endl;
        //std::cout << "  StorageFormat: " << data3 << std::endl;
        //std::cout << "  filename: " << filename << std::endl;
        //std::cout << "  filepath: " << filepath << std::endl;
        //std::cout << "  filepathname: " << filepathname << std::endl;
        //std::cout << "  filepathpath: " << filepathpath << std::endl;
        //std::cout << std::endl;

        RegCloseKey(hsubkey);
        }
      else
        {
        std::cout << "error opening subkey: " << subkeyname << std::endl;
        std::cout << std::endl;
        }

      ++index;
      cch_subkeyname = sizeof(subkeyname)/sizeof(subkeyname[0]);
      cch_keyclass = sizeof(keyclass)/sizeof(keyclass[0]);
      lastWriteTime.dwHighDateTime = 0;
      lastWriteTime.dwLowDateTime = 0;
      }

    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }


  // Pass back next available sub key name, assuming sub keys always
  // follow the expected naming scheme. Expected naming scheme is that
  // the subkeys of OtherProjects7 is 0 to n-1, so it's ok to use "n"
  // as the name of the next subkey.
  std::ostringstream ossNext;
  ossNext << index;
  nextAvailableSubKeyName = ossNext.str();


  keyname = regKeyBase + "\\RecordingProject7";
  hkey = NULL;
  result = RegOpenKeyEx(HKEY_CURRENT_USER, keyname.c_str(),
                        0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result)
    {
    DWORD valueType = REG_SZ;
    CHAR data1[256];
    DWORD cch_data1 = sizeof(data1)/sizeof(data1[0]);
    RegQueryValueEx(hkey, "Path", 0, &valueType,
                    (LPBYTE) &data1[0], &cch_data1);

    DWORD data2 = 0;
    DWORD cch_data2 = sizeof(data2);
    RegQueryValueEx(hkey, "Security", 0, &valueType,
                    (LPBYTE) &data2, &cch_data2);

    DWORD data3 = 0;
    DWORD cch_data3 = sizeof(data3);
    RegQueryValueEx(hkey, "StorageFormat", 0, &valueType,
                    (LPBYTE) &data3, &cch_data3);

    s2 = cmSystemTools::LowerCase(data1);
    cmSystemTools::ConvertToUnixSlashes(s2);
    if (s2 == s1)
      {
      macrosRegistered = true;
      }

    //std::cout << keyname << ":" << std::endl;
    //std::cout << "  Path: " << data1 << std::endl;
    //std::cout << "  Security: " << data2 << std::endl;
    //std::cout << "  StorageFormat: " << data3 << std::endl;
    //std::cout << std::endl;

    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }

  return macrosRegistered;
}

//----------------------------------------------------------------------------
void WriteVSMacrosFileRegistryEntry(
  const std::string& nextAvailableSubKeyName,
  const std::string& macrosFile,
  const std::string& regKeyBase)
{
  std::string keyname = regKeyBase + "\\OtherProjects7";
  HKEY hkey = NULL;
  LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, keyname.c_str(), 0,
    KEY_READ|KEY_WRITE, &hkey);
  if (ERROR_SUCCESS == result)
    {
    // Create the subkey and set the values of interest:
    HKEY hsubkey = NULL;
    result = RegCreateKeyEx(hkey, nextAvailableSubKeyName.c_str(), 0, "", 0,
      KEY_READ|KEY_WRITE, 0, &hsubkey, 0);
    if (ERROR_SUCCESS == result)
      {
      DWORD dw = 0;

      std::string s(macrosFile);
      cmSystemTools::ReplaceString(s, "/", "\\");

      result = RegSetValueEx(hsubkey, "Path", 0, REG_SZ, (LPBYTE) s.c_str(),
        static_cast<DWORD>(strlen(s.c_str()) + 1));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 1: " << result << std::endl;
        std::cout << std::endl;
        }

      // Security value is always "1" for sample macros files (seems to be "2"
      // if you put the file somewhere outside the standard VSMacros folder)
      dw = 1;
      result = RegSetValueEx(hsubkey, "Security",
                             0, REG_DWORD, (LPBYTE) &dw, sizeof(DWORD));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 2: " << result << std::endl;
        std::cout << std::endl;
        }

      // StorageFormat value is always "0" for sample macros files
      dw = 0;
      result = RegSetValueEx(hsubkey, "StorageFormat",
                             0, REG_DWORD, (LPBYTE) &dw, sizeof(DWORD));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 3: " << result << std::endl;
        std::cout << std::endl;
        }

      RegCloseKey(hsubkey);
      }
    else
      {
      std::cout << "error creating subkey: "
                << nextAvailableSubKeyName << std::endl;
      std::cout << std::endl;
      }
    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }
}

//----------------------------------------------------------------------------
void RegisterVisualStudioMacros(const std::string& macrosFile,
  const std::string& regKeyBase)
{
  bool macrosRegistered;
  std::string nextAvailableSubKeyName;

  macrosRegistered = IsVisualStudioMacrosFileRegistered(macrosFile,
    regKeyBase, nextAvailableSubKeyName);

  if (!macrosRegistered)
    {
    int count = cmCallVisualStudioMacro::
      GetNumberOfRunningVisualStudioInstances("ALL");

    // Only register the macros file if there are *no* instances of Visual
    // Studio running. If we register it while one is running, first, it has
    // no effect on the running instance; second, and worse, Visual Studio
    // removes our newly added registration entry when it quits. Instead,
    // emit a warning asking the user to exit all running Visual Studio
    // instances...
    //
    if (0 != count)
      {
      std::ostringstream oss;
      oss << "Could not register CMake's Visual Studio macros file '"
        << CMAKE_VSMACROS_FILENAME "' while Visual Studio is running."
        << " Please exit all running instances of Visual Studio before"
        << " continuing." << std::endl
        << std::endl
        << "CMake needs to register Visual Studio macros when its macros"
        << " file is updated or when it detects that its current macros file"
        << " is no longer registered with Visual Studio."
        << std::endl;
      cmSystemTools::Message(oss.str().c_str(), "Warning");

      // Count them again now that the warning is over. In the case of a GUI
      // warning, the user may have gone to close Visual Studio and then come
      // back to the CMake GUI and clicked ok on the above warning. If so,
      // then register the macros *now* if the count is *now* 0...
      //
      count = cmCallVisualStudioMacro::
        GetNumberOfRunningVisualStudioInstances("ALL");

      // Also re-get the nextAvailableSubKeyName in case Visual Studio
      // wrote out new registered macros information as it was exiting:
      //
      if (0 == count)
        {
        IsVisualStudioMacrosFileRegistered(macrosFile, regKeyBase,
          nextAvailableSubKeyName);
        }
      }

    // Do another if check - 'count' may have changed inside the above if:
    //
    if (0 == count)
      {
      WriteVSMacrosFileRegistryEntry(nextAvailableSubKeyName, macrosFile,
        regKeyBase);
      }
    }
}
bool cmGlobalVisualStudioGenerator::TargetIsFortranOnly(cmTarget& target)
{
  // check to see if this is a fortran build
  std::set<cmStdString> languages;
  target.GetLanguages(languages);
  if(languages.size() == 1)
    {
    if(*languages.begin() == "Fortran")
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool
cmGlobalVisualStudioGenerator::TargetCompare
::operator()(cmTarget const* l, cmTarget const* r) const
{
  // Make sure ALL_BUILD is first so it is the default active project.
  if(strcmp(r->GetName(), "ALL_BUILD") == 0)
    {
    return false;
    }
  if(strcmp(l->GetName(), "ALL_BUILD") == 0)
    {
    return true;
    }
  return strcmp(l->GetName(), r->GetName()) < 0;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::OrderedTargetDependSet
::OrderedTargetDependSet(cmGlobalGenerator::TargetDependSet const& targets)
{
  for(cmGlobalGenerator::TargetDependSet::const_iterator ti =
        targets.begin(); ti != targets.end(); ++ti)
    {
    this->insert(*ti);
    }
}
