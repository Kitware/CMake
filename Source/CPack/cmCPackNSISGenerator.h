/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackNSISGenerator_h
#define cmCPackNSISGenerator_h


#include "cmCPackGenerator.h"
#include <set>

/** \class cmCPackNSISGenerator
 * \brief A generator for NSIS files
 *
 * http://people.freebsd.org/~kientzle/libarchive/
 */
class cmCPackNSISGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackNSISGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackNSISGenerator();
  virtual ~cmCPackNSISGenerator();

protected:
  virtual int InitializeInternal();
  void CreateMenuLinks( cmOStringStream& str,
                        cmOStringStream& deleteStr);
  int PackageFiles();
  virtual const char* GetOutputExtension() { return ".exe"; }
  virtual const char* GetOutputPostfix() { return "win32"; }

  bool GetListOfSubdirectories(const char* dir,
    std::vector<std::string>& dirs);

  virtual bool SupportsComponentInstallation() const;

  /// Produce a string that contains the NSIS code to describe a
  /// particular component. Any added macros will be emitted via
  /// macrosOut.
  std::string
  CreateComponentDescription(cmCPackComponent *component,
                             cmOStringStream& macrosOut);

  /// Produce NSIS code that selects all of the components that this component
  /// depends on, recursively.
  std::string CreateSelectionDependenciesDescription
                (cmCPackComponent *component,
                 std::set<cmCPackComponent *>& visited);

  /// Produce NSIS code that de-selects all of the components that are
  /// dependent on this component, recursively.
  std::string CreateDeselectionDependenciesDescription
                (cmCPackComponent *component,
                 std::set<cmCPackComponent *>& visited);

  /// Produce a string that contains the NSIS code to describe a
  /// particular component group, including its components. Any
  /// added macros will be emitted via macrosOut.
  std::string
  CreateComponentGroupDescription(cmCPackComponentGroup *group,
                                  cmOStringStream& macrosOut);

  /// Translations any newlines found in the string into \\r\\n, so that the
  /// resulting string can be used within NSIS.
  static std::string TranslateNewlines(std::string str);
};

#endif
