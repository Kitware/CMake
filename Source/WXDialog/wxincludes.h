#ifdef WIN32

#define __WIN32 
#define _WINDOWS
#define __WINDOWS__
#define __WXMSW__
#define __WIN32__
#define WINVER 0x0400
#define STRICT


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

#pragma hdrstop("wxincludes.pch")

#else


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

#endif

#undef FileExists
