/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmFileCommand.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>

// cmLibraryCommand
bool cmFileCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("must be called with at least two arguments.");
    return false;
    }
  std::string subCommand = args[0];
  if ( subCommand == "WRITE" )
    {
    return this->HandleWriteCommand(args, false);
    }
  else if ( subCommand == "APPEND" )
    {
    return this->HandleWriteCommand(args, true);
    }
  else if ( subCommand == "READ" )
    {
    return this->HandleReadCommand(args);
    }
  else if ( subCommand == "GLOB" )
    {
    return this->HandleGlobCommand(args, false);
    }
  else if ( subCommand == "GLOB_RECURSE" )
    {
    return this->HandleGlobCommand(args, true);
    }
  else if ( subCommand == "MAKE_DIRECTORY" )
    {
    return this->HandleMakeDirectoryCommand(args);
    }
  else if ( subCommand == "REMOVE" )
    {
    return this->HandleRemove(args, false);
    }
  else if ( subCommand == "REMOVE_RECURSE" )
    {
    return this->HandleRemove(args, true);
    }
  else if ( subCommand == "INSTALL" )
    {
    return this->HandleInstallCommand(args);
    }
  else if ( subCommand == "RELATIVE_PATH" )
    {
    return this->HandleRelativePathCommand(args);
    }
  else if ( subCommand == "TO_CMAKE_PATH" )
    {
    return this->HandleCMakePathCommand(args, false);
    }
  else if ( subCommand == "TO_NATIVE_PATH" )
    {
    return this->HandleCMakePathCommand(args, true);
    }

  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleWriteCommand(std::vector<std::string> const& args,
  bool append)
{
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string fileName = *i;
  if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
    {
    fileName = this->Makefile->GetCurrentDirectory();
    fileName += "/" + *i;
    }

  i++;

  for(;i != args.end(); ++i)
    {
    message += *i;
    }
  if ( !this->Makefile->CanIWriteThisFile(fileName.c_str()) )
    {
    std::string e
      = "attempted to write a file: " + fileName +
      " into a source directory.";
    this->SetError(e.c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir.c_str());

  mode_t mode =
#if defined( _MSC_VER ) || defined( __MINGW32__ )
    S_IREAD | S_IWRITE
#elif defined( __BORLANDC__ )
    S_IRUSR | S_IWUSR
#else
    S_IRUSR | S_IWUSR |
    S_IRGRP |
    S_IROTH
#endif
    ;

  // Set permissions to writable
  if ( cmSystemTools::GetPermissions(fileName.c_str(), mode) )
    {
    cmSystemTools::SetPermissions(fileName.c_str(),
#if defined( _MSC_VER ) || defined( __MINGW32__ )
      S_IREAD | S_IWRITE
#else
      S_IRUSR | S_IWUSR
#endif
    );
    }
  // If GetPermissions fails, pretend like it is ok. File open will fail if
  // the file is not writable
  std::ofstream file(fileName.c_str(), append?std::ios::app: std::ios::out);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for writing.";
    this->SetError(error.c_str());
    return false;
    }
  file << message;
  file.close();
  cmSystemTools::SetPermissions(fileName.c_str(), mode);
  this->Makefile->AddWrittenFile(fileName.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleReadCommand(std::vector<std::string> const& args)
{
  if ( args.size() != 3 )
    {
    this->SetError("READ must be called with two additional arguments");
    return false;
    }

  std::string fileName = args[1];
  if ( !cmsys::SystemTools::FileIsFullPath(args[1].c_str()) )
    {
    fileName = this->Makefile->GetCurrentDirectory();
    fileName += "/" + args[1];
    }

  std::string variable = args[2];
  std::ifstream file(fileName.c_str(), std::ios::in);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for reading.";
    this->SetError(error.c_str());
    return false;
    }

  std::string output;
  std::string line;
  bool has_newline = false;
  while ( cmSystemTools::GetLineFromStream(file, line, &has_newline) )
    {
    output += line;
    if ( has_newline )
      {
      output += "\n";
      }
    }
  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleGlobCommand(std::vector<std::string> const& args,
  bool recurse)
{
  if ( args.size() < 2 )
    {
    this->SetError("GLOB requires at least a variable name");
    return false;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string variable = *i;
  i++;
  cmsys::Glob g;
  g.SetRecurse(recurse);
  std::string output = "";
  bool first = true;
  for ( ; i != args.end(); ++i )
    {
    if ( *i == "RELATIVE" )
      {
      ++i; // skip RELATIVE
      if ( i == args.end() )
        {
        this->SetError("GLOB requires a directory after the RELATIVE tag");
        return false;
        }
      g.SetRelative(i->c_str());
      ++i;
      if(i == args.end())
        {
        this->SetError("GLOB requires a glob expression after the directory");
        return false;
        }
      }
    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      std::string expr = this->Makefile->GetCurrentDirectory();
      // Handle script mode
      if ( expr.size() > 0 )
        {
        expr += "/" + *i;
        g.FindFiles(expr);
        }
      else
        {
        g.FindFiles(*i);
        }
      }
    else
      {
      g.FindFiles(*i);
      }
    std::vector<std::string>::size_type cc;
    std::vector<std::string>& files = g.GetFiles();
    for ( cc = 0; cc < files.size(); cc ++ )
      {
      if ( !first )
        {
        output += ";";
        }
      output += files[cc];
      first = false;
      }
    }
  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleMakeDirectoryCommand(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string expr;
  for ( ; i != args.end(); ++i )
    {
    const std::string* cdir = &(*i);
    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      expr = this->Makefile->GetCurrentDirectory();
      expr += "/" + *i;
      cdir = &expr;
      }
    if ( !this->Makefile->CanIWriteThisFile(cdir->c_str()) )
      {
      std::string e = "attempted to create a directory: " + *cdir
        + " into a source directory.";
      this->SetError(e.c_str());
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }
    if ( !cmSystemTools::MakeDirectory(cdir->c_str()) )
      {
      std::string error = "problem creating directory: " + *cdir;
      this->SetError(error.c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleInstallCommand(
  std::vector<std::string> const& args)
{
  if ( args.size() < 6 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string rename = "";
  std::string destination = "";
  std::string stype = "FILES";
  const char* destdir = cmSystemTools::GetEnv("DESTDIR");

  std::set<cmStdString> components;
  std::set<cmStdString> configurations;
  std::vector<std::string> files;
  int itype = cmTarget::INSTALL_FILES;

  std::vector<std::string>::size_type i = 0;
  i++; // Get rid of subcommand

  std::map<cmStdString, const char*> properties;

  // Build a table of permissions flags.
#if defined(_WIN32) && !defined(__CYGWIN__)
  mode_t mode_owner_read = S_IREAD;
  mode_t mode_owner_write = S_IWRITE;
  mode_t mode_owner_execute = S_IEXEC;
  mode_t mode_group_read = 0;
  mode_t mode_group_write = 0;
  mode_t mode_group_execute = 0;
  mode_t mode_world_read = 0;
  mode_t mode_world_write = 0;
  mode_t mode_world_execute = 0;
  mode_t mode_setuid = 0;
  mode_t mode_setgid = 0;
#else
  mode_t mode_owner_read = S_IRUSR;
  mode_t mode_owner_write = S_IWUSR;
  mode_t mode_owner_execute = S_IXUSR;
  mode_t mode_group_read = S_IRGRP;
  mode_t mode_group_write = S_IWGRP;
  mode_t mode_group_execute = S_IXGRP;
  mode_t mode_world_read = S_IROTH;
  mode_t mode_world_write = S_IWOTH;
  mode_t mode_world_execute = S_IXOTH;
  mode_t mode_setuid = S_ISUID;
  mode_t mode_setgid = S_ISGID;
#endif

  bool in_files = false;
  bool in_properties = false;
  bool in_permissions = false;
  bool in_components = false;
  bool in_configurations = false;
  bool use_given_permissions = false;
  mode_t permissions = 0;
  bool optional = false;
  for ( ; i != args.size(); ++i )
    {
    const std::string* cstr = &args[i];
    if ( *cstr == "DESTINATION" && i < args.size()-1 )
      {
      i++;
      destination = args[i];
      in_files = false;
      in_properties = false;
      in_permissions = false;
      in_components = false;
      in_configurations = false;
      }
    else if ( *cstr == "TYPE" && i < args.size()-1 )
      {
      i++;
      stype = args[i];
      if ( args[i+1] == "OPTIONAL" )
        {
        i++;
        optional = true;
        }
      in_properties = false;
      in_files = false;
      in_permissions = false;
      in_components = false;
      in_configurations = false;
      }
    else if ( *cstr == "RENAME" && i < args.size()-1 )
      {
      i++;
      rename = args[i];
      in_properties = false;
      in_files = false;
      in_permissions = false;
      in_components = false;
      in_configurations = false;
      }
    else if ( *cstr == "PROPERTIES"  )
      {
      in_properties = true;
      in_files = false;
      in_permissions = false;
      in_components = false;
      in_configurations = false;
      }
    else if ( *cstr == "PERMISSIONS"  )
      {
      use_given_permissions = true;
      in_properties = false;
      in_files = false;
      in_permissions = true;
      in_components = false;
      in_configurations = false;
      }
    else if ( *cstr == "COMPONENTS"  )
      {
      in_properties = false;
      in_files = false;
      in_permissions = false;
      in_components = true;
      in_configurations = false;
      }
    else if ( *cstr == "CONFIGURATIONS"  )
      {
      in_properties = false;
      in_files = false;
      in_permissions = false;
      in_components = false;
      in_configurations = true;
      }
    else if ( *cstr == "FILES" && !in_files)
      {
      in_files = true;
      in_properties = false;
      in_permissions = false;
      in_components = false;
      in_configurations = false;
      }
    else if ( in_properties && i < args.size()-1 )
      {
      properties[args[i]] = args[i+1].c_str();
      i++;
      }
    else if ( in_files )
      {
      files.push_back(*cstr);
      }
    else if ( in_components )
      {
      components.insert(*cstr);
      }
    else if ( in_configurations )
      {
      configurations.insert(cmSystemTools::UpperCase(*cstr));
      }
    else if(in_permissions && args[i] == "OWNER_READ")
      {
      permissions |= mode_owner_read;
      }
    else if(in_permissions && args[i] == "OWNER_WRITE")
      {
      permissions |= mode_owner_write;
      }
    else if(in_permissions && args[i] == "OWNER_EXECUTE")
      {
      permissions |= mode_owner_execute;
      }
    else if(in_permissions && args[i] == "GROUP_READ")
      {
      permissions |= mode_group_read;
      }
    else if(in_permissions && args[i] == "GROUP_WRITE")
      {
      permissions |= mode_group_write;
      }
    else if(in_permissions && args[i] == "GROUP_EXECUTE")
      {
      permissions |= mode_group_execute;
      }
    else if(in_permissions && args[i] == "WORLD_READ")
      {
      permissions |= mode_world_read;
      }
    else if(in_permissions && args[i] == "WORLD_WRITE")
      {
      permissions |= mode_world_write;
      }
    else if(in_permissions && args[i] == "WORLD_EXECUTE")
      {
      permissions |= mode_world_execute;
      }
    else if(in_permissions && args[i] == "SETUID")
      {
      permissions |= mode_setuid;
      }
    else if(in_permissions && args[i] == "SETGID")
      {
      permissions |= mode_setgid;
      }
    else
      {
      this->SetError("called with inappropriate arguments");
      return false;
      }
    }

  if ( destination.size() < 2 )
    {
    this->SetError("called with inapropriate arguments. "
      "No DESTINATION provided or .");
    return false;
    }

  // Check for component-specific installation.
  const char* cmake_install_component =
    this->Makefile->GetDefinition("CMAKE_INSTALL_COMPONENT");
  if(cmake_install_component && *cmake_install_component)
    {
    // This install rule applies only if it is associated with the
    // current component.
    if(components.find(cmake_install_component) == components.end())
      {
      return true;
      }
    }

  // Check for configuration-specific installation.
  if(!configurations.empty())
    {
    std::string cmake_install_configuration =
      cmSystemTools::UpperCase(
        this->Makefile->GetSafeDefinition("CMAKE_INSTALL_CONFIG_NAME"));
    if(cmake_install_configuration.empty())
      {
      // No configuration specified for installation but this install
      // rule is configuration-specific.  Skip it.
      return true;
      }
    else if(configurations.find(cmake_install_configuration) ==
            configurations.end())
      {
      // This rule is specific to a configuration not being installed.
      return true;
      }
    }

  int destDirLength = 0;
  if ( destdir && *destdir )
    {
    std::string sdestdir = destdir;
    cmSystemTools::ConvertToUnixSlashes(sdestdir);

    char ch1 = destination[0];
    char ch2 = destination[1];
    char ch3 = 0;
    if ( destination.size() > 2 )
      {
      ch3 = destination[2];
      }
    int skip = 0;
    if ( ch1 != '/' )
      {
      int relative = 0;
      if ( ( ch1 >= 'a' && ch1 <= 'z' || ch1 >= 'A' && ch1 <= 'Z' ) &&
        ch2 == ':' )
        {
        // Assume windows
        // let's do some destdir magic:
        skip = 2;
        if ( ch3 != '/' )
          {
          relative = 1;
          }
        }
      else
        {
        relative = 1;
        }
      if ( relative )
        {
        // This is relative path on unix or windows. Since we are doing
        // destdir, this case does not make sense.
        this->SetError("called with relative DESTINATION. This "
          "does not make sense when using DESTDIR. Specify "
          "absolute path or remove DESTDIR environment variable.");
        return false;
        }
      }
    else
      {
      if ( ch2 == '/' )
        {
        // looks like a network path.
        this->SetError("called with network path DESTINATION. This "
          "does not make sense when using DESTDIR. Specify local "
          "absolute path or remove DESTDIR environment variable.");
        return false;
        }
      }
    destination = sdestdir + (destination.c_str() + skip);
    destDirLength = int(sdestdir.size());
    }

  if ( files.size() == 0 )
    {
    this->SetError(
      "called with inapropriate arguments. No FILES provided.");
    return false;
    }
  if ( stype == "EXECUTABLE" )
    {
    itype = cmTarget::EXECUTABLE;
    }
  else if ( stype == "PROGRAM" )
    {
    itype = cmTarget::INSTALL_PROGRAMS;
    }
  else if ( stype == "STATIC_LIBRARY" )
    {
    itype = cmTarget::STATIC_LIBRARY;
    }
  else if ( stype == "SHARED_LIBRARY" )
    {
    itype = cmTarget::SHARED_LIBRARY;
    }
  else if ( stype == "MODULE" )
    {
    itype = cmTarget::MODULE_LIBRARY;
    }
  else if ( stype == "DIRECTORY" )
    {
    itype = cmTarget::INSTALL_DIRECTORY;
    }

  if ( !cmSystemTools::FileExists(destination.c_str()) )
    {
    if ( !cmSystemTools::MakeDirectory(destination.c_str()) )
      {
      std::string errstring = "cannot create directory: " + destination +
        ". Maybe need administrative privileges.";
      this->SetError(errstring.c_str());
      return false;
      }
    }
  if ( !cmSystemTools::FileIsDirectory(destination.c_str()) )
    {
    std::string errstring = "INSTALL destination: " + destination +
      " is not a directory.";
    this->SetError(errstring.c_str());
    return false;
    }

  // Check rename form.
  if(!rename.empty())
    {
    if(itype != cmTarget::INSTALL_FILES)
      {
      this->SetError("INSTALL option RENAME may be used only with FILES.");
      return false;
      }
    if(files.size() > 1)
      {
      this->SetError("INSTALL option RENAME may be used only with one file.");
      return false;
      }
    }

  // If permissions were not specified set default permissions for
  // this target type.
  if(!use_given_permissions)
    {
    switch(itype)
      {
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
#if defined(__linux__)
        // Use read/write permissions.
        permissions = 0;
        permissions |= mode_owner_read;
        permissions |= mode_owner_write;
        permissions |= mode_group_read;
        permissions |= mode_world_read;
        break;
#endif
      case cmTarget::EXECUTABLE:
      case cmTarget::INSTALL_PROGRAMS:
        // Use read/write/executable permissions.
        permissions = 0;
        permissions |= mode_owner_read;
        permissions |= mode_owner_write;
        permissions |= mode_owner_execute;
        permissions |= mode_group_read;
        permissions |= mode_group_execute;
        permissions |= mode_world_read;
        permissions |= mode_world_execute;
        break;
      default:
        // Use read/write permissions.
        permissions = 0;
        permissions |= mode_owner_read;
        permissions |= mode_owner_write;
        permissions |= mode_group_read;
        permissions |= mode_world_read;
        break;
      }
    }

  // Get the current manifest.
  const char* manifest_files =
    this->Makefile->GetDefinition("CMAKE_INSTALL_MANIFEST_FILES");
  std::string smanifest_files;
  if ( manifest_files )
    {
    smanifest_files = manifest_files;
    }

  // Check whether files should be copied always or only if they have
  // changed.
  bool copy_always =
    cmSystemTools::IsOn(cmSystemTools::GetEnv("CMAKE_INSTALL_ALWAYS"));

  // Handle each file listed.
  for ( i = 0; i < files.size(); i ++ )
    {
    // Split the input file into its directory and name components.
    std::string fromDir = cmSystemTools::GetFilenamePath(files[i]);
    std::string fromName = cmSystemTools::GetFilenameName(files[i]);

    // Compute the full path to the destination file.
    std::string toFile = destination;
    toFile += "/";
    toFile += rename.empty()? fromName : rename;

    // Handle type-specific installation details.
    switch(itype)
      {
      case cmTarget::MODULE_LIBRARY:
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
        {
        // Handle shared library versioning
        const char* lib_version = 0;
        const char* lib_soversion = 0;
        if ( properties.find("VERSION") != properties.end() )
          {
          lib_version = properties["VERSION"];
          }
        if ( properties.find("SOVERSION") != properties.end() )
          {
          lib_soversion = properties["SOVERSION"];
          }
        if ( !lib_version && lib_soversion )
          {
          lib_version = lib_soversion;
          }
        if ( !lib_soversion && lib_version )
          {
          lib_soversion = lib_version;
          }
        if ( lib_version && lib_soversion )
          {
          std::string libname = toFile;
          std::string soname = toFile;
          std::string soname_nopath = fromName;
          this->ComputeVersionedName(soname, lib_soversion);
          this->ComputeVersionedName(soname_nopath, lib_soversion);
          this->ComputeVersionedName(fromName, lib_version);
          this->ComputeVersionedName(toFile, lib_version);

          cmSystemTools::RemoveFile(soname.c_str());
          cmSystemTools::RemoveFile(libname.c_str());

          if (!cmSystemTools::CreateSymlink(soname_nopath.c_str(), 
                                            libname.c_str()) )
            {
            std::string errstring = "error when creating symlink from: " 
              + libname + " to " + soname_nopath;
            this->SetError(errstring.c_str());
            return false;
            }
          smanifest_files += ";";
          smanifest_files += libname.substr(destDirLength);;
          if ( toFile != soname )
            {
            if ( !cmSystemTools::CreateSymlink(fromName.c_str(), 
                                               soname.c_str()) )
              {
              std::string errstring = "error when creating symlink from: " 
                + soname + " to " + fromName;
              this->SetError(errstring.c_str());
              return false;
              }
            smanifest_files += ";";
            smanifest_files += soname.substr(destDirLength);
            }
          }
        }
        break;
      case cmTarget::EXECUTABLE:
        {
        // Handle executable versioning
        const char* exe_version = 0;
        if ( properties.find("VERSION") != properties.end() )
          {
          exe_version = properties["VERSION"];
          }
        if ( exe_version )
          {
          std::string exename = toFile;
          std::string exename_nopath = fromName;
          exename_nopath += "-";
          exename_nopath += exe_version;

          fromName += "-";
          fromName += exe_version;
          toFile += "-";
          toFile += exe_version;

          cmSystemTools::RemoveFile(exename.c_str());

          if (!cmSystemTools::CreateSymlink(exename_nopath.c_str(), 
                                            exename.c_str()) )
            {
            std::string errstring = "error when creating symlink from: " 
              + exename + " to " + exename_nopath;
            this->SetError(errstring.c_str());
            return false;
            }
          smanifest_files += ";";
          smanifest_files += exename.substr(destDirLength);
          }
        }
        break;
      }

    // Construct the full path to the source file.  The file name may
    // have been changed above.
    std::string fromFile = fromDir;
    fromFile += "/";
    fromFile += fromName;

    std::string message;
    if(!cmSystemTools::SameFile(fromFile.c_str(), toFile.c_str()))
      {
      if(itype == cmTarget::INSTALL_DIRECTORY &&
        cmSystemTools::FileIsDirectory(fromFile.c_str()))
        {
        // We will install this file.  Display the information.
        message = "Installing ";
        message += toFile.c_str();
        this->Makefile->DisplayStatus(message.c_str(), -1);
        if(!cmSystemTools::CopyADirectory(fromFile.c_str(), toFile.c_str(),
                                          copy_always))
          {
          cmOStringStream e;
          e << "INSTALL cannot copy directory \"" << fromFile
            << "\" to \"" << toFile + "\".";
          this->SetError(e.str().c_str());
          return false;
          }
        }
      else if(cmSystemTools::FileExists(fromFile.c_str()))
        {
        // We will install this file.  Display the information.
        message = "Installing ";
        message += toFile.c_str();
        this->Makefile->DisplayStatus(message.c_str(), -1);

        // Copy the file.
        if(!cmSystemTools::CopyAFile(fromFile.c_str(), toFile.c_str(),
                                     copy_always))
          {
          cmOStringStream e;
          e << "INSTALL cannot copy file \"" << fromFile
            << "\" to \"" << toFile + "\".";
          this->SetError(e.str().c_str());
          return false;
          }

        // Perform post-installation processing on the file depending
        // on its type.
#if defined(__APPLE_CC__)
        // Static libraries need ranlib on this platform.
        if(itype == cmTarget::STATIC_LIBRARY)
          {
          std::string ranlib = "ranlib ";
          ranlib += cmSystemTools::ConvertToOutputPath(toFile.c_str());
          if(!cmSystemTools::RunSingleCommand(ranlib.c_str()))
            {
            std::string err = "ranlib failed: ";
            err += ranlib;
            this->SetError(err.c_str());
            return false;
            }
          }
#endif

        // Set permissions of the destination file.
        if(!cmSystemTools::SetPermissions(toFile.c_str(), permissions))
          {
          cmOStringStream e;
          e << "Problem setting permissions on file \""
            << toFile.c_str() << "\"";
          this->SetError(e.str().c_str());
          return false;
          }

        // Add the file to the manifest.
        smanifest_files += ";";
        smanifest_files += toFile.substr(destDirLength);
        }
      else if(!optional)
        {
        // The input file does not exist and installation is not optional.
        cmOStringStream e;
        e << "INSTALL cannot find file \"" << fromFile << "\" to install.";
        this->SetError(e.str().c_str());
        return false;
        }
      }
    }

  // Save the updated install manifest.
  this->Makefile->AddDefinition("CMAKE_INSTALL_MANIFEST_FILES",
                            smanifest_files.c_str());

  return true;
}

//----------------------------------------------------------------------------
void cmFileCommand::ComputeVersionedName(std::string& name,
                                         const char* version)
{
#if defined(__APPLE__)
  std::string ext;
  kwsys_stl::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != name.npos)
    {
    ext = name.substr(dot_pos, name.npos);
    name = name.substr(0, dot_pos);
    }
#endif
  name += ".";
  name += version;
#if defined(__APPLE__)
  name += ext;
#endif
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleRelativePathCommand(
  std::vector<std::string> const& args)
{
  if(args.size() != 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const std::string& outVar = args[1];
  const std::string& directoryName = args[2];
  const std::string& fileName = args[3];

  if(!cmSystemTools::FileIsFullPath(directoryName.c_str()))
    {
    std::string errstring = 
      "RelativePath must be passed a full path to the directory: " 
      + directoryName;
    this->SetError(errstring.c_str());
    return false;
    }
  if(!cmSystemTools::FileIsFullPath(fileName.c_str()))
    {
    std::string errstring = 
      "RelativePath must be passed a full path to the file: " 
      + fileName;
    this->SetError(errstring.c_str());
    return false;
    }

  std::string res = cmSystemTools::RelativePath(directoryName.c_str(), 
                                                fileName.c_str());
  this->Makefile->AddDefinition(outVar.c_str(),
    res.c_str());
  return true;
}


//----------------------------------------------------------------------------
bool cmFileCommand::HandleRemove(std::vector<std::string> const& args,
                                 bool recurse)
{

  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand
  for(;i != args.end(); ++i)
    {
    if(cmSystemTools::FileIsDirectory(i->c_str()) && recurse)
      {
      cmSystemTools::RemoveADirectory(i->c_str());
      }
    else
      {
      cmSystemTools::RemoveFile(i->c_str());
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleCMakePathCommand(std::vector<std::string>
                                           const& args, 
                                           bool nativePath)
{
  std::vector<std::string>::const_iterator i = args.begin();
  if(args.size() != 3)
    {
    this->SetError("FILE(SYSTEM_PATH ENV result) must be called with "
                   "only three arguments.");
    return false;
    }
  i++; // Get rid of subcommand
#if defined(_WIN32) && !defined(__CYGWIN__)
  char pathSep = ';';
#else
  char pathSep = ':';
#endif
  std::vector<cmsys::String> path = cmSystemTools::SplitString(i->c_str(), 
                                                             pathSep);
  i++;
  const char* var =  i->c_str();
  std::string value;
  for(std::vector<cmsys::String>::iterator j = path.begin();
      j != path.end(); ++j)
    {
    if(j != path.begin())
      {
      value += ";";
      }
    if(!nativePath)
      {
      cmSystemTools::ConvertToUnixSlashes(*j);
      }
    else
      {
      *j = cmSystemTools::ConvertToOutputPath(j->c_str());
      // remove double quotes in the path
      cmsys::String& s = *j;
      
      if(s.size() > 1 && s[0] == '\"' && s[s.size()-1] == '\"')
        {
        s = s.substr(1,s.size()-2);
        }
      }
    value += *j;
    }
  this->Makefile->AddDefinition(var, value.c_str());
  return true;
}


