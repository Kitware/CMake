/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmSourceFile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"



// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
void cmSourceFile::SetName(const char* name, const char* dir)
{
  m_HeaderFileOnly = true;

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
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
    }
  
  // Try various extentions
  hname = pathname;
  hname += ".cxx";
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    m_SourceExtension = "cxx";
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
    }
  
  hname = pathname;
  hname += ".c";
  if(cmSystemTools::FileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_SourceExtension = "c";
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".txx";
  if(cmSystemTools::FileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_SourceExtension = "txx";
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".h";
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    m_SourceExtension = "h";
    m_FullPath = hname;
    return;
    }
  
  cmSystemTools::Error("can not find file ", hname.c_str());
  cmSystemTools::Error("Tried .txx .cxx .c for ", hname.c_str());
}

void cmSourceFile::SetName(const char* name, const char* dir, const char *ext,
                          bool hfo)
{
  m_HeaderFileOnly = hfo;
  m_SourceName = name;
  std::string pathname = dir;
  if(pathname != "")
    {
    pathname += "/";
    }
  
  pathname += m_SourceName + "." + ext;
  m_FullPath = pathname;
  m_SourceExtension = ext;
  return;
}

void cmSourceFile::Print() const
{
  if(m_AbstractClass)
    {
    std::cout <<  "Abstract ";
    }
  else
    {
    std::cout << "Concrete ";
    }
  if(m_HeaderFileOnly)
    {
    std::cout << "Header file ";
    }
  else
    {
    std::cout << "CXX file ";
    }
  std::cout << m_SourceName << std::endl;
}
