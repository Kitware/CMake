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
#include "cmSourceFile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"



// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
void cmSourceFile::SetName(const char* name, const char* dir,
                           const std::vector<std::string>& sourceExts,
                           const std::vector<std::string>& headerExts)
{
  this->SetProperty("HEADER_FILE_ONLY","1");

  m_SourceName = name;
  std::string pathname = dir;

  // the name might include the full path already, so
  // check for this case
  if (name &&  (name[0] == '/' || 
		(name[0] != '\0' && name[1] == ':')))
    {
    pathname = "";
    }
  if(pathname != "")
    {
    pathname += "/";
    }


  // First try and see whether the listed file can be found
  // as is without extensions added on.
  pathname += m_SourceName;
  std::string hname = pathname;
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    std::string::size_type pos = hname.rfind('.');
    if(pos != std::string::npos)
      {
      m_SourceExtension = hname.substr(pos+1, hname.size()-pos);
      std::string::size_type pos2 = hname.rfind('/');
      if(pos2 != std::string::npos)
	{
	  m_SourceName = hname.substr(pos2+1, pos - pos2-1);
	}
      else
	{
	  m_SourceName = hname.substr(0, pos);
	}
      }

    // See if the file is a header file
    if(std::find( headerExts.begin(), headerExts.end(), m_SourceExtension ) ==
       headerExts.end())
      {
      this->SetProperty("HEADER_FILE_ONLY","0");
      }
    m_FullPath = hname;
    return;
    }
  
  // Next, try the various source extensions
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      m_SourceExtension = *ext;
      this->SetProperty("HEADER_FILE_ONLY","0");
      m_FullPath = hname;
      return;
      }
    }

  // Finally, try the various header extensions
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      m_SourceExtension = *ext;
      m_FullPath = hname;
      return;
      }
    }

  std::string errorMsg = "\n\nTried";
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    errorMsg += " .";
    errorMsg += *ext;
    }
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    errorMsg += " .";
    errorMsg += *ext;
    }
  cmSystemTools::Error("can not find file ", pathname.c_str(), 
                       errorMsg.c_str());
}


void cmSourceFile::SetName(const char* name, const char* dir, const char *ext,
                           bool hfo)
{
  this->SetProperty("HEADER_FILE_ONLY",(hfo ? "1" : "0"));
  m_SourceName = name;
  std::string pathname = dir;
  if(pathname != "")
    {
    pathname += "/";
    }
  
  pathname += m_SourceName;
  if(ext && strlen(ext))
    {
    pathname += ".";
    pathname += ext;
    }
  m_FullPath = pathname;
  m_SourceExtension = ext;
  return;
}

void cmSourceFile::Print() const
{
  std::cerr << "m_FullPath: " <<  m_FullPath << "\n";
  std::cerr << "m_SourceName: " << m_SourceName << std::endl;
  std::cerr << "m_SourceExtension: " << m_SourceExtension << "\n";
}

void cmSourceFile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  m_Properties[prop] = value;
}

const char *cmSourceFile::GetProperty(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmSourceFile::GetPropertyAsBool(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return cmSystemTools::IsOn(i->second.c_str());
    }
  return false;
}
