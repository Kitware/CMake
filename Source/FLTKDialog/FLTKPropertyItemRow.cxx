#include "FLTKPropertyItemRow.h"
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Tile.H>
#include <Fl/fl_ask.H>
#include <Fl/fl_file_chooser.H>

namespace fltk {

PropertyItemRow::PropertyItemRow( PropertyItem * pItem )
{
   
  m_PropertyItem = pItem;
  m_ItemValue    = new ItemValue;

  const unsigned int nameWidth    =        200;
  const unsigned int textWidth    =       1400;
  const unsigned int checkWidth   =  textWidth;
  const unsigned int browseWidth  =         20;
  const unsigned int firstColumn  =          0;

  const unsigned int secondColumn =  nameWidth;

  const unsigned int rowHeight    =         20;
  const unsigned int rowSpacing   =         20;
 
  Fl_Tile * group = new Fl_Tile(0,0,nameWidth+textWidth,rowHeight,"");

  // Make the parent Fl_Pack widget at least a row wide.
  group->parent()->size( nameWidth + textWidth , rowHeight );

  Fl_Button * name = new 
                Fl_Button( firstColumn, 0, nameWidth, rowHeight, 
                                              pItem->m_propName.c_str() );
  name->align( FL_ALIGN_CLIP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
  name->labelsize(11);
  name->box( FL_DOWN_BOX );
  name->callback( NameClickCallback, (void *)pItem );
      
  switch( pItem->m_nItemType )
  {
  case 1: 
    {

      name->size( secondColumn, rowHeight );

      Fl_Input * input = new 
                    Fl_Input( secondColumn, 0, textWidth ,rowHeight ,"");
      input->value( pItem->m_curValue.c_str() );
      input->textsize(11);
      input->callback( InputTextCallback, (void *)pItem );
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

      name->size( secondColumn, rowHeight );
      Fl_Button * browseButton = new 
            Fl_Button( secondColumn, 0, browseWidth  ,rowHeight ,"...");
      browseButton->labelsize(11);

      Fl_Input * input = new 
                    Fl_Input( secondColumn+browseWidth, 0, textWidth ,rowHeight ,"");
      input->value( pItem->m_curValue.c_str() );
      input->textsize(11);

      m_ItemValue->m_InputText    = input;
      m_ItemValue->m_PropertyItem = pItem;
        
      browseButton->callback( BrowsePathCallback, (void *)m_ItemValue );
      input->callback( InputTextCallback, pItem );
      input->when( FL_WHEN_CHANGED );
      
      break;
    }
  case 5:
    {
      Fl_Button * button = new 
            Fl_Button( secondColumn, 0, checkWidth  ,rowHeight ,"");
      button->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
      button->callback( CheckButtonCallback, (void *)pItem );

      if( pItem->m_curValue == "ON" ) 
      {
        button->label(" ON  ");
        button->value(1);
      }
      else if( pItem->m_curValue == "OFF" )
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

      name->size( secondColumn, rowHeight );
      Fl_Button * browseButton = new 
            Fl_Button( secondColumn, 0, browseWidth  ,rowHeight ,"...");
      browseButton->labelsize(11);

      Fl_Input * input = new 
                    Fl_Input( secondColumn+browseWidth, 0, textWidth ,rowHeight ,"");
      input->value( pItem->m_curValue.c_str() );
      input->textsize(11);

      m_ItemValue->m_InputText    = input;
      m_ItemValue->m_PropertyItem = pItem;
        
      browseButton->callback( BrowsePathCallback, (void *)m_ItemValue );
      input->callback( InputTextCallback, pItem );
      input->when( FL_WHEN_CHANGED );
      
      break;
    }
    break;
  default:
    fl_alert("Unkown item type %d",pItem->m_nItemType);
    break;
  }


  group->end();

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
NameClickCallback(   Fl_Widget * widget, void * data) 
{
  PropertyItem * pItem  = (PropertyItem *)data;
  fl_message( pItem->m_HelpString.c_str() );
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



} // end namespace fltk
