#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmMakefile.h"
#include "cmClassFile.h"
#include "cmDirectory.h"
#include "cmSystemTools.h"
#include <fstream>
#include <iostream>


// default is not to be building executables
cmMakefile::cmMakefile()
{
  m_Executables = false;
}


// call print on all the classes in the makefile
void cmMakefile::Print()
{
  for(int i = 0; i < m_Classes.size(); i++)
    m_Classes[i].Print();
}

// Parse the given CMakeLists.txt file into a list of classes.
bool cmMakefile::ReadMakefile(const char* filename)
{
  m_BuildFlags.SetSourceHomeDirectory(this->GetHomeDirectory());
  m_BuildFlags.SetStartDirectory(this->GetCurrentDirectory());
  m_BuildFlags.ParseDirectories();
  m_BuildFlags.ExpandVaribles(this);
 
  std::ifstream fin(filename);
  if(!fin)
    {
    std::cerr << "error can not open file " << filename << std::endl;
    return false;
    }
  char inbuffer[2048];
  while ( fin.getline(inbuffer, 2047 ) )
    {
    std::string line = inbuffer;
    cmClassFile file;
    std::string::size_type pos = line.find("SOURCE_FILES");
    if((pos != std::string::npos) && (pos == 0 ) )
      {
      if(line.find("\\") != std::string::npos)
	{
	this->ReadClasses(fin, false);
	}
      }
#ifdef _WIN32
    else if(line.find("WIN32_SOURCE_FILES") != std::string::npos)
      {
      if(line.find("\\") != std::string::npos)
	{
	this->ReadClasses(fin, false);
	}
      }
#else
    else if(line.find("UNIX_SOURCE_FILES") != std::string::npos)
      {
      if(line.find("\\") != std::string::npos)
	{
	this->ReadClasses(fin, false);
	}
      }
#endif
    else if(line.find("ABSTRACT_CLASSES") != std::string::npos)
      {
      if(line.find("\\") != std::string::npos)
	{
	this->ReadClasses(fin, true);
	}
      }
    else if(line.find("AUX_SOURCE_DIRECTORY") != std::string::npos)
      {
      this->ReadTemplateInstanceDirectory(line);
      }
    else if(line.find("SUBDIRS") != std::string::npos)
      {
      if(line.find("\\") != std::string::npos)
	{
        cmSystemTools::ReadList(m_SubDirectories, fin);
	}
      }
    else if(line.find("EXECUTABLES") != std::string::npos)
      {
      if(line.find("\\") != std::string::npos)
	{
	this->ReadClasses(fin, false);
	m_Executables = true;
	}
      }
    else if(line.find("BEGIN MAKE VERBATIM") != std::string::npos)
      {
      char inbuffer[2048];
      bool done = false;
      m_MakeVerbatim.push_back("# Begin CMakeLists Verbatim\n");
      while(!done)
        {
        fin.getline(inbuffer, 2047);
        m_MakeVerbatim.push_back(inbuffer);
        if((m_MakeVerbatim.end()-1)->find("END MAKE VERBATIM") 
           != std::string::npos )
          {
          done = true;
          *(m_MakeVerbatim.end()-1) = "# End CMakeLists VERBATIM\n\n";
          }
        }
      }
    else if(line.find("LIBRARY") != std::string::npos)
      {
      std::string libname = cmSystemTools::ExtractVariable("LIBRARY", 
                                                           line.c_str());
      this->SetLibraryName(libname.c_str());
      }
    else if(line.find("PROJECT") != std::string::npos)
      {
      std::string libname = cmSystemTools::ExtractVariable("PROJECT", 
                                                           line.c_str());
      this->SetProjectName(libname.c_str());
      }
    }
  return true;
}
  

// Read a list from the Makefile stream
void cmMakefile::ReadClasses(std::ifstream& fin,
			      bool abstract)
{
  char inbuffer[2048];
  bool done = false;
  while (!done)
    {  
    // read a line from the makefile
    fin.getline(inbuffer, 2047); 
    // convert to a string class
    std::string classname = inbuffer;
    // if the line does not end in \ then we are at the
    // end of the list
    if(classname.find('\\') == std::string::npos)
      {
      done = true;
      }
    // remove extra spaces and \ from the class name
    classname = cmSystemTools::CleanUpName(classname.c_str());
    
    // if this is not an abstract list then add new class
    // to the list of classes in this makefile
    if(!abstract)
      {
      cmClassFile file;
      file.SetName(classname.c_str(), this->GetCurrentDirectory());
      file.m_AbstractClass = false;
      m_Classes.push_back(file);
      }
    else
      {
      // if this is an abstract list, then look
      // for an existing class and set it to abstract
      for(int i = 0; i < m_Classes.size(); i++)
	{
	if(m_Classes[i].m_ClassName == classname)
	  {
	  m_Classes[i].m_AbstractClass = true;
	  break;
	  }
	}
      }
    }
}

// Find all of the files in dir as specified from this line:
// AUX_SOURCE_DIRECTORY = dir
// Add all the files to the m_Classes array.

void cmMakefile::ReadTemplateInstanceDirectory(std::string& line)
{
  std::string::size_type start = line.find("=");
  if(start != std::string::npos)
    {
    std::string templateDirectory = line.substr(start+1, line.size());
    templateDirectory = cmSystemTools::CleanUpName(templateDirectory.c_str());
    m_TemplateDirectories.push_back(templateDirectory);
    std::string tdir = this->GetCurrentDirectory();
    tdir += "/";
    tdir += templateDirectory;
    // Load all the files in the directory
    cmDirectory dir;
    if(dir.Load(tdir.c_str()))
      {
      int numfiles = dir.GetNumberOfFiles();
      for(int i =0; i < numfiles; ++i)
        {
        std::string file = dir.GetFile(i);
        // ignore files less than f.cxx in length
        if(file.size() > 4)
          {
          // Remove the extension
          std::string::size_type dotpos = file.rfind(".");
          file = file.substr(0, dotpos);
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so 
          // depends can be done
          cmClassFile cmfile;
          cmfile.SetName(fullname.c_str(), this->GetCurrentDirectory());
          cmfile.m_AbstractClass = false;
          m_Classes.push_back(cmfile);
          }
        }
      }
    else
      {
      std::cerr << "Warning can not open template instance directory "
                << templateDirectory.c_str() << std::endl;
      }
    }
}


