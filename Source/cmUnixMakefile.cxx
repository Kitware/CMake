#include "cmUnixMakefile.h"
#include "cmSystemTools.h"
#include <fstream>
#include <iostream>

// Output the depend information for all the classes 
// in the makefile.  These would have been generated
// by the class cmMakeDepend in the main of CMakeBuildTargets.
void cmUnixMakefile::OutputDepends(std::ostream& fout)
{
  for(int i = 0; i < m_Classes.size(); i++)
    {
    if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
      {
      if( m_Classes[i].m_Depends.size())
	{
	fout << m_Classes[i].m_ClassName << ".o : \\\n";
	for(std::vector<std::string>::iterator j =  
	      m_Classes[i].m_Depends.begin();
	    j != m_Classes[i].m_Depends.end(); ++j)
	  {
	  if(j+1 == m_Classes[i].m_Depends.end())
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

// This is where CMakeTargets.make is generated
// This function ouputs the following:
// 1. Include flags for the compiler
// 2. List of .o files that need to be compiled
// 3. Rules to build executables including -l and -L options
// 4. Rules to build in sub directories
// 5. The name of the library being built, if it is a library

void cmUnixMakefile::OutputMakefile(const char* file)
{
  if( m_TemplateDirectories.size() )
    {
    // For the case when this is running as a remote build
    // on unix, make the directory
    
    for(std::vector<std::string>::iterator i = m_TemplateDirectories.begin();
        i != m_TemplateDirectories.end(); ++i)
      {
      cmSystemTools::MakeDirectory(i->c_str());
      }
    }
  
  std::ofstream fout(file);
  if(!fout)
    {
    std::cerr  << "Error can not open " << file << " for write" << std::endl;
    return;
    }
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_BuildFlags.GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    fout << "-I" << i->c_str() << " ";
    }
  fout << " ${LOCAL_INCLUDE_FLAGS} ";
  fout << "\n";
  // see if there are files to compile in this makefile
  // These are used for both libraries and executables
  if(m_Classes.size() )
    {
    // Ouput Library name if there are SRC_OBJS
    if(strlen(this->GetLibraryName()) > 0)
      {
      fout << "ME = " <<  this->GetLibraryName() << "\n\n";
      fout << "BUILD_LIB_FILE = lib${ME}${ITK_LIB_EXT}\n\n";
      }
    // Output SRC_OBJ list for all the classes to be compiled
    fout << "SRC_OBJ = \\\n";
    for(int i = 0; i < m_Classes.size(); i++)
      {
      if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
	{
	fout << m_Classes[i].m_ClassName << ".o ";
	if(i ==  m_Classes.size() -1)
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
  // Ouput user make text embeded in the input file
  for(int i =0; i < m_MakeVerbatim.size(); i++)
    {
    fout << m_MakeVerbatim[i] << "\n";
    }
  fout << "\n\n";

  // Output rules for building executables  
  if( m_Executables )
    {
    // collect all the flags needed for linking libraries
    std::string linkLibs;        
    std::vector<std::string>::iterator j;
    std::vector<std::string>& libdirs = m_BuildFlags.GetLinkDirectories();
    for(j = libdirs.begin(); j != libdirs.end(); ++j)
      {
      linkLibs += "-L";
      linkLibs += *j;
      linkLibs += " ";
      }
    std::vector<std::string>& libs = m_BuildFlags.GetLinkLibraries();
    for(j = libs.begin(); j != libs.end(); ++j)
      {
      linkLibs += "-l";
      linkLibs += *j;
      linkLibs += " ";
      }
    std::vector<std::string>& libsUnix = m_BuildFlags.GetLinkLibrariesUnix();
    for(j = libsUnix.begin(); j != libsUnix.end(); ++j)
      {
      linkLibs += *j;
      linkLibs += " ";
      }
    linkLibs += " ${LOCAL_LINK_FLAGS} ";
    // Now create rules for all of the executables to be built
    for(int i = 0; i < m_Classes.size(); i++)
      {
      if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
	{ 
        std::string DotO = m_Classes[i].m_ClassName;
        DotO += ".o";
        fout << m_Classes[i].m_ClassName << ": " << DotO << "\n";
	fout << "\t${CXX}  ${CXX_FLAGS}  " 
	     << DotO.c_str() << " "
             << linkLibs.c_str() 
	     << " -o $@ ""\n\n";
	}
      }
    // ouput the list of executables
    fout << "EXECUTABLES = \\\n";
    for(int i = 0; i < m_Classes.size(); i++)
      {
      if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
	{ 
        fout << m_Classes[i].m_ClassName;
	if(i < m_Classes.size()-1)
	  {
	    fout << " \\";
	  }
	fout << "\n";
	}
      }
    fout << "\n";
    }
  // Output Sub directory build rules
  if( m_SubDirectories.size() )
    {
    fout << "SUBDIR_BUILD = \\\n";
    int i;
    for(i =0; i < m_SubDirectories.size(); i++)
      { 
      std::string subdir = FixDirectoryName(m_SubDirectories[i].c_str());
      fout << "build_" << subdir.c_str();
      if(i == m_SubDirectories.size()-1)
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
    for(i =0; i < m_SubDirectories.size(); i++)
      { 
      std::string subdir = FixDirectoryName(m_SubDirectories[i].c_str());
      fout << "clean_" << subdir.c_str();
      if(i == m_SubDirectories.size()-1)
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

    for(i =0; i < m_SubDirectories.size(); i++)
      {
      std::string subdir = FixDirectoryName(m_SubDirectories[i].c_str());
      fout << "build_" << subdir.c_str() << ":\n";
      fout << "\tcd " << m_SubDirectories[i].c_str()
	   << "; ${MAKE} -${MAKEFLAGS} CMakeTargets.make\n";
      fout << "\tcd " << m_SubDirectories[i].c_str()
	   << "; ${MAKE} -${MAKEFLAGS} all\n\n";

      fout << "clean_" << subdir.c_str() << ": \n";
      fout << "\tcd " << m_SubDirectories[i].c_str() 
	   << "; ${MAKE} -${MAKEFLAGS} clean\n\n";
      }
    }
  this->OutputDepends(fout);
}
