#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmClassFile.h"
#include <sys/stat.h>
#include <iostream>


// Helper function to hide the use of system stat function
bool cmFileExists(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
  {
    return false;
  }
  else
  {
    return true;
  }
}

// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
void cmClassFile::SetName(const char* name, const char* dir)
{
  m_HeaderFileOnly = true;
  m_ClassName = name;
  std::string pathname = dir;
  if(pathname != "")
    {
    pathname += "/";
    }
  
  pathname += m_ClassName;
  std::string hname = pathname;
  hname += ".cxx";
  if(cmFileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".c";
  if(cmFileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".txx";
  if(cmFileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
  }
  std::cerr << "file seems to be a header only " << hname << " " << m_ClassName.c_str() << std::endl;
}


void cmClassFile::Print()
{
  if(m_AbstractClass)
    std::cout <<  "Abstract ";
  else
    std::cout << "Concrete ";
  if(m_HeaderFileOnly)
    std::cout << "Header file ";
  else
    std::cout << "CXX file ";
  std::cout << m_ClassName << std::endl;
}
