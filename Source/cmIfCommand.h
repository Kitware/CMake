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
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmIfFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  virtual ~cmIfFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                                 const cmMakefile &mf) const;
  virtual bool ShouldRemove(const char *name, const std::vector<std::string> &args, 
                            const cmMakefile &mf) const;
  std::string m_Define;
};

/** \class cmIfCommand
 * \brief starts an if block
 *
 * cmIfCommand starts an if block
 */
class cmIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIfCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "IF";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "start an if block";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "IF(define)";
    }
  
  cmTypeMacro(cmIfCommand, cmCommand);
};


#endif
