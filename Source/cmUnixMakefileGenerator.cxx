/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmClassFile.h"
#include "cmMakeDepend.h"

void cmUnixMakefileGenerator::GenerateMakefile()
{
  // Generate depends 
  cmMakeDepend md;
  md.SetMakefile(m_Makefile);
  md.DoDepends();
  // output the makefile fragment
  this->OutputMakefile("CMakeTargets.make"); 
}


// This is where CMakeTargets.make is generated
void cmUnixMakefileGenerator::OutputMakefile(const char* file)
{
  // Create sub directories fro aux source directories
  std::vector<std::string>& auxSourceDirs = 
    m_Makefile->GetAuxSourceDirectories();
  if( auxSourceDirs.size() )
    {
    // For the case when this is running as a remote build
    // on unix, make the directory
    for(std::vector<std::string>::iterator i = auxSourceDirs.begin();
        i != auxSourceDirs.end(); ++i)
      {
      cmSystemTools::MakeDirectory(i->c_str());
      }
    }
  std::ofstream fout(file);
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", file);
    return;
    }
  this->OutputMakeFlags(fout);
  this->OutputSourceToObjectList(fout);
  this->OutputVerbatim(fout);
  this->OutputExecutableRules(fout);
  this->OutputSubDirectoryRules(fout);
  this->OutputDepends(fout);
}

// Output the LIBRARY and SRC_OBJS list based on
// the library name and cmClassFile objects in the
// makefile
void cmUnixMakefileGenerator::OutputSourceToObjectList(std::ostream& fout)
{
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  if(Classes.size() == 0)
    {
    return;
    }
  // Ouput Library name if there are SRC_OBJS
  if(strlen(m_Makefile->GetLibraryName()) > 0)
    {
    fout << "LIBRARY = " <<  m_Makefile->GetLibraryName() << "\n\n";
    fout << "BUILD_LIB_FILE = lib${LIBRARY}${CMAKE_LIB_EXT}\n\n";
    }
  // Output SRC_OBJ list for all the classes to be compiled
  fout << "SRC_OBJ = \\\n";
  for(unsigned int i = 0; i < Classes.size(); i++)
    {
    if(!Classes[i].m_AbstractClass && !Classes[i].m_HeaderFileOnly
       && !Classes[i].m_IsExecutable)
      {
      fout << Classes[i].m_ClassName << ".o ";
      if(i ==  Classes.size() -1)
        {
        fout << "\n\n";
        }
      else
        {
        fout << "\\\n";
        }
      }
    }
  fout << "\n";
}


// output the list of libraries that the executables 
// in this makefile will depend on.
void cmUnixMakefileGenerator::OutputDependLibraries(std::ostream& fout)
{
  std::vector<std::string>& libs = m_Makefile->GetLinkLibraries();
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  std::vector<std::string>::iterator dir, lib;
  // Search the list of libraries that will be linked into
  // the executable
  for(lib = libs.begin(); lib != libs.end(); ++lib)
    {
    bool found = false;
    // loop over the list of directories that the libraries might
    // be in, looking for a LIBRARY=(lib) line.
    for(dir = libdirs.begin(); dir != libdirs.end() && !found; ++dir)
      {
      std::string expression = "LIBRARY.*=.*";
      expression += lib->c_str();
      if(cmSystemTools::Grep(dir->c_str(), "CMakeTargets.make",
                             expression.c_str()))
        {
        std::string libpath = *dir;
        libpath += "/lib";
        libpath += *lib;
        libpath += "${CMAKE_LIB_EXT}";
        fout << libpath << " ";
        found = true;
        }
      }
    }
  fout << "\n";
}


// output make include flags
void cmUnixMakefileGenerator::OutputMakeFlags(std::ostream& fout)
{
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    fout << "-I" << i->c_str() << " ";
    }
  fout << " ${LOCAL_INCLUDE_FLAGS} ";
  fout << "\n";
  fout << "default_target: all\n\n";
  // see if there are files to compile in this makefile
  // These are used for both libraries and executables
}

// output verbatim section
void cmUnixMakefileGenerator::OutputVerbatim(std::ostream& fout)
{
    std::vector<std::string>& MakeVerbatim = m_Makefile->GetMakeVerbatim();
  // Ouput user make text embeded in the input file
  for(unsigned int i =0; i < MakeVerbatim.size(); i++)
    {
    fout << MakeVerbatim[i] << "\n";
    }
  fout << "\n\n";
  
}

// output executables
void cmUnixMakefileGenerator::OutputExecutableRules(std::ostream& fout)
{
  if(!m_Makefile->HasExecutables())
    {
    return ;
    }
  // collect all the flags needed for linking libraries
  std::string linkLibs;        
  std::vector<std::string>::iterator j;
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(j = libdirs.begin(); j != libdirs.end(); ++j)
    { 
    std::string::size_type pos = (*j).find("-L");
    if((pos == std::string::npos || pos > 0)
       && (*j).find("${") == std::string::npos)
      {
      linkLibs += "-L";
      }
    linkLibs += *j;
    linkLibs += " ";
    }
  std::vector<std::string>& libs = m_Makefile->GetLinkLibraries();
  for(j = libs.begin(); j != libs.end(); ++j)
    {
    std::string::size_type pos = (*j).find("-l");
    if((pos == std::string::npos || pos > 0)
       && (*j).find("${") == std::string::npos)
      {
      linkLibs += "-l";
      }
    linkLibs += *j;
    linkLibs += " ";
    }
  std::vector<std::string>& libsUnix = m_Makefile->GetLinkLibrariesUnix();
  for(j = libsUnix.begin(); j != libsUnix.end(); ++j)
    {
    linkLibs += *j;
    linkLibs += " ";
    }
  linkLibs += " ${LOCAL_LINK_FLAGS} ";
  // create and output a varible in the makefile that
  // each executable will depend on.  This will have all the
  // libraries that the executable uses
  fout << "CMAKE_DEPEND_LIBS = ";
  this->OutputDependLibraries(fout);
  // Now create rules for all of the executables to be built
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(unsigned int i = 0; i < Classes.size(); i++)
    {
    if(!Classes[i].m_AbstractClass && !Classes[i].m_HeaderFileOnly
       && Classes[i].m_IsExecutable)
      { 
      std::string DotO = Classes[i].m_ClassName;
      DotO += ".o";
      fout << Classes[i].m_ClassName << ": " << DotO << " ";
      fout << "${CMAKE_DEPEND_LIBS}\n";
      fout << "\t${CXX}  ${CXX_FLAGS}  " << m_Makefile->GetDefineFlags()
           << DotO.c_str() << " "
           << linkLibs.c_str() 
           << " -o $@ ""\n\n";
      }
    }
  // ouput the list of executables
  fout << "EXECUTABLES = \\\n";
  for(unsigned int i = 0; i < Classes.size(); i++)
    {
    if(!Classes[i].m_AbstractClass && !Classes[i].m_HeaderFileOnly
       && Classes[i].m_IsExecutable)
      { 
      fout << Classes[i].m_ClassName;
      if(i < Classes.size()-1)
        {
        fout << " \\";
        }
      fout << "\n";
      }
    }
  fout << "\n";
}

// fix up names of directories so they can be used
// as targets in makefiles.
inline std::string FixDirectoryName(const char* dir)
{
  std::string s = dir;
  // replace ../ with 3 under bars
  size_t pos = s.find("../");
  if(pos != std::string::npos)
    {
    s.replace(pos, 3, "___");
    }
  // replace / directory separators with a single under bar 
  pos = s.find("/");
  while(pos != std::string::npos)
    {
    s.replace(pos, 1, "_");
    pos = s.find("/");
    }
  return s;
}


// output rules for decending into sub directories
void cmUnixMakefileGenerator::OutputSubDirectoryRules(std::ostream& fout)
{
    // Output Sub directory build rules
  const std::vector<std::string>& SubDirectories
    = m_Makefile->GetSubDirectories();
    
  if( SubDirectories.size() == 0)
    {
    return;
    }
  fout << "SUBDIR_BUILD = \\\n";
  unsigned int i;
  for(i =0; i < SubDirectories.size(); i++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "build_" << subdir.c_str();
    if(i == SubDirectories.size()-1)
      {
      fout << " \n\n";
      }
    else
      {
      fout << " \\\n";
      }
    }
  fout << std::endl;
  fout << "SUBDIR_CLEAN = \\\n";
  for(i =0; i < SubDirectories.size(); i++)
    { 
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "clean_" << subdir.c_str();
    if(i == SubDirectories.size()-1)
      {
      fout << " \n\n";
      }
    else
      {
      fout << " \\\n";
      }
    }
  fout << std::endl;
  fout << "alldirs : ${SUBDIR_BUILD}\n\n";

  for(i =0; i < SubDirectories.size(); i++)
    {
    std::string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "build_" << subdir.c_str() << ":\n";
    fout << "\tcd " << SubDirectories[i].c_str()
         << "; ${MAKE} -${MAKEFLAGS} CMakeTargets.make\n";
    fout << "\tcd " << SubDirectories[i].c_str()
         << "; ${MAKE} -${MAKEFLAGS} all\n\n";

    fout << "clean_" << subdir.c_str() << ": \n";
    fout << "\tcd " << SubDirectories[i].c_str() 
         << "; ${MAKE} -${MAKEFLAGS} clean\n\n";
    }
}




// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend GenerateMakefile
void cmUnixMakefileGenerator::OutputDepends(std::ostream& fout)
{
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(unsigned int i = 0; i < Classes.size(); i++)
    {
    if(!Classes[i].m_AbstractClass && !Classes[i].m_HeaderFileOnly)
      {
      if( Classes[i].m_Depends.size())
	{
	fout << Classes[i].m_ClassName << ".o : \\\n";
	for(std::vector<std::string>::iterator j =  
	      Classes[i].m_Depends.begin();
	    j != Classes[i].m_Depends.end(); ++j)
	  {
	  if(j+1 == Classes[i].m_Depends.end())
	    {
	    fout << *j << " \n";
	    }
	  else
	    {
	    fout << *j << " \\\n";
	    }
	  }
	fout << "\n\n";
	}
      }
    }
}
