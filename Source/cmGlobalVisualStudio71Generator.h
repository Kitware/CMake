/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalVisualStudio71Generator_h
#define cmGlobalVisualStudio71Generator_h

#include "cmGlobalVisualStudio7Generator.h"


/** \class cmGlobalVisualStudio71Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio71Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio71Generator : public cmGlobalVisualStudio7Generator
{
public:
  cmGlobalVisualStudio71Generator();
  static cmGlobalGenerator* New() { return new cmGlobalVisualStudio71Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio71Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 7 .NET 2003";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

protected:
  virtual void WriteSLNFile(std::ostream& fout, 
                            cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  virtual void WriteProject(std::ostream& fout, 
                            const char* name, const char* path,
                            const cmTarget &t);
  virtual void WriteProjectDepends(std::ostream& fout, 
                           const char* name, const char* path,
                           const cmTarget &t);
  virtual void WriteProjectConfigurations(std::ostream& fout, const char* name, bool in_all);
  virtual void WriteExternalProject(std::ostream& fout, const char* name, const char* path,
                                    const std::vector<std::string>& depends);
  virtual void WriteSLNFooter(std::ostream& fout);
  virtual void WriteSLNHeader(std::ostream& fout);
};
#endif
