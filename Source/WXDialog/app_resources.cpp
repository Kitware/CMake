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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "app_resources.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "app_resources.h"

////@begin XPM images
////@end XPM images

/*!
 * Resource functions
 */

////@begin AppResources resource functions
/*!
 * Menu creation function for ID_MENU
 */

wxMenu* AppResources::CreatePopupMenu()
{
    wxMenu* itemMenu1 = new wxMenu;
    itemMenu1->Append(ID_CACHE_IGNORE, _("&Ignore cache entry"), _("Ignores the value of the current cache entry"), wxITEM_NORMAL);
    itemMenu1->Append(ID_CACHE_DELETE, _("&Delete cache entry"), _("Deletes the current cache entry (reverts on next configure)"), wxITEM_NORMAL);
    itemMenu1->AppendSeparator();
    itemMenu1->Append(ID_CACHE_BROWSE, _("&Browse ..."), _T(""), wxITEM_NORMAL);
    return itemMenu1;
}

////@end AppResources resource functions

/*!
 * Get bitmap resources
 */

wxBitmap AppResources::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin AppResources bitmap retrieval
    return wxNullBitmap;
////@end AppResources bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AppResources::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin AppResources icon retrieval
    return wxNullIcon;
////@end AppResources icon retrieval
}
