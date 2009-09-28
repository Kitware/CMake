/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDefinesPropertyCommand_h
#define cmDefinesPropertyCommand_h

#include "cmCommand.h"

class cmDefinePropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmDefinePropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "define_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define and document custom properties.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  define_property(<GLOBAL | DIRECTORY | TARGET | SOURCE |\n"
        "                   TEST | VARIABLE | CACHED_VARIABLE>\n"
        "                   PROPERTY <name> [INHERITED]\n"
        "                   BRIEF_DOCS <brief-doc> [docs...]\n"
        "                   FULL_DOCS <full-doc> [docs...])\n"
        "Define one property in a scope for use with the "
        "set_property and get_property commands.  "
        "This is primarily useful to associate documentation with property "
        "names that may be retrieved with the get_property command.  "
        "The first argument determines the kind of scope in which the "
        "property should be used.  It must be one of the following:\n"
        "  GLOBAL    = associated with the global namespace\n"
        "  DIRECTORY = associated with one directory\n"
        "  TARGET    = associated with one target\n"
        "  SOURCE    = associated with one source file\n"
        "  TEST      = associated with a test named with add_test\n"
        "  VARIABLE  = documents a CMake language variable\n"
        "  CACHED_VARIABLE = documents a CMake cache variable\n"
        "Note that unlike set_property and get_property no actual scope "
        "needs to be given; only the kind of scope is important.\n"
        "The required PROPERTY option is immediately followed by the name "
        "of the property being defined.\n"
        "If the INHERITED option then the get_property command will chain "
        "up to the next higher scope when the requested property is not "
        "set in the scope given to the command.  "
        "DIRECTORY scope chains to GLOBAL.  "
        "TARGET, SOURCE, and TEST chain to DIRECTORY.\n"
        "The BRIEF_DOCS and FULL_DOCS options are followed by strings to be "
        "associated with the property as its brief and full documentation.  "
        "Corresponding options to the get_property command will retrieve the "
        "documentation.";
    }
  
  cmTypeMacro(cmDefinePropertyCommand, cmCommand);
private:
  std::string PropertyName;
  std::string BriefDocs;
  std::string FullDocs;
};



#endif
