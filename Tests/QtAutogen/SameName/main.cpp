#include "aaa/bbb/item.hpp"
#include "aaa/item.hpp"
#include "bbb/aaa/item.hpp"
#include "bbb/item.hpp"
#include "ccc/item.hpp"
#include "item.hpp"
#include "object.h"
#include "object.h++"
#include "object.hh"
#include "object.hpp"
#include "object.hxx"
#include "object_upper_ext.H"

int main(int argv, char** args)
{
  // Item instances
  ::Item item;
  ::aaa::Item aaa_item;
  ::aaa::bbb::Item aaa_bbb_item;
  ::bbb::Item bbb_item;
  ::bbb::aaa::Item bbb_aaa_item;
  ::ccc::Item ccc_item;
  // Object instances
  ::Object_h obj_h;
  ::Object_hh obj_hh;
  ::Object_hplpl obj_hplpl;
  ::Object_hpp obj_hpp;
  ::Object_hxx obj_hxx;
  ::Object_Upper_Ext_H obj_upper_ext_h;
  return 0;
}
