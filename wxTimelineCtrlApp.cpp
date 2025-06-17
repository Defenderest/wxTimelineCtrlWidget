/////////////////////////////////////////////////////////////////////////////
// Name:        wxTimelineCtrlApp.cpp
// Purpose:     
// Author:      Volodymyr (T-Rex) Triapichko
// Modified by: 
// Created:     27/04/2025 18:44:25
// RCS-ID:      
// Copyright:   Volodymyr (T-Rex) Triapichko, 2025
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "wxTimelineCtrlApp.h"

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( wxTimelineCtrlApp )
////@end implement app


/*
 * wxTimelineCtrlApp type definition
 */

IMPLEMENT_CLASS( wxTimelineCtrlApp, wxApp )


/*
 * wxTimelineCtrlApp event table definition
 */

BEGIN_EVENT_TABLE( wxTimelineCtrlApp, wxApp )

////@begin wxTimelineCtrlApp event table entries
////@end wxTimelineCtrlApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for wxTimelineCtrlApp
 */

wxTimelineCtrlApp::wxTimelineCtrlApp()
{
    Init();
}


/*
 * Member initialisation
 */

void wxTimelineCtrlApp::Init()
{
////@begin wxTimelineCtrlApp member initialisation
////@end wxTimelineCtrlApp member initialisation
}

/*
 * Initialisation for wxTimelineCtrlApp
 */

bool wxTimelineCtrlApp::OnInit()
{    
////@begin wxTimelineCtrlApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	wxTimelineCtrlTestMainFrame* mainWindow = new wxTimelineCtrlTestMainFrame( NULL );
	mainWindow->Show(true);
////@end wxTimelineCtrlApp initialisation

    return true;
}


/*
 * Cleanup for wxTimelineCtrlApp
 */

int wxTimelineCtrlApp::OnExit()
{    
////@begin wxTimelineCtrlApp cleanup
	return wxApp::OnExit();
////@end wxTimelineCtrlApp cleanup
}