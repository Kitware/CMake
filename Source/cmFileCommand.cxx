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

#include "cmGlob.h"

#include <sys/types.h>
#include <sys/stat.h>

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
  else if ( subCommand == "INSTALL" )
    {
    return this->HandleInstallCommand(args);
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
    fileName = m_Makefile->GetCurrentDirectory();
    fileName += "/" + *i;
    }

  i++;

  for(;i != args.end(); ++i)
    {
    message += *i;
    }
  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir.c_str());

  std::ofstream file(fileName.c_str(), append?std::ios::app: std::ios::out);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for writting.";
    this->SetError(error.c_str());
    return false;
    }
  file << message;
  file.close();
  m_Makefile->AddWrittenFile(fileName.c_str());
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
    fileName = m_Makefile->GetCurrentDirectory();
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
  m_Makefile->AddDefinition(variable.c_str(), output.c_str());
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

#ifdef CMAKE_BUILD_WITH_CMAKE
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string variable = *i;
  i++;
  cmGlob g;
  g.SetRecurse(recurse);
  std::string output = "";
  bool first = true;
  for ( ; i != args.end(); ++i )
    {
    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      std::string expr = m_Makefile->GetCurrentDirectory();
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
  m_Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
#else
  (void)recurse;
  this->SetError("GLOB is not implemented in the bootstrap CMake");
  return false;
#endif
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
      expr = m_Makefile->GetCurrentDirectory();
      expr += "/" + *i;
      cdir = &expr;
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

  std::string destination = "";
  std::string stype = "FILES";
  const char* build_type = m_Makefile->GetDefinition("BUILD_TYPE");
  if ( build_type && strcmp(build_type, ".") == 0 )
    {
    build_type = 0;
    }
  if ( build_type && strncmp(build_type, ".\\", 2) == 0 )
    {
    build_type += 2;
    }

  const char* debug_postfix
    = m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
  if(!debug_postfix)
    {
    debug_postfix = "";
    }
  const char* destdir = cmSystemTools::GetEnv("DESTDIR");

  std::string extra_dir = "";
  int debug = 0;
  if ( build_type )
    {
    extra_dir = build_type;
    std::string btype = cmSystemTools::LowerCase(build_type);
    if ( strncmp(btype.c_str(), "debug", strlen("debug")) == 0 )
      {
      debug = 1;
      }
    }

  std::vector<std::string> files;
  int itype = cmTarget::INSTALL_FILES;

  std::vector<std::string>::size_type i = 0;
  i++; // Get rid of subcommand

  std::map<cmStdString, const char*> properties;

  bool in_files = false;
  bool in_properties = false;
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
      }
    else if ( *cstr == "PROPERTIES"  )
      {
      in_properties = true;
      in_files = false;
      }
    else if ( *cstr == "FILES" && !in_files)
      {
      in_files = true;
      in_properties = false;
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
      if ( ( ch1 >= 'a' && ch1 <= 'z' || ch1 >= 'a' && ch1 <= 'z' ) &&
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
    std::string errstring = "found file: " + destination +
      " where expecting directory with the same name.";
    this->SetError(errstring.c_str());
    return false;
    }

  const char* manifest_files = 
    m_Makefile->GetDefinition("CMAKE_INSTALL_MANIFEST_FILES");
  std::string smanifest_files;
  if ( manifest_files )
    {
    smanifest_files = manifest_files;
    }

  for ( i = 0; i < files.size(); i ++ )
    {
    std::string destfilewe
      = destination + "/"
      + cmSystemTools::GetFilenameWithoutExtension(files[i]);
    std::string ctarget = files[i].c_str();
    std::string fname = cmSystemTools::GetFilenameName(ctarget);
    std::string ext = cmSystemTools::GetFilenameExtension(ctarget);
    std::string fnamewe 
      = cmSystemTools::GetFilenameWithoutExtension(ctarget);
    std::string destfile = destfilewe;
    if ( ext.size() )
      {
      destfile += ext;
      }
    switch( itype )
      {
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
      if ( debug )
        {
        fname = fnamewe + debug_postfix + ext;
        destfile = destfilewe + debug_postfix + ext;
        }
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
          std::string libname = destfile;
          std::string soname = destfile;
          std::string soname_nopath = fname;
          soname += ".";
          soname += lib_soversion;
          soname_nopath += ".";
          soname_nopath += lib_soversion;

          fname += ".";
          fname += lib_version;
          destfile += ".";
          destfile += lib_version;

          cmSystemTools::RemoveFile(soname.c_str());
          cmSystemTools::RemoveFile(libname.c_str());

          if (!cmSystemTools::CreateSymlink(soname_nopath.c_str(), libname.c_str()) )
            {
            std::string errstring = "error when creating symlink from: " + libname + " to " + soname_nopath;
            this->SetError(errstring.c_str());
            return false;
            }
          smanifest_files += ";";
          smanifest_files += libname.substr(destDirLength);;
          if ( destfile != soname )
            {
            if ( !cmSystemTools::CreateSymlink(fname.c_str(), soname.c_str()) )
              {
              std::string errstring = "error when creating symlink from: " + soname + " to " + fname;
              this->SetError(errstring.c_str());
              return false;
              }
            smanifest_files += ";";
            smanifest_files += soname.substr(destDirLength);
            }
          }
        cmOStringStream str;
        str << cmSystemTools::GetFilenamePath(ctarget) << "/";
        if ( extra_dir.size() > 0 )
          {
          str << extra_dir << "/";
          }
        str << fname;
        ctarget = str.str();
        }
      break;
    case cmTarget::EXECUTABLE:
      if ( extra_dir.size() > 0 )
        {
        cmOStringStream str;
        str << cmSystemTools::GetFilenamePath(ctarget) 
          << "/" << extra_dir << "/" 
          << fname;
        ctarget = str.str();
        }
      break;
      }

    if ( !cmSystemTools::SameFile(ctarget.c_str(), destfile.c_str()) )
      {
      if ( cmSystemTools::FileExists(ctarget.c_str()) )
        {
        cmSystemTools::RemoveFile(destfile.c_str());
        if ( !cmSystemTools::CopyFileAlways(ctarget.c_str(), 
            destination.c_str()) )
          {
          std::string errstring = "cannot copy file: " + ctarget + 
            " to directory : " + destination + ".";
          this->SetError(errstring.c_str());
          return false;
          }
        switch( itype )
          {
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::EXECUTABLE:
        case cmTarget::INSTALL_PROGRAMS:

          if ( !cmSystemTools::SetPermissions(destfile.c_str(), 
#if defined( _MSC_VER ) || defined( __MINGW32__ )
              S_IREAD | S_IWRITE | S_IEXEC
#elif defined( __BORLANDC__ )
              S_IRUSR | S_IWUSR | S_IXUSR
#else
              S_IRUSR | S_IWUSR | S_IXUSR | 
              S_IRGRP | S_IXGRP | 
              S_IROTH | S_IXOTH 
#endif
          ) )
            {
            cmOStringStream err;
            err << "Problem setting permissions on file: " << destfile.c_str();
            perror(err.str().c_str());
            }
          }
        smanifest_files += ";";
        smanifest_files += destfile.substr(destDirLength);
        }
      else
        {
        if ( !optional )
          {
          std::string errstring = "cannot find file: " + 
            ctarget + " to install.";
          this->SetError(errstring.c_str());
          return false;
          }
        }
      }
    }
  m_Makefile->AddDefinition("CMAKE_INSTALL_MANIFEST_FILES",
    smanifest_files.c_str());

  return true;
}
