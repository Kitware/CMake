/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    m_AbstractClass = false;
    m_HeaderFileOnly = false;
    m_WrapExclude = false;
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

  /**
   * Indicate whether the class is abstract (non-instantiable).
   */
  bool IsAnAbstractClass() const { return m_AbstractClass; }
  bool GetIsAnAbstractClass() const { return m_AbstractClass; }
  void SetIsAnAbstractClass(bool f) { m_AbstractClass = f; }

  /**
   * Indicate whether the class should not be wrapped
   */
  bool GetWrapExclude() const { return m_WrapExclude; }
  void SetWrapExclude(bool f) { m_WrapExclude = f; }

  /**
   * Indicate whether this class is defined with only the header file.
   */
  bool IsAHeaderFileOnly() const { return m_HeaderFileOnly; }
  bool GetIsAHeaderFileOnly() const { return m_HeaderFileOnly; }
  void SetIsAHeaderFileOnly(bool f) { m_HeaderFileOnly = f; }

  /**
   * The full path to the file.
   */
  std::string GetFullPath() const {return m_FullPath;}
  void SetFullPath(const char *name) {m_FullPath = name;}

  /**
   * The file name associated with stripped off directory and extension.
   * (In most cases this is the name of the class.)
   */
  std::string GetSourceName() const {return m_SourceName;}
  void SetSourceName(const char *name) {m_SourceName = name;}

  /**
   * The file name associated with stripped off directory and extension.
   * (In most cases this is the name of the class.)
   */
  std::string GetSourceExtension() const {return m_SourceExtension;}
  void SetSourceExtension(const char *name) {m_SourceExtension = name;}

  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return m_Depends;}
  std::vector<std::string> &GetDepends() {return m_Depends;}

private:
  bool m_AbstractClass;
  bool m_WrapExclude;
  bool m_HeaderFileOnly;
  std::string m_FullPath;
  std::string m_SourceName;
  std::string m_SourceExtension;
  std::vector<std::string> m_Depends;
};

#endif
