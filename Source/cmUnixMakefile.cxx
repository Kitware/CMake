#include "cmUnixMakefile.h"
#include <fstream>
#include <iostream>

// Output the depend information for all the classes 
// in the makefile.
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

// output the makefile to the named file
void cmUnixMakefile::OutputMakefile(const char* file)
{
  std::ofstream fout(file);
  if(!fout)
    {
    std::cerr  << "Error can not open " << file << " for write" << std::endl;
    return;
    }
  if(m_Classes.size() )
    {
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
  if( m_Executables )
    {
    for(int i = 0; i < m_Classes.size(); i++)
      {
      if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
	{ 
        std::string DotO = m_Classes[i].m_ClassName;
        DotO += ".o";
        fout << m_Classes[i].m_ClassName << ": " << DotO << "\n";
	fout << "\t ${CXX}  ${CXX_FLAGS}  " << DotO.c_str() << " -o $@ -L${ITK_OBJ}/Code/Common -lITKCommon \\\n"
	     << "\t-L${ITK_OBJ}/Code/Insight3DParty/vxl -lITKNumerics -lm ${DL_LIBS}\n\n";
	}
      }
    }
  
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
      fout << "build_" << subdir.c_str() << ": targets.make\n";
      fout << "\tcd " << m_SubDirectories[i].c_str() 
	   << "; ${MAKE} -${MAKEFLAGS} targets.make\n";
      fout << "\tcd " << m_SubDirectories[i].c_str()
	   << "; ${MAKE} -${MAKEFLAGS} all\n\n";

      fout << "clean_" << subdir.c_str() << ": \n";
      fout << "\tcd " << m_SubDirectories[i].c_str() 
	   << "; ${MAKE} -${MAKEFLAGS} clean\n\n";
      }
    }
  this->OutputDepends(fout);
}
