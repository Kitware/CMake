/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalVisualStudio15Generator_h
#define cmGlobalVisualStudio15Generator_h

#include "cmGlobalVisualStudio14Generator.h"

/** \class cmGlobalVisualStudio15Generator  */
class cmGlobalVisualStudio15Generator : public cmGlobalVisualStudio14Generator
{
public:
  cmGlobalVisualStudio15Generator(cmake* cm, const std::string& name,
                                  const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

  virtual const char* GetToolsVersion() { return "15.0"; }
protected:
  virtual bool SelectWindowsStoreToolset(std::string& toolset) const;

  virtual const char* GetIDEVersion() { return "15.0"; }

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  virtual bool IsWindowsDesktopToolsetInstalled() const;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

private:
  class Factory;
};
#endif
