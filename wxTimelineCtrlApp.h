/////////////////////////////////////////////////////////////////////////////
// Name:        wxTimelineCtrlApp.h
// Purpose:     
// Author:      Volodymyr (T-Rex) Triapichko
// Modified by: 
// Created:     27/04/2025 18:44:25
// RCS-ID:      
// Copyright:   Volodymyr (T-Rex) Triapichko, 2025
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXTIMELINECTRLAPP_H_
#define _WXTIMELINECTRLAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "wxTimelineCtrlTestMainFrame.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * wxTimelineCtrlApp class declaration
 */

class wxTimelineCtrlApp: public wxApp
{    
    DECLARE_CLASS( wxTimelineCtrlApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    wxTimelineCtrlApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin wxTimelineCtrlApp event handler declarations

////@end wxTimelineCtrlApp event handler declarations

////@begin wxTimelineCtrlApp member function declarations

////@end wxTimelineCtrlApp member function declarations

////@begin wxTimelineCtrlApp member variables
////@end wxTimelineCtrlApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(wxTimelineCtrlApp)
////@end declare app

#endif
    // _WXTIMELINECTRLAPP_H_