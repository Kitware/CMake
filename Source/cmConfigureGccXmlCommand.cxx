/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmConfigureGccXmlCommand.h"
#include "cmCacheManager.h"

cmConfigureGccXmlCommand::cmConfigureGccXmlCommand()
{
}

cmConfigureGccXmlCommand::~cmConfigureGccXmlCommand()
{
}

// cmConfigureGccXmlCommand
bool cmConfigureGccXmlCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // If the cache entry already exists, we are done.
  std::string cacheName = args[1];
  const char* cacheValue =
    m_Makefile->GetDefinition(cacheName.c_str());
  if(cacheValue && (std::string(cacheValue) != ""))
    { return true; }
  
  // Get the gccxml support directory location.  This is based on the
  // executable location.
  if(!this->GetSupportDirectory(args[0].c_str()))
    { return false; }
  
#if defined(_WIN32) && !defined(__CYGWIN__)
  // On Windows, we will just look at VcInclude/FLAGS.txt for now.
  if(!this->FindVcIncludeFlags())
    { return false; }
#else
  // On UNIX, we have to determine which compiler is being used, and
  // attempt to use that compiler's support directory.
  if(this->CompilerIsGCC())
    {
    if(!this->FindGccIncludeFlags())
      { return false; }
    }
  else if(this->CompilerIsMipsPro())
    {
    if(!this->FindMproIncludeFlags())
      { return false; }
    }
  else
    {
    this->SetError("Compiler is not supported by GCC-XML!\n");
    return false;
    }
#endif
  
  // Add the cache entry with the flags found.
  m_Makefile->AddCacheDefinition(
    cacheName.c_str(),
    m_Flags.c_str(),
    "Flags to GCC-XML to get it to parse the native compiler's headers.",
    cmCacheManager::STRING);
  
  return true;
}


/**
 * Given the location of the GCC-XML executable, find the root of the
 * support library tree.  Subdirectories of the returned location should
 * contain the compiler-specific support libraries.
 */
bool cmConfigureGccXmlCommand::GetSupportDirectory(const char* exeLoc)
{
  std::string gccxml = exeLoc;
  m_Makefile->ExpandVariablesInString(gccxml);

  if(!cmSystemTools::FileExists(gccxml.c_str()))
    {
    std::string err = "Can't find GCC-XML at given path: ";
    err += gccxml;
    this->SetError(err.c_str());
    return false;
    }
  
  std::string dir;
  std::string file;
  // Get the directory (also converts to unix slashes).
  cmSystemTools::SplitProgramPath(gccxml.c_str(), dir, file);
  
#if !defined(_WIN32) || defined(__CYGWIN__)
  // On UNIX platforms, we must replace the "/bin" suffix with
  // "/share/GCC_XML".  If there is no "/bin" suffix, we will assume
  // that the user has put everything in one directory, and not change
  // the path.
  if(dir.substr(dir.length()-4, 4) == "/bin")
    {
    dir = dir.substr(0, dir.length()-4) + "/share/GCC_XML";
    }
#endif
  
  m_SupportDir = dir;
  return true;
}


/**
 * Find the flags needed to use the Visual C++ support library.
 */
bool cmConfigureGccXmlCommand::FindVcIncludeFlags()
{
  std::string fname = m_SupportDir+"/VcInclude/FLAGS.txt";
  std::ifstream flagsFile(fname.c_str());
  
  if(!flagsFile)
    {
    std::string err = "Cannot open GCC-XML flags file \""+fname+"\".";
    this->SetError(err.c_str());
    return false;
    }
  
  // TODO: Replace this with a real implementation.
  char buf[4096];
  flagsFile.getline(buf, 4096);
  if(!flagsFile)
    {
    std::string err = "Error reading from GCC-XML flags file \""+fname+"\".";
    this->SetError(err.c_str());
    return false;
    }

  m_Flags = buf;
  
  return true;
}


/**
 * Find the flags needed to use the GCC support library.
 */
bool cmConfigureGccXmlCommand::FindGccIncludeFlags()
{
  std::string supportDir = m_SupportDir+"/GccInclude";
  if(!cmSystemTools::FileIsDirectory(supportDir.c_str()))
    {
    std::string err = "No GCC support library for GCC-XML.  Couldn't find directory \""+supportDir+"\".";
    this->SetError(err.c_str());
    return false;
    }
  
  // Try to run the find_gcc_options command.
  std::string command = supportDir+"/find_gcc_options";
  std::string flags;
  if(!cmSystemTools::RunCommand(command.c_str(), flags))
    {
    this->SetError("Could not run find_gcc_options!");
    return false;
    }  
  
  // Strip newline from end of flags.
  if((flags.length() > 0)
     && (flags[flags.length()-1] == '\n'))
    {
    flags = flags.substr(0, flags.length()-1);
    if((flags.length() > 0)
       && (flags[flags.length()-1] == '\r'))
      {
      flags = flags.substr(0, flags.length()-1);
      }
    }
  
  // Use the result of the command as the flags.
  m_Flags = flags;
  
  return true;
}


/**
 * Find the flags needed to use the MIPSpro support library.
 */
bool cmConfigureGccXmlCommand::FindMproIncludeFlags()
{
  std::string supportDir = m_SupportDir+"/MproInclude";
  if(!cmSystemTools::FileIsDirectory(supportDir.c_str()))
    {
    std::string err = "No MIPSpro support library for GCC-XML.  Couldn't find directory \""+supportDir+"\".";
    this->SetError(err.c_str());
    return false;
    }
  
  // Try to run the find_mpro_options command.
  std::string command = supportDir+"/find_mpro_options";
  std::string flags;
  if(!cmSystemTools::RunCommand(command.c_str(), flags))
    {
    this->SetError("Could not run find_mpro_options!");
    return false;
    }
  
  // Strip newline from end of flags.
  if((flags.length() > 0)
     && (flags[flags.length()-1] == '\n'))
    {
    flags = flags.substr(0, flags.length()-1);
    if((flags.length() > 0)
       && (flags[flags.length()-1] == '\r'))
      {
      flags = flags.substr(0, flags.length()-1);
      }
    }
  
  // Use the result of the command as the flags.
  m_Flags = flags;
  
  return true;
}


/**
 * Determine whether the compiler is GCC.
 */
bool cmConfigureGccXmlCommand::CompilerIsGCC() const
{
  const char* isGNU = m_Makefile->GetDefinition("CMAKE_COMPILER_IS_GNUCXX");
  return (isGNU && !cmSystemTools::IsOff(isGNU));
}


/**
 * Determine whether the compiler is MipsPro.
 */
bool cmConfigureGccXmlCommand::CompilerIsMipsPro() const
{
  const char* compiler = m_Makefile->GetDefinition("CMAKE_CXX_COMPILER");
  if(!compiler) { return false; }
  std::string command = compiler;
  command += " -version 2>&1";
  std::string output;
  if(!cmSystemTools::RunCommand(command.c_str(), output, false))
    { return false; }
  if(output.find("MIPSpro") != std::string::npos)
    {
    return true;
    }
  return false;
}
