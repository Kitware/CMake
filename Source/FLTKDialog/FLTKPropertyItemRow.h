
#ifndef  FLTKPropertyItemRow_h
#define  FLTKPropertyItemRow_h

#include "FLTKPropertyList.h"

#include <FL/Fl_Tile.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>


class CMakeSetupGUIImplementation;  


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

    PropertyItemRow( PropertyItem *);
    ~PropertyItemRow();

  private:
    
    PropertyItem * m_PropertyItem;
    ItemValue    * m_ItemValue;
    Fl_Button    * m_NameButton;

    static CMakeSetupGUIImplementation * m_CMakeSetup;

    static void CheckButtonCallback( Fl_Widget *, void *);
    static void NameButtonCallback( Fl_Widget *, void *);
    static void InputTextCallback(   Fl_Widget *, void *);
    static void BrowsePathCallback(  Fl_Widget *, void *);
    static void ColorSelectionCallback(   Fl_Widget * widget, void * data);

    static void FillCacheManagerFromCacheGUI( void );

  public:
    
    static void SetCMakeSetupGUI( CMakeSetupGUIImplementation * );
};


} // end namespace fltk


#endif


