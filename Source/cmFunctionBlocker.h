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
#ifndef cmFunctionBlocker_h
#define cmFunctionBlocker_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmFunctionBlocker
 * \brief A class that defines an interface for blocking cmake functions
 *
 * This is the superclass for any classes that need to block a cmake function
 */
class cmFunctionBlocker
{
public:
  /**
   * should a function be blocked
   */
  virtual bool IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                                 const cmMakefile &mf) const = 0;

  /**
   * should this function blocker be removed, useful when one function adds a
   * blocker and another must remove it 
   */
  virtual bool ShouldRemove(const char *name, 
                            const std::vector<std::string> &args, 
                            const cmMakefile &mf) const {return false;}

  /**
   * When the end of a CMakeList file is reached this method is called.  It
   * is not called on the end of an INCLUDE cmake file, just at the end of a
   * regular CMakeList file 
   */
  virtual void ScopeEnded(const cmMakefile &mf) const {}
};

#endif
