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
  m_Indentation(0),
  m_Package(NULL),
  m_PackageNamespaceDepth(0),
  m_PackageClassIndex(-1),
  m_OutputFileName(configurationFile),
  m_OutputFile(m_OutputFileName.c_str())
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

