#ifdef WIN32

#ifndef __WIN32
#  define __WIN32 
#endif 

#ifndef _WINDOWS
#  define _WINDOWS
#endif

#ifndef __WINDOWS__
#  define __WINDOWS__
#endif

#ifndef __WXMSW__
#  define __WXMSW__
#endif

#ifndef __WIN32__
#  define __WIN32__
#endif

#ifndef WINVER
#  define WINVER 0x0400
#endif

#ifndef STRICT
#  define STRICT
#endif


#include "wx/defs.h"

#include "wx/app.h"
#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/combobox.h"
#include "wx/config.h"
#include "wx/control.h"
#include "wx/dirdlg.h"
#include "wx/filedlg.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/timer.h"

#pragma hdrstop("wxincludes.pch")

#else


#include "wx/defs.h"

#include "wx/app.h"
#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/combobox.h"
#include "wx/config.h"
#include "wx/control.h"
#include "wx/dirdlg.h"
#include "wx/filedlg.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/timer.h"

#endif

#undef FileExists
