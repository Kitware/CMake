#ifndef cmFLTKWrapUICommand_h
#define cmFLTKWrapUICommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmFLTKWrapUICommand
 * \brief Create .h and .cxx files rules for FLTK user interfaces files
 *
 * cmFLTKWrapUICommand is used to create wrappers for FLTK classes into normal C++
 */
class cmFLTKWrapUICommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFLTKWrapUICommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "FLTK_WRAP_UI";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create FLTK user interfaces Wrappers.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "FLTK_WRAP_UI(resultingLibraryName SourceList)\n"
      "Produce .h and .cxx files for all the .fl and .fld file listed "
      "in the SourceList.\n"
      "The .h files will be added to the library using the base name in\n"
      "source list.\n"
      "The .cxx files will be added to the library using the base name in \n"
      "source list.";
    }
  
private:
  /**
   * List of produced files.
   */
  std::vector<cmSourceFile> m_GeneratedSourcesClasses;
  std::vector<cmSourceFile> m_GeneratedHeadersClasses;
  /**
   * List of Fluid files that provide the source 
   * generating .cxx and .h files
   */
  std::vector<std::string> m_WrapUserInterface;
  std::string m_GUISourceList;
  std::string m_GeneratedSourceList;
};



#endif
