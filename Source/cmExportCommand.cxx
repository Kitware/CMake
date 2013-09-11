/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExportCommand.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"

#include <cmsys/RegularExpression.hxx>

#include "cmExportBuildFileGenerator.h"

#if defined(__HAIKU__)
#include <StorageKit.h>
#endif

cmExportCommand::cmExportCommand()
:cmCommand()
,ArgumentGroup()
,Targets(&Helper, "TARGETS")
,Append(&Helper, "APPEND", &ArgumentGroup)
,Namespace(&Helper, "NAMESPACE", &ArgumentGroup)
,Filename(&Helper, "FILE", &ArgumentGroup)
,ExportOld(&Helper, "EXPORT_LINK_INTERFACE_LIBRARIES", &ArgumentGroup)
{
  // at first TARGETS
  this->Targets.Follows(0);
  // and after that the other options in any order
  this->ArgumentGroup.Follows(&this->Targets);
}


// cmExportCommand
bool cmExportCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with too few arguments");
    return false;
    }

  if(args[0] == "PACKAGE")
    {
    return this->HandlePackage(args);
    }

  std::vector<std::string> unknownArgs;
  this->Helper.Parse(&args, &unknownArgs);

  if (!unknownArgs.empty())
    {
    this->SetError("Unknown arguments.");
    return false;
    }

  if (this->Targets.WasFound() == false)
    {
    this->SetError("TARGETS option missing.");
    return false;
    }

  if(!this->Filename.WasFound())
    {
    this->SetError("FILE <filename> option missing.");
    return false;
    }

  // Make sure the file has a .cmake extension.
  if(cmSystemTools::GetFilenameLastExtension(this->Filename.GetCString())
     != ".cmake")
    {
    cmOStringStream e;
    e << "FILE option given filename \"" << this->Filename.GetString()
      << "\" which does not have an extension of \".cmake\".\n";
    this->SetError(e.str().c_str());
    return false;
    }

  // Get the file to write.
  std::string fname = this->Filename.GetString();
  if(cmSystemTools::FileIsFullPath(fname.c_str()))
    {
    if(!this->Makefile->CanIWriteThisFile(fname.c_str()))
      {
      cmOStringStream e;
      e << "FILE option given filename \"" << fname
        << "\" which is in the source tree.\n";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  else
    {
    // Interpret relative paths with respect to the current build dir.
    fname = this->Makefile->GetCurrentOutputDirectory();
    fname += "/";
    fname += this->Filename.GetString();
    }

  // Collect the targets to be exported.
  std::vector<cmTarget*> targets;
  for(std::vector<std::string>::const_iterator
      currentTarget = this->Targets.GetVector().begin();
      currentTarget != this->Targets.GetVector().end();
      ++currentTarget)
    {
    if (this->Makefile->IsAlias(currentTarget->c_str()))
      {
      cmOStringStream e;
      e << "given ALIAS target \"" << *currentTarget
        << "\" which may not be exported.";
      this->SetError(e.str().c_str());
      return false;
      }

    if(cmTarget* target =
       this->Makefile->GetLocalGenerator()->
       GetGlobalGenerator()->FindTarget(0, currentTarget->c_str()))
      {
      if((target->GetType() == cmTarget::EXECUTABLE) ||
         (target->GetType() == cmTarget::STATIC_LIBRARY) ||
         (target->GetType() == cmTarget::SHARED_LIBRARY) ||
         (target->GetType() == cmTarget::MODULE_LIBRARY) ||
         (target->GetType() == cmTarget::INTERFACE_LIBRARY))
        {
        targets.push_back(target);
        }
      else if(target->GetType() == cmTarget::OBJECT_LIBRARY)
        {
        cmOStringStream e;
        e << "given OBJECT library \"" << *currentTarget
          << "\" which may not be exported.";
        this->SetError(e.str().c_str());
        return false;
        }
      else
        {
        cmOStringStream e;
        e << "given target \"" << *currentTarget
          << "\" which is not an executable or library.";
        this->SetError(e.str().c_str());
        return false;
        }
      }
    else
      {
      cmOStringStream e;
      e << "given target \"" << *currentTarget
        << "\" which is not built by this project.";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Setup export file generation.
  cmExportBuildFileGenerator ebfg;
  ebfg.SetExportFile(fname.c_str());
  ebfg.SetNamespace(this->Namespace.GetCString());
  ebfg.SetAppendMode(this->Append.IsEnabled());
  ebfg.SetExports(&targets);
  ebfg.SetCommand(this);
  ebfg.SetExportOld(this->ExportOld.IsEnabled());

  this->Makefile->AddExportedTargetsFile(fname);

  // Compute the set of configurations exported.
  std::vector<std::string> configurationTypes;
  this->Makefile->GetConfigurations(configurationTypes);
  if(!configurationTypes.empty())
    {
    for(std::vector<std::string>::const_iterator
          ci = configurationTypes.begin();
        ci != configurationTypes.end(); ++ci)
      {
      ebfg.AddConfiguration(ci->c_str());
      }
    }
  else
    {
    ebfg.AddConfiguration("");
    }

  // Generate the import file.
  if(!ebfg.GenerateImportFile() && this->ErrorMessage.empty())
    {
    this->SetError("could not write export file.");
    return false;
    }

  // Report generated error message if any.
  if(!this->ErrorMessage.empty())
    {
    this->SetError(this->ErrorMessage.c_str());
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmExportCommand::HandlePackage(std::vector<std::string> const& args)
{
  // Parse PACKAGE mode arguments.
  enum Doing { DoingNone, DoingPackage };
  Doing doing = DoingPackage;
  std::string package;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(doing == DoingPackage)
      {
      package = args[i];
      doing = DoingNone;
      }
    else
      {
      cmOStringStream e;
      e << "PACKAGE given unknown argument: " << args[i];
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Verify the package name.
  if(package.empty())
    {
    this->SetError("PACKAGE must be given a package name.");
    return false;
    }
  const char* packageExpr = "^[A-Za-z0-9_.-]+$";
  cmsys::RegularExpression packageRegex(packageExpr);
  if(!packageRegex.find(package.c_str()))
    {
    cmOStringStream e;
    e << "PACKAGE given invalid package name \"" << package << "\".  "
      << "Package names must match \"" << packageExpr << "\".";
    this->SetError(e.str().c_str());
    return false;
    }

  // We store the current build directory in the registry as a value
  // named by a hash of its own content.  This is deterministic and is
  // unique with high probability.
  const char* outDir = this->Makefile->GetCurrentOutputDirectory();
  std::string hash = cmSystemTools::ComputeStringMD5(outDir);
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->StorePackageRegistryWin(package, outDir, hash.c_str());
#else
  this->StorePackageRegistryDir(package, outDir, hash.c_str());
#endif

  return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
# include <windows.h>
# undef GetCurrentDirectory
//----------------------------------------------------------------------------
void cmExportCommand::ReportRegistryError(std::string const& msg,
                                          std::string const& key,
                                          long err)
{
  cmOStringStream e;
  e << msg << "\n"
    << "  HKEY_CURRENT_USER\\" << key << "\n";
  char winmsg[1024];
  if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS, 0, err,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   winmsg, 1024, 0) > 0)
    {
    e << "Windows reported:\n"
      << "  " << winmsg;
    }
  this->Makefile->IssueMessage(cmake::WARNING, e.str());
}

//----------------------------------------------------------------------------
void cmExportCommand::StorePackageRegistryWin(std::string const& package,
                                              const char* content,
                                              const char* hash)
{
  std::string key = "Software\\Kitware\\CMake\\Packages\\";
  key += package;
  HKEY hKey;
  LONG err = RegCreateKeyEx(HKEY_CURRENT_USER,
                            key.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE, 0, &hKey, 0);
  if(err != ERROR_SUCCESS)
    {
    this->ReportRegistryError(
      "Cannot create/open registry key", key, err);
    return;
    }
  err = RegSetValueEx(hKey, hash, 0, REG_SZ, (BYTE const*)content,
                      static_cast<DWORD>(strlen(content)+1));
  RegCloseKey(hKey);
  if(err != ERROR_SUCCESS)
    {
    cmOStringStream msg;
    msg << "Cannot set registry value \"" << hash << "\" under key";
    this->ReportRegistryError(msg.str(), key, err);
    return;
    }
}
#else
//----------------------------------------------------------------------------
void cmExportCommand::StorePackageRegistryDir(std::string const& package,
                                              const char* content,
                                              const char* hash)
{
#if defined(__HAIKU__)
  BPath dir;
  if (find_directory(B_USER_SETTINGS_DIRECTORY, &dir) != B_OK)
    {
    return;
    }
  dir.Append("cmake/packages");
  dir.Append(package.c_str());
  std::string fname = dir.Path();
#else
  const char* home = cmSystemTools::GetEnv("HOME");
  if(!home)
    {
    return;
    }
  std::string fname = home;
  cmSystemTools::ConvertToUnixSlashes(fname);
  fname += "/.cmake/packages/";
  fname += package;
#endif
  cmSystemTools::MakeDirectory(fname.c_str());
  fname += "/";
  fname += hash;
  if(!cmSystemTools::FileExists(fname.c_str()))
    {
    cmGeneratedFileStream entry(fname.c_str(), true);
    if(entry)
      {
      entry << content << "\n";
      }
    else
      {
      cmOStringStream e;
      e << "Cannot create package registry file:\n"
        << "  " << fname << "\n"
        << cmSystemTools::GetLastSystemError() << "\n";
      this->Makefile->IssueMessage(cmake::WARNING, e.str());
      }
    }
}
#endif
