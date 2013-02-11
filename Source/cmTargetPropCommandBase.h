/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmTargetPropCommandBase_h
#define cmTargetPropCommandBase_h

#include "cmCommand.h"
#include "cmDocumentGeneratorExpressions.h"

class cmTarget;

//----------------------------------------------------------------------------
class cmTargetPropCommandBase : public cmCommand
{
public:

  enum ArgumentFlags {
    NO_FLAGS = 0,
    PROCESS_BEFORE = 1
  };

  bool HandleArguments(std::vector<std::string> const& args,
                           const char *prop, ArgumentFlags flags = NO_FLAGS);

private:
  virtual void HandleImportedTarget(const std::string &tgt) = 0;
  virtual void HandleMissingTarget(const std::string &name) = 0;

  virtual bool HandleNonTargetArg(std::string &content,
                          const std::string &sep,
                          const std::string &entry,
                          const std::string &tgt) = 0;

  virtual void HandleDirectContent(cmTarget *tgt,
                                   const std::string &content,
                                   bool prepend) = 0;

  bool ProcessContentArgs(std::vector<std::string> const& args,
                          unsigned int &argIndex, bool prepend);
  void PopulateTargetProperies(const std::string &scope,
                               const std::string &content, bool prepend);

private:
  cmTarget *Target;
  std::string Property;
};

#endif
