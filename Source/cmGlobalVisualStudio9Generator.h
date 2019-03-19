/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalVisualStudio9Generator_h
#define cmGlobalVisualStudio9Generator_h

#include "cmGlobalVisualStudio8Generator.h"

/** \class cmGlobalVisualStudio9Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio9Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio9Generator : public cmGlobalVisualStudio8Generator
{
public:
  static cmGlobalGeneratorFactory* NewFactory();

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  std::string GetUserMacrosDirectory() override;

  /**
   * What is the reg key path to "vsmacros" for this version of Visual
   * Studio?
   */
  std::string GetUserMacrosRegKeyBase() override;

protected:
  cmGlobalVisualStudio9Generator(cmake* cm, const std::string& name,
                                 std::string const& platformInGeneratorName);

private:
  class Factory;
  friend class Factory;
};
#endif
