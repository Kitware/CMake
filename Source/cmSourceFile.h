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
#ifndef cmSourceFile_h
#define cmSourceFile_h

#include "cmStandardIncludes.h"

/** \class cmSourceFile
 * \brief Represent a class loaded from a makefile.
 *
 * cmSourceFile is represents a class loaded from 
 * a makefile.
 */
class cmSourceFile
{
public:
  /**
   * Construct instance as a concrete class with both a
   * .h and .cxx file.
   */
  cmSourceFile()
    {
    }
  
  /**
   * Set the name of the file, given the directory the file should be
   * in.  The various extensions provided are tried on the name
   * (e.g., cxx, cpp) in the directory to find the actual file.
   */
  void SetName(const char* name, const char* dir,
               const std::vector<std::string>& sourceExts,
               const std::vector<std::string>& headerExts);

  /**
   * Set the name of the file, given the directory the file should be in.  IN
   * this version the extension is provided in the call. This is useful for
   * generated files that do not exist prior to the build.  
   */
  void SetName(const char* name, const char* dir, const char *ext, 
               bool headerFileOnly);

  /**
   * Print the structure to std::cout.
   */
  void Print() const;

  ///! Set/Get a property of this source file
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;
    
  /**
   * The full path to the file.
   */
  const std::string &GetFullPath() const {return m_FullPath;}
  void SetFullPath(const char *name) {m_FullPath = name;}

  /**
   * The file name associated with stripped off directory and extension.
   * (In most cases this is the name of the class.)
   */
  const std::string &GetSourceName() const {return m_SourceName;}
  void SetSourceName(const char *name) {m_SourceName = name;}

  /**
   * The file name associated with stripped off directory and extension.
   * (In most cases this is the name of the class.)
   */
  const std::string &GetSourceExtension() const {return m_SourceExtension;}
  void SetSourceExtension(const char *name) {m_SourceExtension = name;}

  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return m_Depends;}
  std::vector<std::string> &GetDepends() {return m_Depends;}

private:

  std::map<cmStdString,cmStdString> m_Properties;
  std::string m_FullPath;
  std::string m_SourceName;
  std::string m_SourceExtension;
  std::vector<std::string> m_Depends;
};

#endif
