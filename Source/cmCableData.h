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
