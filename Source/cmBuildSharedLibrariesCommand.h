#ifndef cmBuildSharedLibrariesCommand_h
#define cmBuildSharedLibrariesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmBuildSharedLibrariesCommand
 * \brief Define a command that searches for an include file.
 *
 * cmBuildSharedLibrariesCommand is used to define a CMake variable include
 * path location by specifying a file and list of directories.
 */
class cmBuildSharedLibrariesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmBuildSharedLibrariesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);
  
  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "BUILD_SHARED_LIBRARIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Build shared libraries instead of static";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "BUILD_SHARED_LIBRARIES()";
    }
  
private:
};



#endif
