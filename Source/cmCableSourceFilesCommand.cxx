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
#include "cmCableSourceFilesCommand.h"
#include "cmCacheManager.h"

void cmCableSourceFilesCommand::FinalPass()
{
  // Get the index of the current package's cmClassFile.
  // If it doesn't exist, ignore this command.
  cmCablePackageCommand *cablePackage = m_CableData->GetCurrentPackage();
  std::string fileName = "Cxx/";
  fileName += cablePackage->GetPackageName();
  fileName += "_cxx";
  cmSourceFile *ci = m_Makefile->GetSource(cablePackage->GetPackageName(),
                                           fileName.c_str());
  
  if(ci == 0)
    { return; }
  
  // The package's file has not yet been generated yet.  The dependency
  // finder will need hints.  Add one for each source file.
  for(Entries::const_iterator f = m_Entries.begin();
      f != m_Entries.end(); ++f)
    {
    std::string header = *f+".h";
    ci->GetDepends().push_back(header);
    }
}


/**
 * Write the CABLE configuration code to indicate header dependencies for
 * a package.
 */
void cmCableSourceFilesCommand::WriteConfiguration() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  
  cmRegularExpression needCdataBlock("[&<>]");
  
  os << indent << "<Headers>" << std::endl;
  for(Entries::const_iterator f = m_Entries.begin();
      f != m_Entries.end(); ++f)
    {
    // Look for the normal include file.
    std::string header = *f+".h";
    if(this->SourceFileExists(header))
      {
      os << indent << "  <File name=\"" << header.c_str() << "\"/>"
         << std::endl;
      }
    else
      {
      cmSystemTools::Error("Unable to find source file ", header.c_str());
      }
    
    // Look for an instantiation file.
    std::string txx = *f+".txx";
    if(this->SourceFileExists(txx))
      {
      os << indent << "  <File name=\"" << txx.c_str()
         << "\" purpose=\"instantiate\"/>" << std::endl;
      }
    }
  os << indent << "</Headers>" << std::endl;
}


/**
 * Search the include path for the specified file.
 */
bool cmCableSourceFilesCommand::SourceFileExists(const std::string& name) const
{
  // We must locate the file in the include path so that we can detect
  // its extension, and whether there is more than one to find.
  std::string file = name;
  m_Makefile->ExpandVariablesInString(file);
      
  // See if the file just exists here.  The compiler's search path will
  // locate it.
  if(cmSystemTools::FileExists(file.c_str()))
    {
    return true;
    }
  
  // We must look for the file in the include search path.
  const std::vector<std::string>& includeDirectories =
    m_Makefile->GetIncludeDirectories();
  
  for(std::vector<std::string>::const_iterator dir = includeDirectories.begin();
      dir != includeDirectories.end(); ++dir)
    {
    std::string path = *dir + "/";
    m_Makefile->ExpandVariablesInString(path);
    if(cmSystemTools::FileExists((path+file).c_str()))
      {
      return true;
      }
    }

  // We couldn't locate the source file.
  return false;
}
