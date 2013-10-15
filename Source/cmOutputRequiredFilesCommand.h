/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOutputRequiredFilesCommand_h
#define cmOutputRequiredFilesCommand_h

#include "cmCommand.h"
#include "cmMakeDepend.h"

/** \class cmOutputRequiredFilesCommand
 * \brief Output a list of required files for a source file
 *
 */
class cmOutputRequiredFilesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmOutputRequiredFilesCommand;
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
  virtual const char* GetName() const { return "output_required_files";}

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }


  cmTypeMacro(cmOutputRequiredFilesCommand, cmCommand);
  void ListDependencies(cmDependInformation const *info,
                        FILE *fout,
                        std::set<cmDependInformation const*> *visited);

private:
  std::string File;
  std::string OutputFile;
};



#endif
