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
#ifndef cmCommand_h
#define cmCommand_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmCommand
 * \brief Superclass for all commands in CMake.
 *
 * cmCommand is the base class for all commands in CMake. A command
 * manifests as an entry in CMakeLists.txt and produces one or
 * more makefile rules. Commands are associated with a particular
 * makefile. This base class cmCommand defines the API for commands 
 * to support such features as enable/disable, inheritance, 
 * documentation, and construction.
 */
class cmCommand
{
public:
  /**
   * Construct the command. By default it is enabled with no makefile.
   */
  cmCommand()  
    {m_Makefile = 0; m_Enabled = true;}

  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmCommand() {}
  
  /**
   * Specify the makefile.
   */
  void SetMakefile(cmMakefile*m) 
    {m_Makefile = m; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args) = 0;

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass() {};
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() = 0;
  
  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {
    return false;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() = 0;

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() = 0;

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() = 0;

  /**
   * Enable the command.
   */
  void EnabledOn() 
    {m_Enabled = true;}

  /**
   * Disable the command.
   */
  void EnabledOff() 
    {m_Enabled = false;}

  /**
   * Query whether the command is enabled.
   */
  bool GetEnabled()  
    {return m_Enabled;}

  /**
   * Disable or enable the command.
   */
  void SetEnabled(bool enabled)  
    {m_Enabled = enabled;}

  /**
   * Return the last error string.
   */
  const char* GetError() 
    {
      if(m_Error.length() == 0)
        {
        m_Error = this->GetName();
        m_Error += " unknown error.";
        }
      return m_Error.c_str();
    }

  /**
   * Returns true if this class is the given class, or a subclass of it.
   */
  static bool IsTypeOf(const char *type)
    { return !strcmp("cmCommand", type); }
  
  /**
   * Returns true if this object is an instance of the given class or
   * a subclass of it.
   */
  virtual bool IsA(const char *type)
    { return cmCommand::IsTypeOf(type); }

protected:
  void SetError(const char* e)
    {
    m_Error = this->GetName();
    m_Error += " ";
    m_Error += e;
    }
  cmMakefile* m_Makefile;

private:
  bool m_Enabled;
  std::string m_Error;
};

// All subclasses of cmCommand should invoke this macro.
#define cmTypeMacro(thisClass,superclass) \
static bool IsTypeOf(const char *type) \
{ \
  if ( !strcmp(#thisClass,type) ) \
    { \
    return true; \
    } \
  return superclass::IsTypeOf(type); \
} \
virtual bool IsA(const char *type) \
{ \
  return thisClass::IsTypeOf(type); \
} \
static thisClass* SafeDownCast(cmCommand *c) \
{ \
  if ( c && c->IsA(#thisClass) ) \
    { \
    return (thisClass *)c; \
    } \
  return 0;\
}


#endif
