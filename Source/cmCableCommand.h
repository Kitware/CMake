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
#ifndef cmCableCommand_h
#define cmCableCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCableData.h"

/** \class cmCableCommand
 * \brief Superclass for all cmCable command classes.
 *
 * cmCableCommand is the superclass for all CABLE-related commands.
 * The C++ Automated Bindings for Language Extension (CABLE) tool is
 * configured using an XML input file.  The input format is quite
 * flexible, but XML is hard for humans to write by hand.  The CABLE
 * commands in CMake are designed to simplify the interface with only
 * a small loss in functionality.  These commands can be used to
 * automatically generate CABLE configuration files.
 */
class cmCableCommand : public cmCommand
{
public:
  cmCableCommand();
  virtual ~cmCableCommand();
  
  cmTypeMacro(cmCableCommand, cmCommand);
protected:
  void SetupCableData();
  
  /**
   * The cmCableData holding common information for all cmCableCommand
   * instances.
   */
  cmCableData* m_CableData;
};



#endif
