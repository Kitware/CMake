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

#ifndef _APP_RESOURCES_H_
#define _APP_RESOURCES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "app_resources.cpp"
#endif

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CACHE_IGNORE 10051
#define ID_CACHE_DELETE 10052
#define ID_CACHE_BROWSE 10053
////@end control identifiers

class AppResources
{
public:
    AppResources() {}

/*!
 * Resource functions
 */

////@begin AppResources resource functions
    /// Menu creation function for ID_MENU
    static wxMenu* CreatePopupMenu();

    /// Retrieves bitmap resources
    static wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    static wxIcon GetIconResource( const wxString& name );
////@end AppResources resource functions

};

#endif
    // _APP_RESOURCES_H_
