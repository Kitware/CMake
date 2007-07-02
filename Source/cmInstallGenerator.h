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
#ifndef cmInstallGenerator_h
#define cmInstallGenerator_h

#include "cmStandardIncludes.h"

class cmLocalGenerator;

class cmInstallGeneratorIndent
{
public:
  cmInstallGeneratorIndent(): Level(0) {}
  cmInstallGeneratorIndent(int level): Level(level) {}
  void Write(std::ostream& os) const
    {
    for(int i=0; i < this->Level; ++i)
      {
      os << " ";
      }
    }
  cmInstallGeneratorIndent Next(int step = 2) const
    {
    return cmInstallGeneratorIndent(this->Level + step);
    }
private:
  int Level;
};
inline std::ostream& operator<<(std::ostream& os,
                                cmInstallGeneratorIndent const& indent)
{
  indent.Write(os);
  return os;
}

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator
{
public:
  cmInstallGenerator();
  cmInstallGenerator(const char* dest):Destination(dest?dest:"") {}
  virtual ~cmInstallGenerator();

  void Generate(std::ostream& os, const char* config,
                std::vector<std::string> const& configurationTypes);

  static void AddInstallRule(
    std::ostream& os, const char* dest, int type,
    std::vector<std::string> const& files,
    bool optional = false,
    const char* properties = 0,
    const char* permissions_file = 0,
    const char* permissions_dir = 0,
    std::vector<std::string> const& configurations 
    = std::vector<std::string>(),
    const char* component = 0,
    const char* rename = 0,
    const char* literal_args = 0,
    cmInstallGeneratorIndent const& indent = cmInstallGeneratorIndent()
    );

  const char* GetDestination() const        {return this->Destination.c_str();}
protected:
  virtual void GenerateScript(std::ostream& os)=0;

  const char* ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;
  std::string Destination;
};

#endif
