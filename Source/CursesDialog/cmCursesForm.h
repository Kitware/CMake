/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __cmCursesForm_h
#define __cmCursesForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesStandardIncludes.h"

class cmCursesForm
{
public:
  cmCursesForm();
  virtual ~cmCursesForm();
  
  // Description:
  // Handle user input.
  virtual void HandleInput() = 0;

  // Description:
  // Display form.
  virtual void Render(int left, int top, int width, int height) = 0;

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar() = 0;

  // Description:
  // During a CMake run, an error handle should add errors
  // to be displayed afterwards.
  virtual void AddError(const char*, const char*) {}

  // Description:
  // Turn debugging on. This will create ccmakelog.txt.
  static void DebugStart();

  // Description:
  // Turn debugging off. This will close ccmakelog.txt.
  static void DebugEnd();

  // Description:
  // Write a debugging message.
  static void LogMessage(const char* msg);

  // Description:
  // Return the FORM. Should be only used by low-level methods.
  FORM* GetForm()
    {
      return m_Form;
    }

  static cmCursesForm* CurrentForm;
  

protected:

  static std::ofstream DebugFile;
  static bool Debug;

  cmCursesForm(const cmCursesForm& from);
  void operator=(const cmCursesForm&);

  FORM* m_Form;
};

#endif // __cmCursesForm_h
