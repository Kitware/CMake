#include "FLTKPropertyItemRow.h"
#include <Fl/Fl.H>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Tile.H>
#include <Fl/fl_ask.H>
#include <Fl/fl_file_chooser.H>

namespace fltk {

 
PropertyItemRow::PropertyItemRow( PropertyItem * pItem ):Fl_Tile(0,0,10,10,"")
{
   
  m_PropertyItem =     pItem;
  m_ItemValue    = new ItemValue;


  const unsigned int nameWidth    =        200;
  const unsigned int textWidth    =       1400;
  const unsigned int checkWidth   =  textWidth;
  const unsigned int browseWidth  =         20;
  const unsigned int firstColumn  =          0;

  const unsigned int secondColumn =  nameWidth;

  const unsigned int rowHeight    =         20;
  const unsigned int rowSpacing   =         20;
 
  size( nameWidth + textWidth , rowHeight );

  // Make the parent Fl_Pack widget at least a row wide.
  parent()->size( nameWidth + textWidth , rowHeight );

  m_NameButton = new 
          PropertyNameButtonWithHelp( firstColumn, 0, nameWidth, rowHeight, 
                                    m_PropertyItem->m_propName.c_str() );

  m_NameButton->align( FL_ALIGN_CLIP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
  m_NameButton->labelsize(11);
  m_NameButton->box( FL_DOWN_BOX );
  m_NameButton->SetHelpText( m_PropertyItem->m_HelpString.c_str() );
  m_NameButton->size( secondColumn, rowHeight );
    
  switch( m_PropertyItem->m_nItemType )
  {
  case 1: 
    {

      Fl_Input * input = new 
                    Fl_Input( secondColumn, 0, textWidth ,rowHeight ,"");
      input->value( m_PropertyItem->m_curValue.c_str() );
      input->textsize(11);
      input->callback( InputTextCallback, (void *)m_PropertyItem );
      input->when( FL_WHEN_CHANGED );

      break;
    }
  case 2:
    {
      break;
    }
  case 3:
    {
      break;
    }
  case 4:
    {

      Fl_Button * browseButton = new 
            Fl_Button( secondColumn, 0, browseWidth  ,rowHeight ,"...");
      browseButton->labelsize(11);

      Fl_Input * input = new 
                    Fl_Input( secondColumn+browseWidth, 0, textWidth ,rowHeight ,"");
      input->value( m_PropertyItem->m_curValue.c_str() );
      input->textsize(11);

      m_ItemValue->m_InputText    = input;
      m_ItemValue->m_PropertyItem = m_PropertyItem;
        
      browseButton->callback( BrowsePathCallback, (void *)m_ItemValue );
      input->callback( InputTextCallback, m_PropertyItem );
      input->when( FL_WHEN_CHANGED );
      
      break;
    }
  case 5:
    {
      Fl_Button * button = new 
            Fl_Button( secondColumn, 0, checkWidth  ,rowHeight ,"");
      button->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
      button->callback( CheckButtonCallback, (void *)m_PropertyItem );

      if( m_PropertyItem->m_curValue == "ON" ) 
      {
        button->label(" ON  ");
        button->value(1);
      }
      else if( m_PropertyItem->m_curValue == "OFF" )
      {
        button->label(" OFF ");
        button->value(0);
      }
      button->type( FL_TOGGLE_BUTTON );
      button->labelsize(11);
      break;
    }
  case 6:
    {

      Fl_Button * browseButton = new 
            Fl_Button( secondColumn, 0, browseWidth  ,rowHeight ,"...");
      browseButton->labelsize(11);

      Fl_Input * input = new 
                    Fl_Input( secondColumn+browseWidth, 0, textWidth ,rowHeight ,"");
      input->value( m_PropertyItem->m_curValue.c_str() );
      input->textsize(11);

      m_ItemValue->m_InputText    = input;
      m_ItemValue->m_PropertyItem = m_PropertyItem;
        
      browseButton->callback( BrowsePathCallback, (void *)m_ItemValue );
      input->callback( InputTextCallback, m_PropertyItem );
      input->when( FL_WHEN_CHANGED );
      
      break;
    }
    break;
  default:
    fl_alert("Unkown item type %d",m_PropertyItem->m_nItemType);
    break;
  }


  end(); // Close the inclusion of widgets in the Tile object


}





PropertyItemRow::~PropertyItemRow( )
{
  delete m_ItemValue;
}




  
void 
PropertyItemRow::
CheckButtonCallback( Fl_Widget * widget, void * data) 
{
  Fl_Button    * button = (Fl_Button *)widget;
  PropertyItem * pItem  = (PropertyItem *)data;
  
  int value = button->value();

  if( value )
  {
    button->label(" ON ");
    pItem->m_curValue = "ON";
  }
  else 
  {
    button->label(" OFF ");
    pItem->m_curValue = "OFF";
  }
  button->redraw();
}



void 
PropertyItemRow::
InputTextCallback(   Fl_Widget * widget, void * data)
{
  Fl_Input  * input     = (Fl_Input *)widget;
  PropertyItem * item   = (PropertyItem *)data;
  
  item->m_curValue      = input->value();

}





void 
PropertyItemRow::
BrowsePathCallback(   Fl_Widget * widget, void * data)
{
  ItemValue    * itemValue    = (ItemValue *)data;
  Fl_Input     * inputText    = itemValue->m_InputText;
  PropertyItem * propertyItem = itemValue->m_PropertyItem;

  const char * newpath = 
    fl_file_chooser("Select a path","*", inputText->value() );
  
  if( newpath ) 
  {
    propertyItem->m_curValue = newpath;
    inputText->value( newpath );
  }

}


int 
PropertyItemRow::
handle(int event)
{
  
  int status = Fl_Tile::handle( event );
  
  switch( event ) 
  {
    case FL_LEAVE:
      m_NameButton->HideHelp();
      status = 1;
      break;
    case FL_MOVE:
      m_NameButton->HideHelp();
      status = 1;
      break;
  }
  
  return status;
}


} // end namespace fltk
