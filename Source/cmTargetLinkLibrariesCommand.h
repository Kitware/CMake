/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetLinkLibrariesCommand_h
#define cmTargetLinkLibrariesCommand_h

#include "cmCommand.h"
#include "cmDocumentGeneratorExpressions.h"

/** \class cmTargetLinkLibrariesCommand
 * \brief Specify a list of libraries to link into executables.
 *
 * cmTargetLinkLibrariesCommand is used to specify a list of libraries to link
 * into executable(s) or shared objects. The names of the libraries
 * should be those defined by the LIBRARY(library) command(s).
 */
class cmTargetLinkLibrariesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetLinkLibrariesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "target_link_libraries";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return
      "Link a target to given libraries.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  target_link_libraries(<target> [item1 [item2 [...]]]\n"
      "                        [[debug|optimized|general] <item>] ...)\n"
      "Specify libraries or flags to use when linking a given target.  "
      "The named <target> must have been created in the current directory "
      "by a command such as add_executable or add_library.  "
      "The remaining arguments specify library names or flags.  "
      "Repeated calls for the same <target> append items in the order called."
      "\n"
      "If a library name matches that of another target in the project "
      "a dependency will automatically be added in the build system to make "
      "sure the library being linked is up-to-date before the target links.  "
      "Item names starting with '-', but not '-l' or '-framework', are "
      "treated as linker flags."
      "\n"
      "A \"debug\", \"optimized\", or \"general\" keyword indicates that "
      "the library immediately following it is to be used only for the "
      "corresponding build configuration.  "
      "The \"debug\" keyword corresponds to the Debug configuration "
      "(or to configurations named in the DEBUG_CONFIGURATIONS global "
      "property if it is set).  "
      "The \"optimized\" keyword corresponds to all other configurations.  "
      "The \"general\" keyword corresponds to all configurations, and is "
      "purely optional (assumed if omitted).  "
      "Higher granularity may be achieved for per-configuration rules "
      "by creating and linking to IMPORTED library targets.  "
      "See the IMPORTED mode of the add_library command for more "
      "information.  "
      "\n"
      "Library dependencies are transitive by default.  "
      "When this target is linked into another target then the libraries "
      "linked to this target will appear on the link line for the other "
      "target too.  "
      "See the LINK_INTERFACE_LIBRARIES target property to override the "
      "set of transitive link dependencies for a target.  "
      "Calls to other signatures of this command may set the property "
      "making any libraries linked exclusively by this signature private."
      "\n"
      "CMake will also propagate \"usage requirements\" from linked library "
      "targets.  "
      "Usage requirements affect compilation of sources in the <target>.  "
      "They are specified by properties defined on linked targets.  "
      "During generation of the build system, CMake integrates "
      "usage requirement property values with the corresponding "
      "build properties for <target>:\n"
      " INTERFACE_COMPILE_DEFINITONS: Appends to COMPILE_DEFINITONS\n"
      " INTERFACE_INCLUDE_DIRECTORIES: Appends to INCLUDE_DIRECTORIES\n"
      " INTERFACE_POSITION_INDEPENDENT_CODE: Sets POSITION_INDEPENDENT_CODE\n"
      "   or checked for consistency with existing value\n"
      "\n"
      "If an <item> is a library in a Mac OX framework, the Headers "
      "directory of the framework will also be processed as a \"usage "
      "requirement\".  This has the same effect as passing the framework "
      "directory as an include directory."
      "\n"
      "  target_link_libraries(<target> LINK_INTERFACE_LIBRARIES\n"
      "                        [[debug|optimized|general] <lib>] ...)\n"
      "The LINK_INTERFACE_LIBRARIES mode appends the libraries "
      "to the LINK_INTERFACE_LIBRARIES and its per-configuration equivalent "
      "target properties instead of using them for linking.  "
      "Libraries specified as \"debug\" are appended to the "
      "LINK_INTERFACE_LIBRARIES_DEBUG property (or to the properties "
      "corresponding to configurations listed in the DEBUG_CONFIGURATIONS "
      "global property if it is set).  "
      "Libraries specified as \"optimized\" are appended to the "
      "LINK_INTERFACE_LIBRARIES property.  "
      "Libraries specified as \"general\" (or without any keyword) are "
      "treated as if specified for both \"debug\" and \"optimized\"."
      "\n"
      "  target_link_libraries(<target>\n"
      "                        <LINK_PRIVATE|LINK_PUBLIC>\n"
      "                          [[debug|optimized|general] <lib>] ...\n"
      "                        [<LINK_PRIVATE|LINK_PUBLIC>\n"
      "                          [[debug|optimized|general] <lib>] ...])\n"
      "The LINK_PUBLIC and LINK_PRIVATE modes can be used to specify both "
      "the link dependencies and the link interface in one command.  "
      "Libraries and targets following LINK_PUBLIC are linked to, and are "
      "made part of the LINK_INTERFACE_LIBRARIES. Libraries and targets "
      "following LINK_PRIVATE are linked to, but are not made part of the "
      "LINK_INTERFACE_LIBRARIES.  "
      "\n"
      "The library dependency graph is normally acyclic (a DAG), but in the "
      "case of mutually-dependent STATIC libraries CMake allows the graph "
      "to contain cycles (strongly connected components).  "
      "When another target links to one of the libraries CMake repeats "
      "the entire connected component.  "
      "For example, the code\n"
      "  add_library(A STATIC a.c)\n"
      "  add_library(B STATIC b.c)\n"
      "  target_link_libraries(A B)\n"
      "  target_link_libraries(B A)\n"
      "  add_executable(main main.c)\n"
      "  target_link_libraries(main A)\n"
      "links 'main' to 'A B A B'.  "
      "("
      "While one repetition is usually sufficient, pathological object "
      "file and symbol arrangements can require more.  "
      "One may handle such cases by manually repeating the component in "
      "the last target_link_libraries call.  "
      "However, if two archives are really so interdependent they should "
      "probably be combined into a single archive."
      ")"
      "\n"
      "Arguments to target_link_libraries may use \"generator expressions\" "
      "with the syntax \"$<...>\".  Note however, that generator expressions "
      "will not be used in OLD handling of CMP0003 or CMP0004."
      "\n"
      CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
      ;
    }

  cmTypeMacro(cmTargetLinkLibrariesCommand, cmCommand);
private:
  void LinkLibraryTypeSpecifierWarning(int left, int right);
  static const char* LinkLibraryTypeNames[3];

  cmTarget* Target;
  enum ProcessingState {
    ProcessingLinkLibraries,
    ProcessingLinkInterface,
    ProcessingPublicInterface,
    ProcessingPrivateInterface
  };

  ProcessingState CurrentProcessingState;

  void HandleLibrary(const char* lib, cmTarget::LinkLibraryType llt);
};



#endif
