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
#ifndef cmCabilData_h
#define cmCabilData_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmCabilCommand;

/** \class cmCabilData
 * \brief Hold data in one location for all cmCabilCommand subclasses.
 */
class cmCabilData
{
public:
  /**
   * The cmCabilData instance is owned by one cmCabilCommand, which is given
   * to this constructor.
   */
  cmCabilData(const cmCabilCommand* owner): m_Owner(owner) {}
  
  ~cmCabilData();
  
  /**
   * Returns true if the given cmCabilCommand is the owner of this
   * cmCabilData.
   */
  bool OwnerIs(const cmCabilCommand* owner) const
    { return (owner == m_Owner); }  
  
  /**
   * Hold an output stream for all commands that use it.  Maintain the
   * first and last commands that reference it so that they can write the
   * header/footer lines, if necessary.
   */
  class OutputFile
  {
  public:
    OutputFile(std::string, const cmCabilCommand*);
    ~OutputFile();
    std::ostream& GetStream();
    void SetLastReferencingCommand(const cmCabilCommand*);
    bool FirstReferencingCommandIs(const cmCabilCommand*) const;
    bool LastReferencingCommandIs(const cmCabilCommand*) const;
  private:
    std::ofstream m_FileStream;
    const cmCabilCommand* m_FirstReferencingCommand;
    const cmCabilCommand* m_LastReferencingCommand;
  };
  
  OutputFile* GetOutputFile(const std::string&, const cmCabilCommand*);
  
private:
  typedef std::map<std::string, OutputFile*>  OutputFiles;
  
  /**
   * The cmCabilCommand which created this instance of cmCabilCommand.
   */
  const cmCabilCommand* m_Owner;
  
  /**
   * Hold all output streams by file name.
   */
  OutputFiles m_OutputFiles;  
};

#endif
