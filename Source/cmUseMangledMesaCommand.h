#ifndef cmUseMangledMesaCommand_h
#define cmUseMangledMesaCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmUseMangledMesaCommand
 * \brief Create Tcl Wrappers for VTK classes.
 *
 * cmUseMangledMesaCommand is used to define a CMake variable include
 * path location by specifying a file and list of directories.
 */
class cmUseMangledMesaCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmUseMangledMesaCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string>& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "USE_MANGLED_MESA";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create copies of mesa headers for use in combination with system gl.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "USE_MANGLED_MESA(\"path to mesa includes, should contain gl_mangle.h\""
      " \"directory for output\"  )";
    }

protected:
  void CopyAndFullPathMesaHeader(const char* source,
                                 const char* outdir);
private:
  std::vector<cmSourceFile> m_WrapClasses;
  std::vector<std::string> m_WrapHeaders;
  std::string m_LibraryName;
  std::string m_SourceList;
  std::vector<std::string> m_Commands;
};



#endif
