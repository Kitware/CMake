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

/** \class cmCableData
 * \brief Hold data in one location for all cmCableCommand subclasses.
 */
class cmCableData
{
public:
  /**
   * The cmCableData instance is owned by one cmCableCommand, which is given
   * to this constructor.
   */
  cmCableData(const cmCableCommand* owner): m_Owner(owner) {}
  
  ~cmCableData();
  
  /**
   * Returns true if the given cmCableCommand is the owner of this
   * cmCableData.
   */
  bool OwnerIs(const cmCableCommand* owner) const
    { return (owner == m_Owner); }  
  
  /**
   * Hold an output stream for all commands that use it.  Maintain the
   * first and last commands that reference it so that they can write the
   * header/footer lines, if necessary.
   */
  class OutputFile
  {
  public:
    OutputFile(std::string, const cmCableCommand*);
    ~OutputFile();
    std::ostream& GetStream();
    void SetLastReferencingCommand(const cmCableCommand*);
    bool FirstReferencingCommandIs(const cmCableCommand*) const;
    bool LastReferencingCommandIs(const cmCableCommand*) const;
  private:
    std::ofstream m_FileStream;
    const cmCableCommand* m_FirstReferencingCommand;
    const cmCableCommand* m_LastReferencingCommand;
  };
  
  OutputFile* GetOutputFile(const std::string&, const cmCableCommand*);
  
private:
  typedef std::map<std::string, OutputFile*>  OutputFiles;
  
  /**
   * The cmCableCommand which created this instance of cmCableCommand.
   */
  const cmCableCommand* m_Owner;
  
  /**
   * Hold all output streams by file name.
   */
  OutputFiles m_OutputFiles;  
};

#endif
