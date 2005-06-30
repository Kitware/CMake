/*=========================================================================

  Program:   WXDialog - wxWidgets X-platform GUI Front-End for CMake
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Author:    Jorgen Bodde

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _CMAKESETUP_H_
#define _CMAKESETUP_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CMakeSetup.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * CMakeSetupApp class declaration
 */

class CMakeSetupApp: public wxApp
{    
    DECLARE_CLASS( CMakeSetupApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    CMakeSetupApp();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin CMakeSetupApp event handler declarations

////@end CMakeSetupApp event handler declarations

////@begin CMakeSetupApp member function declarations

////@end CMakeSetupApp member function declarations

////@begin CMakeSetupApp member variables
////@end CMakeSetupApp member variables

private:
    wxString m_AppPath;
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(CMakeSetupApp)
////@end declare app

#endif
    // _CMAKESETUP_H_
