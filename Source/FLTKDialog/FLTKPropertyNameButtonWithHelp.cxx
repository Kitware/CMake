// #include <FLTKPropertyNameButtonWithHelp.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Menu_Button.H>
#include "../cmCacheManager.h"
#include "FLTKPropertyNameButtonWithHelp.h"


namespace fltk {
  
Fl_Window *   PropertyNameButtonWithHelp::helpBlob = 0;
Fl_Box    *   PropertyNameButtonWithHelp::helpText = 0;
unsigned int  PropertyNameButtonWithHelp::counter  = 0;
int           PropertyNameButtonWithHelp::lastMousePositionX = 0;
int           PropertyNameButtonWithHelp::lastMousePositionY = 0;
 


PropertyNameButtonWithHelp
::PropertyNameButtonWithHelp(int x,int y, int w, int h, const char * l):
Fl_Button(x,y,w,h,l) 
{
  counter++;    // one more object instantiated 
}




PropertyNameButtonWithHelp::
~PropertyNameButtonWithHelp( )
{
  counter--;
  if( counter == 0 )
  {
    delete helpBlob;
    delete helpText;
  }
}



void
PropertyNameButtonWithHelp
::SetHelpText( const char * text )
{
  m_HelpText = text;
}



void
PropertyNameButtonWithHelp
::ShowHelp( void )
{
  if( helpBlob )
  {
    helpBlob->show();
  }
}
 

void
PropertyNameButtonWithHelp
::HideHelp( void )
{
  if( helpBlob )
  {
    helpBlob->hide();
  }
}




int 
PropertyNameButtonWithHelp::
handle( int event )
{

  static bool helpBlobVisible = false;

  const float delayForShowingHelpBlob = 1.0; // seconds
  
  const int   maxWidth   = 300;
  const int   lineHeight =  20;


  // Create the help blob window if it doesn't exist
  if( !helpBlob )
  {
    helpBlob = new Fl_Window(0,0,200,20,"");
    helpBlob->border( 0 );
    helpText = new Fl_Box(0,0,200,20,"");
    helpBlob->end();
    Fl_Color yellowHelp = FL_YELLOW;
    helpBlob->color( yellowHelp );
    helpText->color( yellowHelp );
    helpText->align( FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP );
  }
 
  int eventManaged = 0;
  
  switch( event )
  {
    case FL_ENTER: 
    {
      lastMousePositionX = Fl::event_x();
      lastMousePositionY = Fl::event_y();
      const float factor = helpText->labelsize() * 0.5;
      int height = lineHeight;
      int area   = (int)( m_HelpText.size() * factor );
      int width  = area;
      if( width > maxWidth )
      {
        width  = maxWidth;
        height = area / maxWidth * lineHeight;
        if( area % maxWidth != 0 ) 
        {
          height += lineHeight;
        }
      }
      helpText->size( width, height );
      helpBlob->size( width, height );
      helpText->label(  m_HelpText.c_str() );
      Fl_Widget * parent = this->parent();
      Fl::add_timeout( delayForShowingHelpBlob, ShowHelpBlobCallback, (void *)parent );
      helpBlobVisible = true;
      eventManaged = 0;
      break;
     }
    case FL_LEAVE:
    {
      if( helpBlobVisible )
      {
        helpBlobVisible = false;
        helpBlob->hide();
      }
      eventManaged = 0;
      break;
    }
    case FL_MOVE:
      if( helpBlobVisible )
      {
        helpBlobVisible = false;
        helpBlob->hide();
      }
      eventManaged = 0;
      break;
    case FL_PUSH:
      if( Fl::event_button() == FL_RIGHT_MOUSE )
      {
        PopupMenu();
      }
      eventManaged = 0;
      break;
    default:
      eventManaged = 0;
  }
  

  return eventManaged;

}

void 
PropertyNameButtonWithHelp::
ShowHelpBlobCallback( void * data )   
{

  Fl_Widget * thisWidget  = Fl::belowmouse();
  Fl_Widget * eventWidget = (Fl_Widget *)data;

  if( thisWidget == eventWidget )
  {

    helpBlob->position( lastMousePositionX, lastMousePositionY );
    helpBlob->show();
  }

}



////////////////////////////////////////////////////////////////
//  This popup menu is displayed when the 
//  right mouse button is pressed
void 
PropertyNameButtonWithHelp::
PopupMenu(void)
{
  static Fl_Menu_Button * popupMenu = 0;
  if( !popupMenu )
  {
    popupMenu = new Fl_Menu_Button(0,0,100,200);
  }
  
  popupMenu->type( Fl_Menu_Button::POPUP3 );
  popupMenu->add("Remove|Properties...");
  popupMenu->popup();
  
  switch( popupMenu->value() )
  {
    case 0: // Remove
    {
      const char * propertyName = label();
      int answer = fl_ask( "Do you want to remove property %s", propertyName );
      if( answer == 1 )
      {
        // Remove the entry from the cache
        cmCacheManager::GetInstance()->RemoveCacheEntry( propertyName );
        // Get the parent: Fl_Tile that manages the whole row in the GUI
        Fl_Group * parentGroup      = (Fl_Group *) parent();
        // Get the grandParent: Fl_Pack with the property list
        Fl_Group * grandParentGroup = (Fl_Group *) parentGroup->parent();
        // Remove the row from the list
        grandParentGroup->remove( *parentGroup );
        // Destroy the row
        delete parentGroup;  // Patricide... ?
        // Redraw the list
        grandParentGroup->redraw();
        return;
      }
      break;
    }
    case 1: // Properties
      break;
  }
}




} // end namespace fltk
