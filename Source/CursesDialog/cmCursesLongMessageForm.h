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
#ifndef __cmCursesLongMessageForm_h
#define __cmCursesLongMessageForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesCacheEntryComposite;

class cmCursesLongMessageForm : public cmCursesForm
{
public:
  cmCursesLongMessageForm(std::vector<std::string> const& messages, 
                          const char* title);
  virtual ~cmCursesLongMessageForm();
  
  // Description:
  // Handle user input.
  virtual void HandleInput();

  // Description:
  // Display form. Use a window of size width x height, starting
  // at top, left.
  virtual void Render(int left, int top, int width, int height);

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void PrintKeys();

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar();

protected:
  cmCursesLongMessageForm(const cmCursesLongMessageForm& from);
  void operator=(const cmCursesLongMessageForm&);

  std::string m_Messages;
  std::string m_Title;

  FIELD* m_Fields[2];

};

#endif // __cmCursesLongMessageForm_h
