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
#ifndef cmCableData_h
#define cmCableData_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmCableCommand;
class cmCablePackageCommand;

/** \class cmCableData
 * \brief Hold data in one location for all cmCableCommand subclasses.
 */
class cmCableData
{
public:
  cmCableData(const cmCableCommand*, const std::string&);
  ~cmCableData();
  
  /**
   * Returns true if the given cmCableCommand is the owner of this
   * cmCableData.
   */
  bool OwnerIs(const cmCableCommand* owner) const
    { return (owner == m_Owner); }  

  std::ostream& GetOutputStream()
    { return m_OutputFile; }

  void InitializeOutputFile();
  void CloseOutputFile();
  
  void WriteConfigurationHeader();
  void WriteConfigurationFooter();
  
  /**
   * Class to simplify indentation printing.
   */
  class Indentation
  {
  public:
    Indentation(int indent): m_Indent(indent) {}
    void Print(std::ostream& os) const;
    Indentation Next() const { return Indentation(m_Indent+2); }
    Indentation Previous() const { return Indentation(m_Indent-2); }
  private:
    int m_Indent;
  };
  
  void Indent() { m_Indentation = m_Indentation.Next(); }
  void Unindent() { m_Indentation = m_Indentation.Previous(); }
  const Indentation& GetIndentation() const { return m_Indentation; }
  
  void OpenNamespace(const std::string&);
  void CloseNamespace(const std::string&);

  void BeginPackage(cmCablePackageCommand*);
  void EndPackage();
  
  cmCablePackageCommand *GetCurrentPackage() { return m_Package; }
  
private:
  /**
   * The cmCableCommand which created this instance of cmCableCommand.
   */
  const cmCableCommand* m_Owner;

  /**
   * The name of the output file opened as m_OutputFile.
   */
  std::string m_OutputFileName;  
  
  /**
   * The output file to which the configuration is written.
   */
  std::ofstream m_OutputFile;
  
  /**
   * Current indentation for output.
   */
  Indentation m_Indentation;
  
  /**
   * The stack of namespaces.
   */
  std::list<std::string>  m_NamespaceStack;
  
  /**
   * The command that created the package currently being defined.
   */
  cmCablePackageCommand*  m_Package;
  
  /**
   * The namespace level at which the current package was created.
   * This must be the level when the package is ended.
   */
  unsigned int m_PackageNamespaceDepth;
};

std::ostream& operator<<(std::ostream&, const cmCableData::Indentation&);

#endif
