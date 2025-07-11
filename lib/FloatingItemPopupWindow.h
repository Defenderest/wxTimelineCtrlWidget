#ifndef FLOATINGITEMPOPUPWINDOW_H
#define FLOATINGITEMPOPUPWINDOW_H

#include <wx/wx.h>
#include <wx/popupwin.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

#include "TimelineItem.h"
#include "TimelineArtProvider.h"

template<typename T>
class FloatingItemPopupWindow : public wxPopupWindow
{
public:
    FloatingItemPopupWindow(wxWindow* parent,
        const TimelineItem<T>& itemToDraw,
        const wxSize& size,
        TimelineArtProvider* artProvider)
        : wxPopupWindow(),
        m_itemVisual(itemToDraw),
        m_itemDrawSize(size),
        m_artProviderRef(artProvider)
    {
        SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);

        if (!wxPopupWindow::Create(parent, wxBORDER_NONE))
        {
            wxLogError("FloatingItemPopupWindow: Failed to create the window.");
            return;
        }

        wxSize finalSize = size;
        if (!finalSize.IsFullySpecified() || finalSize.x <= 0 || finalSize.y <= 0) {
            finalSize.Set(50, 20);
        }

        m_itemDrawSize = finalSize;
        SetSize(m_itemDrawSize);

        if (m_itemDrawSize.x > 0 && m_itemDrawSize.y > 0) {
            if (!m_buffer.Create(m_itemDrawSize, 32)) {
                wxLogError("FloatingItemPopupWindow Ctor: Failed to create m_buffer with size %d x %d", m_itemDrawSize.x, m_itemDrawSize.y);
            }
        }
        else {
            wxSize emergencySize(30, 15);
            if (!m_buffer.Create(emergencySize, 32)) {
                wxLogError("FloatingItemPopupWindow Ctor: Failed to create emergency m_buffer.");
            }
        }
        Bind(wxEVT_PAINT, &FloatingItemPopupWindow<T>::OnPaint, this);
    }

    ~FloatingItemPopupWindow()
    {
    }

    void OnPaint(wxPaintEvent& )
    {
        if (!m_itemVisual.Data || !m_artProviderRef || !m_buffer.IsOk()) {
            wxPaintDC dc(this);
            dc.SetBackground(*wxTRANSPARENT_BRUSH);
            dc.Clear();
            return;
        }

        wxBufferedPaintDC dc(this, m_buffer);
        wxGCDC gdc(dc);

        wxGraphicsContext* gc = gdc.GetGraphicsContext();
        if (gc)
        {
            gc->SetBrush(*wxTRANSPARENT_BRUSH);
            wxSize clientSize = GetClientSize();
            gc->DrawRectangle(0, 0, clientSize.GetWidth(), clientSize.GetHeight());
        }

        TimelineItem<T> itemToRender = m_itemVisual;
        itemToRender.Colour = itemToRender.Colour.ChangeLightness(110);
        wxColour c = itemToRender.Colour;
        itemToRender.Colour.Set(c.Red(), c.Green(), c.Blue(), 220);
        itemToRender.State = TimelineElementState::Pressed;

        wxRect drawRect(0, 0, m_itemDrawSize.GetWidth(), m_itemDrawSize.GetHeight());

        gdc.SetPen(wxPen(itemToRender.Colour, 0, wxPENSTYLE_TRANSPARENT));
        gdc.SetBrush(wxBrush(itemToRender.Colour));

        gdc.DrawRectangle(drawRect);

        m_artProviderRef->DrawItem(gdc, drawRect,
            drawRect,
            itemToRender,
            false,
            false);
    }

private:
    TimelineItem<T> m_itemVisual;
    wxSize m_itemDrawSize;
    TimelineArtProvider* m_artProviderRef;
    wxBitmap m_buffer;
};

#endif // FLOATINGITEMPOPUPWINDOW_H

    
