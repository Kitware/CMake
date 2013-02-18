/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmConfigureFileCommand_h
#define cmConfigureFileCommand_h

#include "cmCommand.h"

class cmConfigureFileCommand : public cmCommand
{
public:
  cmTypeMacro(cmConfigureFileCommand, cmCommand);

  virtual cmCommand* Clone()
    {
      return new cmConfigureFileCommand;
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
  virtual const char* GetName() const { return "configure_file";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Copy a file to another location and modify its contents.";
    }

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
      return
        "  configure_file(<input> <output>\n"
        "                 [COPYONLY] [ESCAPE_QUOTES] [@ONLY] \n"
        "                 [NEWLINE_STYLE [UNIX|DOS|WIN32|LF|CRLF] ])\n"
        "Copies a file <input> to file <output> and substitutes variable "
        "values referenced in the file content.  "
        "If <input> is a relative path it is evaluated with respect to "
        "the current source directory.  "
        "The <input> must be a file, not a directory.  "
        "If <output> is a relative path it is evaluated with respect to "
        "the current binary directory.  "
        "If <output> names an existing directory the input file is placed "
        "in that directory with its original name.  "
        "\n"
        "If the <input> file is modified the build system will re-run CMake "
        "to re-configure the file and generate the build system again."
        "\n"
        "This command replaces any variables in the input file referenced as "
        "${VAR} or @VAR@ with their values as determined by CMake.  If a "
        "variable is not defined, it will be replaced with nothing.  "
        "If COPYONLY is specified, then no variable expansion will take "
        "place.  If ESCAPE_QUOTES is specified then any substituted quotes "
        "will be C-style escaped.  "
        "The file will be configured with the current values of CMake "
        "variables. If @ONLY is specified, only variables "
        "of the form @VAR@ will be replaced and ${VAR} will be ignored.  "
        "This is useful for configuring scripts that use ${VAR}."
        "\n"
        "Input file lines of the form \"#cmakedefine VAR ...\" "
        "will be replaced with either \"#define VAR ...\" or "
        "\"/* #undef VAR */\" depending on whether VAR is set in CMake to "
        "any value not considered a false constant by the if() command. "
        "(Content of \"...\", if any, is processed as above.) "
        "Input file lines of the form \"#cmakedefine01 VAR\" "
        "will be replaced with either \"#define VAR 1\" or "
        "\"#define VAR 0\" similarly."
        "\n"
        "With NEWLINE_STYLE the line ending could be adjusted: \n"
        "    'UNIX' or 'LF' for \\n, 'DOS', 'WIN32' or 'CRLF' for \\r\\n.\n"
        "COPYONLY must not be used with NEWLINE_STYLE.\n";
    }

  virtual void FinalPass();
  virtual bool HasFinalPass() const { return !this->Immediate; }

private:
  int ConfigureFile();

  cmNewLineStyle NewLineStyle;

  std::string InputFile;
  std::string OutputFile;
  bool CopyOnly;
  bool EscapeQuotes;
  bool Immediate;
  bool AtOnly;
};



#endif
