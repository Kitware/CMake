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
#include "cmCableData.h"
#include "cmCacheManager.h"

#include "cmCablePackageCommand.h"

/**
 * The cmCableData instance is owned by one cmCableCommand, which is given
 * to this constructor.
 */
cmCableData::cmCableData(const cmCableCommand* owner,
                         const std::string& configurationFile):
  m_Owner(owner),
  m_OutputFileName(configurationFile),
  m_OutputFile(configurationFile.c_str()),
  m_Indentation(0),
  m_Package(NULL),
  m_PackageNamespaceDepth(0)
{
  this->InitializeOutputFile();
}


/**
 * Free all data that was stored here.  Also close the output file.
 */
cmCableData::~cmCableData()
{
  // End last package, if any.
  this->EndPackage();
  
  // Finish up the output file.
  this->CloseOutputFile();
}


/**
 * Write the configuration header to the output file.
 */
void cmCableData::InitializeOutputFile()
{
  if(m_OutputFile)
    {
    this->WriteConfigurationHeader();
    }
  else
    {
    cmSystemTools::Error("Unable to open CABLE config file: ",
                         m_OutputFileName.c_str());
    }
}


/**
 * Close the configuration output file.  This writes the configuration
 * footer.
 */
void cmCableData::CloseOutputFile()
{
  if(m_OutputFile)
    {
    this->WriteConfigurationFooter();
    m_OutputFile.close();
    }
}


/**
 * Write a CABLE configuration file header.
 */
void cmCableData::WriteConfigurationHeader()
{
  m_OutputFile << m_Indentation << "<?xml version=\"1.0\"?>" << std::endl
               << m_Indentation << "<CableConfiguration>" << std::endl;  
  this->Indent();
}


/**
 * Write a CABLE configuration file footer.
 */
void cmCableData::WriteConfigurationFooter()
{
  this->Unindent();
  m_OutputFile << m_Indentation << "</CableConfiguration>" << std::endl;
}


/**
 * Print indentation spaces.
 */
void
cmCableData::Indentation
::Print(std::ostream& os) const
{
  if(m_Indent <= 0)
    { return; }
  
  // Use blocks of 8 spaces to speed up big indents.
  unsigned int blockCount = m_Indent >> 3;
  unsigned int singleCount = m_Indent & 7;
  while(blockCount-- > 0)
    {
    os << "        ";
    }
  while(singleCount-- > 0)
    {
    os << " ";
    }
}


/**
 * Open a namespace with the given name.
 */
void cmCableData::OpenNamespace(const std::string& name)
{
  m_NamespaceStack.push_back(name);
}


/**
 * Close the current namespace, checking whether it has the given name.
 */
void cmCableData::CloseNamespace(const std::string& name)
{
  if(m_NamespaceStack.empty())
    {
    cmSystemTools::Error("Unbalanced close-namespace = ", name.c_str());
    return;
    }
  if(m_NamespaceStack.back() != name)
    {
    cmSystemTools::Error("Wrong name on close-namespace = ", name.c_str());
    }
  
  // If this closes the namespace where the current package was opened,
  // the package must end as well.
  if(m_Package && (m_PackageNamespaceDepth == m_NamespaceStack.size()))
    {
    this->EndPackage();
    }
  
  m_NamespaceStack.pop_back();
}


/**
 * Begin a new package definition.  If there is a current one, it
 * will be ended.
 */
void cmCableData::BeginPackage(cmCablePackageCommand* command)
{
  // Close the current package, if any.
  this->EndPackage();
  
  // Open this package.
  m_Package = command;
  
  // Write out the package's header.
  m_Package->WritePackageHeader();
  
  // Save the package's opening namespace depth for later verification
  // on the end of the package.
  m_PackageNamespaceDepth = m_NamespaceStack.size();
}


/**
 * End a package definition.
 */
void cmCableData::EndPackage()
{
  // Make sure we have an open package.
  if(!m_Package)
    {
    return;
    }
  
  // Make sure the namespace nesting depth matches the opening depth
  // of the package.
  if(m_PackageNamespaceDepth != m_NamespaceStack.size())
    {
    cmSystemTools::Error("Package ended at different namespace depth than"
                         "it was created!", "");
    }
  // Write out the package's footer.
  m_Package->WritePackageFooter();
  
  // Done with the package.
  m_Package = NULL;
}


/**
 * Simplify indentation printing by allowing Indentation objects to be added
 * to streams.
 */
std::ostream& operator<<(std::ostream& os,
                         const cmCableData::Indentation& indent)
{  
  indent.Print(os);
  return os;
}

