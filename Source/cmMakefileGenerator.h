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
#ifndef cmMakefileGenerator_h
#define cmMakefileGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmClassFile;

/** \class cmMakefileGenerator
 * \brief Provide an abstract interface for classes generating makefiles.
 *
 * Subclasses of this abstract class generate makefiles for various
 * platforms.
 */
class cmMakefileGenerator
{
public:
  /**
   * Set the cmMakefile instance from which to generate the makefile.
   */
  void SetMakefile(cmMakefile*);

  /**
   * Generate the makefile using the m_Makefile, m_CustomCommands, 
   * and m_ExtraSourceFiles. All subclasses of cmMakefileGenerator
   * must implement this method.
   */
  virtual void GenerateMakefile() = 0;

  /**
   * The local setting indicates that the generator is producing a
   * fully configured makefile in the current directory. In Microsoft
   * terms it is producing a DSP file if local is true and a DSW file
   * if local is false. On UNIX when local is false it skips the
   * dependecy check and recurses the full tree building the structure
   */
  virtual void SetLocal(bool ) {};

protected:
  cmMakefile* m_Makefile;
};

#endif
