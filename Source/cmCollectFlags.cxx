#include "cmCollectFlags.h"
#include "cmSystemTools.h"
#include <fstream>
#include <iterator>
#include <iostream>

cmCollectFlags::cmCollectFlags()
{
}

cmCollectFlags::~cmCollectFlags()
{
}

void cmCollectFlags::Print()
{
  std::ostream_iterator<std::string> out(std::cout, "\n");
  std::cout << "m_IncludeDirectories " << std::endl;
  std::copy(m_IncludeDirectories.begin(), m_IncludeDirectories.end(), out);
  std::cout << "m_linkdirectories " << std::endl;
  std::copy(m_LinkDirectories.begin(), m_LinkDirectories.end(), out);
  std::cout << "m_LinkLibraries " << std::endl;
  std::copy(m_LinkLibraries.begin(), m_LinkLibraries.end(), out);
  std::cout << "m_LinkLibrariesWin32 " << std::endl;
  std::copy(m_LinkLibrariesWin32.begin(), m_LinkLibrariesWin32.end(), out);
  std::cout << "m_LinkLibrariesUnix " << std::endl;
  std::copy(m_LinkLibrariesUnix.begin(), m_LinkLibrariesUnix.end(), out);
}

void cmCollectFlags::SetSourceHomeDirectory(const char* dir)
{
  m_SourceHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(m_SourceHomeDirectory);
}

void cmCollectFlags::SetStartDirectory(const char* dir)
{
  m_StartDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(m_StartDirectory);
}


void cmCollectFlags::ParseDirectories()
{
  this->ParseDirectory(m_StartDirectory.c_str());
}


void cmCollectFlags::ParseFile(const char* filename)
{
  std::ifstream fin(filename);
  if(!fin)
    {
    std::cerr << "error can not open file " << filename << std::endl;
    return;
    }
  char inbuffer[2048];
  while ( fin.getline(inbuffer, 2047 ) )
    {
    std::string line = inbuffer;
    if(line.find("INCLUDE_DIRECTORIES") != std::string::npos)
      {
      cmSystemTools::ReadList(m_IncludeDirectories, fin);
      }
    if(line.find("LINK_DIRECTORIES") != std::string::npos)
      {
      cmSystemTools::ReadList(m_LinkDirectories, fin);
      }
    if(line.find("LINK_LIBRARIES") != std::string::npos)
      {
      cmSystemTools::ReadList(m_LinkLibraries, fin);
      }
    if(line.find("WIN32_LIBRARIES") != std::string::npos)
      {
      cmSystemTools::ReadList(m_LinkLibrariesWin32, fin);
      }
    if(line.find("UNIX_LIBRARIES") != std::string::npos)
      {
      cmSystemTools::ReadList(m_LinkLibrariesUnix, fin);
      }
    }
}



// Go until directory == m_cmHomeDirectory 
// 1. fix slashes
// 2. peal off /dir until home found, go no higher
void cmCollectFlags::ParseDirectory(const char* dir)
{
  std::string listsFile = dir;
  listsFile += "/CMakeLists.txt";
  if(cmSystemTools::FileExists(listsFile.c_str()))
    {
    this->ParseFile(listsFile.c_str());
    }
  if(m_SourceHomeDirectory == dir)
    {
    return;
    }

  std::string dotdotDir = dir;
  std::string::size_type pos = dotdotDir.rfind('/');
  if(pos != std::string::npos)
    {
    dotdotDir = dotdotDir.substr(0, pos);
    this->ParseDirectory(dotdotDir.c_str());
    }
}
