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
#ifndef cmCommand_h
#define cmCommand_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

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
  virtual bool InitialPass(std::vector<std::string>& args) = 0;

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
        m_Error += " uknown error.";
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
