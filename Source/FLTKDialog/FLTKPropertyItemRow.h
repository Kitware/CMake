
#ifndef  FLTKPropertyItemRow_h
#define  FLTKPropertyItemRow_h

#include "FLTKPropertyList.h"
#include "FLTKPropertyNameButtonWithHelp.h"

#include <Fl/Fl_Tile.H>
#include <Fl/Fl_Input.H>

namespace fltk {
  

/**

   Class to manage a GUI row corresponding to a property
  
  */
class PropertyItemRow  : public Fl_Tile
{

  // Helper class for passing data to callbacks
  struct ItemValue
  {
    PropertyItem * m_PropertyItem;
    Fl_Input     * m_InputText;
  };

 
  public:

    PropertyItemRow( PropertyItem * );
    ~PropertyItemRow();
    int handle(int event);
    
  private:
    
    PropertyItem * m_PropertyItem;
    ItemValue    * m_ItemValue;

    // Button that displays the property name and
    // manages the popup help blob
    PropertyNameButtonWithHelp * m_NameButton;

    static void CheckButtonCallback( Fl_Widget *, void *);
    static void InputTextCallback(   Fl_Widget *, void *);
    static void BrowsePathCallback(  Fl_Widget *, void *);

};


} // end namespace fltk


#endif


