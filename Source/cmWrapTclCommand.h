#ifndef cmWrapTclCommand_h
#define cmWrapTclCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmWrapTclCommand
 * \brief Define a command that searches for an include file.
 *
 * cmWrapTclCommand is used to define a CMake variable include
 * path location by specifying a file and list of directories.
 */
class cmWrapTclCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWrapTclCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "WRAP_TCL";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create Tcl Wrappers.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "WRAP_TCL(resultingLibraryName SourceListName SourceLists ...)";
    }

  /**
   * Helper methods
   */
  virtual bool CreateInitFile(std::string &name);
  virtual bool WriteInit(const char *kitName, std::string& outFileName,
                         std::vector<std::string>& classes);
  
private:
  std::vector<cmClassFile> m_WrapClasses;
  std::vector<std::string> m_WrapHeaders;
  std::string m_LibraryName;
  std::string m_SourceList;
};



#endif
