/////////////////////////////////////////////////////////////////////////////
// Name:        wxTimelineCtrlTestMainFrame.h
// Purpose:     
// Author:      Volodymyr (T-Rex) Triapichko
// Modified by: 
// Created:     27/04/2025 18:45:30
// RCS-ID:      
// Copyright:   Volodymyr (T-Rex) Triapichko, 2025
// Licence:     
/////////////////////////////////////////////////////////////////////////////
#ifndef _WXTIMELINECTRLTESTMAINFRAME_H_
#define _WXTIMELINECTRLTESTMAINFRAME_H_

/*!
 * Includes
 */
 ////@begin includes
#include "wx/frame.h"
#include "wx/statusbr.h"
////@end includes

#include "wxTimelineCtrl.h"
#include "SampleData.h"

/*!
 * Forward declarations
 */

 /*!
  * Control identifiers
  */
  ////@begin control identifiers
#define ID_WXTIMELINECTRLTESTMAINFRAME 10000
#define ID_PANEL 10003
#define ID_TIMELINE_CTRL 10004
#define ID_STATUSBAR 10001
#define SYMBOL_WXTIMELINECTRLTESTMAINFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_WXTIMELINECTRLTESTMAINFRAME_TITLE _("wxTimelineCtrl Test Main Frame")
#define SYMBOL_WXTIMELINECTRLTESTMAINFRAME_IDNAME ID_WXTIMELINECTRLTESTMAINFRAME
#define SYMBOL_WXTIMELINECTRLTESTMAINFRAME_SIZE wxSize(800, 500)
#define SYMBOL_WXTIMELINECTRLTESTMAINFRAME_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * wxTimelineCtrlTestMainFrame class declaration
 */
class wxTimelineCtrlTestMainFrame : public wxFrame
{
    DECLARE_CLASS(wxTimelineCtrlTestMainFrame)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    wxTimelineCtrlTestMainFrame();
    wxTimelineCtrlTestMainFrame(wxWindow* parent, wxWindowID id = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_IDNAME, const wxString& caption = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_TITLE, const wxPoint& pos = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_POSITION, const wxSize& size = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_SIZE, long style = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_STYLE);

    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_IDNAME, const wxString& caption = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_TITLE, const wxPoint& pos = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_POSITION, const wxSize& size = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_SIZE, long style = SYMBOL_WXTIMELINECTRLTESTMAINFRAME_STYLE);

    /// Destructor
    ~wxTimelineCtrlTestMainFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin wxTimelineCtrlTestMainFrame event handler declarations
    void OnAddClick(wxCommandEvent& event);
    void OnDeleteClick(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnTimelineItemSelected(wxCommandEvent& event);
    void OnTimelineItemDeleted(wxCommandEvent& event);
    ////@end wxTimelineCtrlTestMainFrame event handler declarations

    ////@begin wxTimelineCtrlTestMainFrame member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);
    ////@end wxTimelineCtrlTestMainFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Updates the state of UI elements
    void UpdateUI();

private:
    ////@begin wxTimelineCtrlTestMainFrame member variables
    wxTimelineCtrl<SampleData>* m_Timeline;
    ////@end wxTimelineCtrlTestMainFrame member variables

    SampleData** m_Data;
    size_t m_DataCount;
    size_t m_DataCapacity;
};

// Helper function to format time
wxString FormatTime(int seconds);

#endif
// _WXTIMELINECTRLTESTMAINFRAME_H_
