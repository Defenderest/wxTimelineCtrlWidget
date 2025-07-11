/////////////////////////////////////////////////////////////////////////////
// Name:        wxTimelineCtrlTestMainFrame.cpp
// Purpose:
// Author:      Volodymyr (T-Rex) Triapichko
// Modified by:
// Created:     27/04/2025 18:45:30
// RCS-ID:
// Copyright:   Volodymyr (T-Rex) Triapichko, 2025
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colourdata.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <wx/checklst.h>

#include "wxTimelineCtrlTestMainFrame.h"
#include "wxTimelineCtrl.h"

wxDEFINE_EVENT(wxEVT_TIMELINE_ZOOM, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_TIMELINE_SELECTION, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_TIMELINE_ITEM_DELETED, wxCommandEvent);

IMPLEMENT_CLASS(wxTimelineCtrlTestMainFrame, wxFrame)

BEGIN_EVENT_TABLE(wxTimelineCtrlTestMainFrame, wxFrame)

EVT_BUTTON(wxID_ADD, wxTimelineCtrlTestMainFrame::OnAddClick)
EVT_BUTTON(wxID_DELETE, wxTimelineCtrlTestMainFrame::OnDeleteClick)
EVT_MENU(wxID_EXIT, wxTimelineCtrlTestMainFrame::OnExit)
EVT_BUTTON(ID_TIMELINE_CTRL, wxTimelineCtrlTestMainFrame::OnTimelineItemSelected)
EVT_COMMAND(ID_TIMELINE_CTRL, wxEVT_TIMELINE_ITEM_DELETED, wxTimelineCtrlTestMainFrame::OnTimelineItemDeleted)

END_EVENT_TABLE()

wxTimelineCtrlTestMainFrame::wxTimelineCtrlTestMainFrame()
{
    Init();
}

wxTimelineCtrlTestMainFrame::wxTimelineCtrlTestMainFrame(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool wxTimelineCtrlTestMainFrame::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxFrame::Create(parent, id, caption, pos, size, style);

    CreateControls();
    Centre();
    return true;
}

wxTimelineCtrlTestMainFrame::~wxTimelineCtrlTestMainFrame()
{
    if (m_Data)
    {
        for (size_t i = 0; i < m_DataCount; ++i)
        {
            if (m_Data[i])
            {
                delete m_Data[i];
                m_Data[i] = nullptr;
            }
        }
        delete[] m_Data;
        m_Data = nullptr;
    }
}

void wxTimelineCtrlTestMainFrame::Init()
{
    m_Timeline = nullptr;

    m_Data = nullptr;
    m_DataCount = 0;
    m_DataCapacity = 0;
}

void wxTimelineCtrlTestMainFrame::CreateControls()
{
    wxTimelineCtrlTestMainFrame* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu10 = new wxMenu;
    itemMenu10->Append(wxID_EXIT, _("E&xit\tAlt+F4"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu10, _("File"));
    itemFrame1->SetMenuBar(menuBar);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer1);

    wxPanel* itemPanel2 = new wxPanel(itemFrame1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemPanel2->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    itemBoxSizer1->Add(itemPanel2, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    m_Timeline = new wxTimelineCtrl<SampleData>(itemPanel2, ID_TIMELINE_CTRL, wxDefaultPosition, wxSize(100, 150));
    itemBoxSizer3->Add(m_Timeline, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxGROW | wxALL, 0);

    wxButton* itemButton6 = new wxButton(itemPanel2, wxID_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer5->Add(itemButton6, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP | wxBOTTOM, 5);

    wxButton* itemButton7 = new wxButton(itemPanel2, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer5->Add(itemButton7, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStatusBar* itemStatusBar8 = new wxStatusBar(itemFrame1, ID_STATUSBAR, wxST_SIZEGRIP | wxNO_BORDER);
    itemStatusBar8->SetFieldsCount(2);
    itemFrame1->SetStatusBar(itemStatusBar8);

    m_DataCapacity = 10;
    m_Data = new SampleData * [m_DataCapacity];
    for (size_t i = 0; i < m_DataCapacity; ++i)
    {
        m_Data[i] = nullptr;
    }

    m_Timeline->SetTotalDuration(240);
    m_Timeline->SetVisibleDuration(160);
    m_Timeline->SetFirstVisibleTime(0);

    m_Data[0] = new SampleData(wxT("First"), 30, 45);
    m_Data[1] = new SampleData(wxT("Second"), 45, 90);
    m_Data[2] = new SampleData(wxT("Third"), 170, 240);
    m_DataCount = 3;

    m_Timeline->AddItem(m_Data[0]);
    m_Timeline->AddItem(m_Data[1]);
    m_Timeline->AddItem(m_Data[2]);

    UpdateUI();

    SetMinSize(wxSize(270, 250));
}

void wxTimelineCtrlTestMainFrame::OnAddClick(wxCommandEvent& )
{
    wxDialog dlg(this, wxID_ANY, _("Add Timeline Item"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(4, 2, 10, 10);
    gridSizer->AddGrowableCol(1);

    gridSizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Name:")), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    wxTextCtrl* nameCtrl = new wxTextCtrl(&dlg, wxID_ANY, _("New Item"));
    gridSizer->Add(nameCtrl, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Start Time:")), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    wxBoxSizer* startTimeSizer = new wxBoxSizer(wxHORIZONTAL);
    wxSpinCtrl* startMinutesCtrl = new wxSpinCtrl(&dlg, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 0, 999, 0);
    startTimeSizer->Add(startMinutesCtrl, 0, wxALIGN_CENTER_VERTICAL);
    startTimeSizer->Add(new wxStaticText(&dlg, wxID_ANY, _(" min ")), 0, wxALIGN_CENTER_VERTICAL);

    wxSpinCtrl* startSecondsCtrl = new wxSpinCtrl(&dlg, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 0, 59, 0);
    startTimeSizer->Add(startSecondsCtrl, 0, wxALIGN_CENTER_VERTICAL);
    startTimeSizer->Add(new wxStaticText(&dlg, wxID_ANY, _(" sec")), 0, wxALIGN_CENTER_VERTICAL);

    gridSizer->Add(startTimeSizer, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Duration:")), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    wxBoxSizer* durationSizer = new wxBoxSizer(wxHORIZONTAL);
    wxSpinCtrl* durationMinutesCtrl = new wxSpinCtrl(&dlg, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 0, 999, 0);
    durationSizer->Add(durationMinutesCtrl, 0, wxALIGN_CENTER_VERTICAL);
    durationSizer->Add(new wxStaticText(&dlg, wxID_ANY, _(" min ")), 0, wxALIGN_CENTER_VERTICAL);

    wxSpinCtrl* durationSecondsCtrl = new wxSpinCtrl(&dlg, wxID_ANY, "10", wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 1, 59, 10);
    durationSizer->Add(durationSecondsCtrl, 0, wxALIGN_CENTER_VERTICAL);
    durationSizer->Add(new wxStaticText(&dlg, wxID_ANY, _(" sec")), 0, wxALIGN_CENTER_VERTICAL);

    gridSizer->Add(durationSizer, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Color:")), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    wxColour suggestedColor = m_Timeline ? m_Timeline->GetItemColour(m_DataCount) : *wxBLUE;
    wxColourPickerCtrl* colorPicker = new wxColourPickerCtrl(&dlg, wxID_ANY, suggestedColor);
    gridSizer->Add(colorPicker, 1, wxEXPAND);

    mainSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 10);

    mainSizer->Add(dlg.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
        0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    dlg.SetSizerAndFit(mainSizer);
    dlg.CenterOnParent();

    if (dlg.ShowModal() == wxID_OK)
    {
        wxString nameValue = nameCtrl->GetValue();
        if (nameValue.IsEmpty())
        {
            nameValue = _("New Item");
        }

        int startTimeSeconds = startMinutesCtrl->GetValue() * 60 + startSecondsCtrl->GetValue();
        int durationValue = durationMinutesCtrl->GetValue() * 60 + durationSecondsCtrl->GetValue();

        if (durationValue <= 0)
        {
            wxMessageBox(_("Duration must be greater than 0 seconds."), _("Error"), wxOK | wxICON_ERROR, this);
            return;
        }

        int endTimeSeconds = startTimeSeconds + durationValue;

        if (m_DataCount >= m_DataCapacity)
        {
            size_t newCapacityValue = (m_DataCapacity == 0) ? 10 : m_DataCapacity * 2;
            SampleData** newDataArray = new SampleData * [newCapacityValue];

            for (size_t i = 0; i < m_DataCount; ++i)
            {
                newDataArray[i] = m_Data[i];
            }

            for (size_t i = m_DataCount; i < newCapacityValue; ++i)
            {
                newDataArray[i] = nullptr;
            }

            delete[] m_Data;
            m_Data = newDataArray;
            m_DataCapacity = newCapacityValue;
        }

        m_Data[m_DataCount] = new SampleData(nameValue, startTimeSeconds, endTimeSeconds);

        if (m_Timeline) {
            m_Timeline->AddItem(m_Data[m_DataCount], colorPicker->GetColour());
        }
        m_DataCount++;

        UpdateUI();
        if (m_Timeline) m_Timeline->Refresh();

        if (GetStatusBar())
        {
            GetStatusBar()->SetStatusText(wxString::Format(_("Added item '%s'"), nameValue.c_str()), 0);
        }
    }
}
void wxTimelineCtrlTestMainFrame::OnDeleteClick(wxCommandEvent& )
{
    wxDialog dlg(this, wxID_ANY, _("Select Items to Delete"), wxDefaultPosition, wxSize(400, 300),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(&dlg, wxID_ANY, _("Select items to delete:")),
        0, wxALL, 10);

    wxCheckListBox* itemList = new wxCheckListBox(&dlg, wxID_ANY);
    mainSizer->Add(itemList, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    for (size_t i = 0; i < m_DataCount; ++i)
    {
        if (m_Data[i] != nullptr)
        {
            wxString timeInfo = wxString::Format(_("%s - %s"),
                FormatTime(m_Data[i]->GetStartTime()),
                FormatTime(m_Data[i]->GetEndTime()));

            wxString itemText = wxString::Format("%s (%s)",
                m_Data[i]->GetName(),
                timeInfo);
            itemList->Append(itemText);

            const wxVector<size_t>& selectedItems = m_Timeline->GetSelectedItems();
            if (std::find(selectedItems.begin(), selectedItems.end(), i) != selectedItems.end())
            {
                itemList->Check(itemList->GetCount() - 1);
            }
        }
    }

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* selectAllBtn = new wxButton(&dlg, wxID_ANY, _("Select All"));
    wxButton* selectNoneBtn = new wxButton(&dlg, wxID_ANY, _("Select None"));
    buttonSizer->Add(selectAllBtn, 0, wxALL, 5);
    buttonSizer->Add(selectNoneBtn, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 5);

    mainSizer->Add(dlg.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
        0, wxEXPAND | wxALL, 10);

    dlg.SetSizer(mainSizer);

    selectAllBtn->Bind(wxEVT_BUTTON, [itemList](wxCommandEvent&) {
        for (unsigned int i = 0; i < itemList->GetCount(); ++i)
            itemList->Check(i);
        });

    selectNoneBtn->Bind(wxEVT_BUTTON, [itemList](wxCommandEvent&) {
        for (unsigned int i = 0; i < itemList->GetCount(); ++i)
            itemList->Check(i, false);
        });

    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayInt checkedItems;
        itemList->GetCheckedItems(checkedItems);

        if (checkedItems.IsEmpty())
        {
            wxMessageBox(_("No items selected for deletion."),
                _("Information"),
                wxOK | wxICON_INFORMATION, this);
            return;
        }

        wxVector<size_t> indicesToDelete;
        for (size_t i = 0; i < checkedItems.GetCount(); ++i)
        {
            indicesToDelete.push_back(checkedItems[i]);
        }
        std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<size_t>());

        int deletedCount = 0;
        for (size_t i = 0; i < indicesToDelete.size(); ++i)
        {
            size_t index = indicesToDelete[i];
            if (index < m_DataCount && m_Data[index] != nullptr)
            {
                m_Timeline->RemoveItem(m_Data[index]);

                delete m_Data[index];
                m_Data[index] = nullptr;

                for (size_t j = index; j < m_DataCount - 1; ++j)
                {
                    m_Data[j] = m_Data[j + 1];
                }

                m_Data[m_DataCount - 1] = nullptr;
                m_DataCount--;
                deletedCount++;
            }
        }

        m_Timeline->ClearSelection();
        m_Timeline->Refresh();
        UpdateUI();

        if (GetStatusBar())
        {
            if (deletedCount == 1)
                GetStatusBar()->SetStatusText(_("1 item deleted"), 0);
            else
                GetStatusBar()->SetStatusText(wxString::Format(_("%d items deleted"), deletedCount), 0);
        }
    }
}

void wxTimelineCtrlTestMainFrame::OnExit(wxCommandEvent& )
{
    Close(true);
}

bool wxTimelineCtrlTestMainFrame::ShowToolTips()
{
    return true;
}

wxBitmap wxTimelineCtrlTestMainFrame::GetBitmapResource(const wxString& name)
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon wxTimelineCtrlTestMainFrame::GetIconResource(const wxString& name)
{
    wxUnusedVar(name);
    return wxNullIcon;
}

void wxTimelineCtrlTestMainFrame::UpdateUI()
{
    wxWindow* deleteButton = FindWindow(wxID_DELETE);
    if (deleteButton)
    {
        bool hasItems = (m_DataCount > 0);
        deleteButton->Enable(hasItems);
    }

    if (GetStatusBar())
    {
        GetStatusBar()->SetStatusText(wxString::Format(_("Items: %zu"), m_DataCount), 1);
    }
}
void wxTimelineCtrlTestMainFrame::OnTimelineItemSelected(wxCommandEvent& )
{
    UpdateUI();

    if (GetStatusBar())
    {
        const wxVector<size_t>& selectedItems = m_Timeline->GetSelectedItems();
        if (selectedItems.size() == 1)
        {
            size_t index = selectedItems[0];
            if (index < m_DataCount && m_Data[index] != nullptr)
            {
                SampleData* data = m_Data[index];
                GetStatusBar()->SetStatusText(
                    wxString::Format(_("Selected: %s (%s - %s)"),
                        data->GetName(),
                        FormatTime(data->GetStartTime()),
                        FormatTime(data->GetEndTime())),
                    0);
            }
        }
        else if (selectedItems.size() > 1)
        {
            GetStatusBar()->SetStatusText(
                wxString::Format(_("%zu items selected"), selectedItems.size()),
                0);
        }
    }
}

wxString FormatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return wxString::Format("%d:%02d", minutes, secs);
}

void wxTimelineCtrlTestMainFrame::OnTimelineItemDeleted(wxCommandEvent& event)
{
    SampleData* dataToDelete = static_cast<SampleData*>(event.GetClientData());
    if (!dataToDelete)
    {
        wxLogWarning("OnTimelineItemDeleted: Received null data to delete.");
        return;
    }

    int foundIndex = -1;
    for (size_t i = 0; i < m_DataCount; ++i)
    {
        if (m_Data[i] == dataToDelete)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1)
    {
        wxString itemName = m_Data[foundIndex]->GetName();

        delete m_Data[foundIndex];
        m_Data[foundIndex] = nullptr;

        for (size_t i = static_cast<size_t>(foundIndex); i < m_DataCount - 1; ++i)
        {
            m_Data[i] = m_Data[i + 1];
        }
        m_Data[m_DataCount - 1] = nullptr;
        m_DataCount--;

        UpdateUI();

        if (GetStatusBar())
        {
            GetStatusBar()->SetStatusText(wxString::Format(_("Item '%s' deleted."), itemName.c_str()), 0);
        }
    }
    else
    {
        wxLogWarning("OnTimelineItemDeleted: Could not find SampleData to delete in m_Data array. Item pointer: %p", static_cast<void*>(dataToDelete));
        for (size_t i = 0; i < m_DataCount; ++i) {
            wxLogDebug("m_Data[%zu] = %p, Name: %s", i, static_cast<void*>(m_Data[i]), m_Data[i] ? m_Data[i]->GetName() : wxT("nullptr"));
        }
        UpdateUI();
    }
}