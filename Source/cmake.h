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


#include "cmMakefile.h"
#include "cmStandardIncludes.h"

class cmake
{
 public:

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void Usage(const char *program);

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  The argument is a list of command line
   * arguments.  The first argument should be the name or full path
   * to the command line version of cmake.  For building a GUI,
   * you would pass in the following arguments:
   * /path/to/cmake -H/path/to/source -B/path/to/build 
   */
  int Generate(const std::vector<std::string>&);

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void SetArgs(cmMakefile& builder, const std::vector<std::string>&);

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  void AddCMakePaths(const std::vector<std::string>&);

  /**
   * constructor
   */
  cmake() {m_Verbose = false;}

 private:
  bool m_Verbose;
  bool m_Local;
};

