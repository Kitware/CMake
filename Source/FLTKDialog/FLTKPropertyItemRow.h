#include "FLTKPropertyList.h"

#include <Fl/Fl_Widget.H>
#include <Fl/Fl_Input.H>

namespace fltk {
  
class PropertyItemRow  
{
  struct ItemValue
  {
    PropertyItem * m_PropertyItem;
    Fl_Input     * m_InputText;
  };

  public:
    PropertyItemRow( PropertyItem * );
    ~PropertyItemRow();
  private:
    PropertyItem * m_PropertyItem;
    ItemValue    * m_ItemValue;

    static void CheckButtonCallback( Fl_Widget *, void *);
    static void NameClickCallback(   Fl_Widget *, void *);
    static void InputTextCallback(   Fl_Widget *, void *);
    static void BrowsePathCallback(  Fl_Widget *, void *);
};


} // end namespace fltk
