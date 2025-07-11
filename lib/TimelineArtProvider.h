#ifndef _TIMELINEARTPROVIDER_H
#define _TIMELINEARTPROVIDER_H

#include <wx/wx.h>
#include <wx/dcgraph.h>
#include "TimelineItem.h"

#if defined(__LINUX__)
#include <wx/dc.h>
#endif

class wxGraphicsGradientStops;

class TimelineArtProvider
{
    wxDECLARE_NO_COPY_CLASS(TimelineArtProvider);

public:
    TimelineArtProvider()
        : m_BackgroundColour(*wxWHITE),
        m_LimitTimeColour(*wxBLUE),
        m_Radius(5),
        m_MinScaleStep(50.0)
    {
        m_TimeScaleFont.Create(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial");
        m_ItemNameFont.Create(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Arial");
        m_PenTimelineMain = wxPen(wxColour(255, 255, 255, 90));
        m_PenScrollerMain = wxPen(wxColour(255, 255, 255, 120));
        m_PenVisibleFrameNormal = wxPen(wxColour(255, 255, 255, 96));
        m_PenVisibleFrameHover = wxPen(wxColour(255, 255, 255, 128));
        m_PenVisibleFramePressed = wxPen(wxColour(255, 255, 255, 160));
        m_PenGap = wxPen(*wxRED, 1, wxPENSTYLE_SHORT_DASH);

        CreateRoundRegions();
    }
    virtual ~TimelineArtProvider() {}

    virtual void DrawBackground(wxDC& dc, const wxRect& rect)
    {
        dc.GradientFillLinear(rect, wxColour(83, 83, 83), wxColour(163, 163, 163), wxDOWN);
    }
    virtual void DrawTimelineBackground(wxDC& dc, const wxRect& rect) { wxUnusedVar(dc);  wxUnusedVar(rect); }
    virtual void DrawTimelineTrack(wxDC& dc, const wxRect& rect)
    {
        dc.SetPen(m_PenTimelineMain);
        DrawGradientRoundedRect(dc, rect, m_Radius, wxGraphicsGradientStops(wxColour(55, 55, 55), wxColour(93, 93, 93)), wxDOWN);
    }
    virtual void DrawScrollerBackground(wxDC& dc, const wxRect& rect) { wxUnusedVar(dc); wxUnusedVar(rect); }
    virtual void DrawScrollerTrack(wxDC& dc, const wxRect& rect)
    {
        dc.SetPen(m_PenScrollerMain);
        DrawGradientRoundedRect(dc, rect, m_Radius, wxGraphicsGradientStops(wxColour(55, 55, 55), wxColour(93, 93, 93)), wxDOWN);
    }
    virtual void DrawGap(wxDC& dc, const wxRect& rTimeline, const wxRect& rVisibleFrame) {
        wxUnusedVar(dc);
		wxUnusedVar(rTimeline);
		wxUnusedVar(rVisibleFrame);
    }

    virtual void DrawLeftArrow(wxDC& dc, const wxRect& rect, TimelineElementState state)
    {
        wxPoint t = rect.GetTopRight();
        wxPoint b = rect.GetBottomRight();
        wxPoint c = rect.GetTopLeft();
        c.y += rect.height / 2;

        DrawArrow(dc, t, b, c, state);
    }
    virtual void DrawRightArrow(wxDC& dc, const wxRect& rect, TimelineElementState state)
    {
        wxPoint t = rect.GetTopLeft();
        wxPoint b = rect.GetBottomLeft();
        wxPoint c = rect.GetTopRight();
        c.y += rect.height / 2;

        DrawArrow(dc, t, b, c, state);
    }
    virtual void DrawVisibleFrame(wxDC& dc, const wxRect& rect, TimelineElementState state)
    {
        double koef =
            state == TimelineElementState::Normal ? 0.3 :
            state == TimelineElementState::Hover ? 0.5 :
            0.7;

        int desiredRadius = 11; 

        int maxPossibleRadiusForRect = (wxMin(rect.width, rect.height) - 2) / 2;
        int actualRadius = wxClip(desiredRadius, 0, maxPossibleRadiusForRect);
        
        dc.SetPen(wxColour(255, 255, 255, 255 * (koef + 0.1)));

        wxGraphicsGradientStops stops(wxColour(255, 255, 255, 192 * koef), wxColour(255, 255, 255, 128 * koef));
        stops.Add(wxColour(255, 255, 255, 128 * koef), 0.6f);
        stops.Add(wxColour(255, 255, 255, 76 * koef), 0.61f);
        DrawGradientRoundedRect(dc, rect, actualRadius, stops, wxDOWN);
    }

    void DrawTimeScale(wxDC& dc, const wxRect& rect,
        const wxDateTime& startTime, const wxDateTime& endTime)
    {
        if (rect.width <= 0 || rect.height <= 0)
            return;

        wxTimeSpan duration = endTime - startTime;
        long totalSeconds = duration.GetSeconds().ToLong();

        if (totalSeconds <= 0)
            return;

        dc.SetTextForeground(wxColour(80, 80, 80));
        dc.SetFont(m_TimeScaleFont);
        dc.SetPen(wxPen(wxColour(80, 80, 80), 1));

        wxString sampleText = "00:00:00";
        wxSize textSize = dc.GetTextExtent(sampleText);
        int textWidth = textSize.GetWidth();
        int textHeight = textSize.GetHeight();

        int minSpacing = textWidth + 20;

        int maxLabels = wxMax(2, rect.width / minSpacing);

        int interval = CalculateNiceInterval(totalSeconds, maxLabels);

        int numMarks = (totalSeconds / interval) + 1;

        for (int i = 0; i <= numMarks && i * interval <= totalSeconds; ++i)
        {
            long offsetSeconds = i * interval;
            if (offsetSeconds > totalSeconds)
                offsetSeconds = totalSeconds;

            double progress = double(offsetSeconds) / double(totalSeconds);
            int x = rect.x + int(progress * rect.width);

            wxDateTime markTime = startTime + wxTimeSpan::Seconds(offsetSeconds);
            wxString timeText = FormatTimeForScale(markTime);

            int tickHeight = 6;
            int tickX = x;
            dc.DrawLine(tickX, rect.y, tickX, rect.y + tickHeight);

            wxSize currentTextSize = dc.GetTextExtent(timeText);
            int textX, textY;

            if (i == 0)
            {
                textX = x + 2;
            }
            else if (offsetSeconds >= totalSeconds)
            {
                textX = x - currentTextSize.GetWidth() - 2;
            }
            else
            {
                textX = x - currentTextSize.GetWidth() / 2;
            }

            textY = rect.y + (rect.height - textHeight) / 2;

            textX = wxMax(rect.x, wxMin(textX, rect.GetRight() - currentTextSize.GetWidth()));

            dc.SetClippingRegion(rect);
            dc.DrawText(timeText, textX, textY);
            dc.DestroyClippingRegion();
        }
    }

    int CalculateNiceInterval(long totalSeconds, int maxLabels)
    {
        if (maxLabels <= 1)
            return totalSeconds;

        double roughInterval = double(totalSeconds) / double(maxLabels - 1);

        const int niceIntervals[] = {
            1, 2, 5, 10, 15, 30,
            60, 120, 300, 600, 900, 1800,
            3600, 7200, 10800, 21600, 43200,
            86400, 172800, 432000, 864000, 2592000
        };
        const int numIntervals = sizeof(niceIntervals) / sizeof(niceIntervals[0]);

        for (int i = 0; i < numIntervals; ++i)
        {
            if (niceIntervals[i] >= roughInterval)
            {
                return niceIntervals[i];
            }
        }

        int largestNice = niceIntervals[numIntervals - 1];
        int multiplier = (int(roughInterval) + largestNice - 1) / largestNice;
        return largestNice * multiplier;
    }

    wxString FormatTimeForScale(const wxDateTime& time)
    {
        return time.Format("%H:%M:%S");
    }

    template<typename T>
    void DrawItem(wxDC& dc, const wxRect& rect, const wxRect& parentRect, const TimelineItem<T>& item, bool isScrollerContext = false, bool makeTransparentDueToOverlap = false);

    void SetBackgroundColour(const wxColour& colour) { m_BackgroundColour = colour; }
    wxColour GetBackgroundColour() const { return m_BackgroundColour; }

protected:
    void CreateRoundRegions()
    {
        m_BmpRound = wxBitmap(m_Radius * 2, m_Radius * 2, 1);
        {
            wxMemoryDC dc(m_BmpRound);
            dc.SetBackground(*wxWHITE_BRUSH);
            dc.Clear();
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRoundedRectangle(0, 0, m_Radius * 2, m_Radius * 2, m_Radius);
        }
        m_RoundRegionTL = m_RoundRegionTR = m_RoundRegionBL = m_RoundRegionBR = wxRegion(m_BmpRound, *wxBLACK, 0);
        m_RoundRegionTR.Offset(-m_Radius, 0);
        m_RoundRegionBL.Offset(0, -m_Radius);
        m_RoundRegionBR.Offset(-m_Radius, -m_Radius);
        m_RoundRegionTL.Intersect(0, 0, m_Radius, m_Radius);
        m_RoundRegionTR.Intersect(0, 0, m_Radius, m_Radius);
        m_RoundRegionBL.Intersect(0, 0, m_Radius, m_Radius);
        m_RoundRegionBR.Intersect(0, 0, m_Radius, m_Radius);
    }

    wxRegion GetRoundRegion(const wxRect& rect)
    {
        wxRegion region(rect);
        wxRegion round;
        round = m_RoundRegionTL;
        round.Offset(rect.x, rect.y);
        region.Subtract(round);

        round = m_RoundRegionBL;
        round.Offset(rect.x, rect.y + rect.height - m_Radius);
        region.Subtract(round);

        round = m_RoundRegionTR;
        round.Offset(rect.x + rect.width - m_Radius, rect.y);
        region.Subtract(round);

        round = m_RoundRegionBR;
        round.Offset(rect.x + rect.width - m_Radius, rect.y + rect.height - m_Radius);
        region.Subtract(round);

        return region;
    }

    void DrawGradientRoundedRect(wxDC& dc, const wxRect& rect, double radius, const wxGraphicsGradientStops& stops, wxDirection direction = wxEAST)
    {
        wxGCDC* gdc = wxDynamicCast(&dc, wxGCDC);
        if (!gdc)
        {
            dc.SetDeviceClippingRegion(GetRoundRegion(rect));
            dc.GradientFillLinear(rect, stops.GetStartColour(), stops.GetEndColour(), direction);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(rect, radius);
            dc.DestroyClippingRegion();
            return;
        }

        int w = rect.width;
        int h = rect.height;

        if (w == 0 || h == 0)
            return;

        if (radius < 0.0)
            radius = -radius * ((w < h) ? w : h);

        wxPoint start;
        wxPoint end;
        switch (direction)
        {
        case wxWEST:
            start = rect.GetRightBottom();
            start.x++;
            end = rect.GetLeftBottom();
            break;
        case wxEAST:
            start = rect.GetLeftBottom();
            end = rect.GetRightBottom();
            end.x++;
            break;
        case wxNORTH:
            start = rect.GetLeftBottom();
            start.y++;
            end = rect.GetLeftTop();
            break;
        case wxSOUTH:
            start = rect.GetLeftTop();
            end = rect.GetLeftBottom();
            end.y++;
            break;
        default:
            break;
        }

        wxGraphicsContext* gc = gdc->GetGraphicsContext();

        if (gc->ShouldOffset())
        {
            w -= 1;
            h -= 1;
        }

        gc->SetBrush(gc->CreateLinearGradientBrush(start.x, start.y, end.x, end.y, stops));
        gc->DrawRoundedRectangle(rect.x, rect.y, w, h, radius);
    }

    void DrawArrow(wxDC& dc, wxPoint& top, wxPoint& bottom, wxPoint& center, TimelineElementState state)
    {
        wxColour penColor, brushColor, shadowColor;

        switch (state)
        {
        case TimelineElementState::Normal:
            penColor = wxColour(60, 60, 60);
            brushColor = wxColour(180, 180, 180, 220);
            shadowColor = wxColour(0, 0, 0, 80);
            break;
        case TimelineElementState::Hover:
            penColor = wxColour(40, 40, 40);
            brushColor = wxColour(240, 240, 240, 240);
            shadowColor = wxColour(0, 0, 0, 120);
            break;
        case TimelineElementState::Pressed:
            penColor = wxColour(20, 20, 20);
            brushColor = wxColour(255, 255, 255, 255);
            shadowColor = wxColour(0, 0, 0, 150);
            break;
        case TimelineElementState::Disabled:
            penColor = wxColour(120, 120, 120);
            brushColor = wxColour(160, 160, 160, 160);
            shadowColor = wxColour(0, 0, 0, 40);
            break;
        default:
            penColor = wxColour(60, 60, 60);
            brushColor = wxColour(180, 180, 180, 220);
            shadowColor = wxColour(0, 0, 0, 80);
            break;
        }

        wxPoint expandedTop = top;
        wxPoint expandedBottom = bottom;
        wxPoint expandedCenter = center;

        int deltaY = (bottom.y - top.y) / 6;
        expandedTop.y -= deltaY;
        expandedBottom.y += deltaY;

        int deltaX = abs(center.x - top.x) / 4;
        if (center.x > top.x)
            expandedCenter.x += deltaX;
        else
            expandedCenter.x -= deltaX;

        wxPoint arrowPoints[] = { expandedTop, expandedCenter, expandedBottom };

        if (state != TimelineElementState::Disabled)
        {
            wxPoint shadowPoints[] = {
                wxPoint(expandedTop.x + 1, expandedTop.y + 1),
                wxPoint(expandedCenter.x + 1, expandedCenter.y + 1),
                wxPoint(expandedBottom.x + 1, expandedBottom.y + 1)
            };

            dc.SetBrush(wxBrush(shadowColor));
            dc.SetPen(wxPen(shadowColor));
            dc.DrawPolygon(WXSIZEOF(shadowPoints), shadowPoints);
        }

        dc.SetBrush(wxBrush(brushColor));
        dc.SetPen(wxPen(brushColor));
        dc.DrawPolygon(WXSIZEOF(arrowPoints), arrowPoints);

        dc.SetPen(wxPen(penColor, state == TimelineElementState::Pressed ? 2 : 1));
        dc.DrawLines(WXSIZEOF(arrowPoints), arrowPoints);

        if (state == TimelineElementState::Hover || state == TimelineElementState::Pressed)
        {
            wxColour highlightColor = wxColour(255, 255, 255, 100);
            dc.SetPen(wxPen(highlightColor, 1));

            if (center.x > top.x)
            {
                dc.DrawLine(expandedTop.x - 1, expandedTop.y + 1,
                    expandedCenter.x - 1, expandedCenter.y);
            }
            else
            {
                dc.DrawLine(expandedTop.x + 1, expandedTop.y + 1,
                    expandedCenter.x + 1, expandedCenter.y);
            }
        }
    }

protected:
    wxColour m_BackgroundColour;
    wxColour m_LimitTimeColour;

    wxFont m_TimeScaleFont;
    wxFont m_ItemNameFont;

    wxPen m_PenTimelineMain;
    wxPen m_PenScrollerMain;
    wxPen m_PenVisibleFrameNormal;
    wxPen m_PenVisibleFrameHover;
    wxPen m_PenVisibleFramePressed;
    wxPen m_PenGap;

    wxBitmap m_BmpRound;
    wxRegion m_RoundRegionTL, m_RoundRegionTR, m_RoundRegionBL, m_RoundRegionBR;

    int m_Radius;
    double m_MinScaleStep;
};

template<typename T>
void TimelineArtProvider::DrawItem(wxDC& dc, const wxRect& rect, const wxRect& parentRect, const TimelineItem<T>& item, bool isScrollerContext, bool makeTransparentDueToOverlap)
{
    if (!item.Data)
        return;

    wxRect r(rect.Intersect(parentRect));
    if (r.IsEmpty())
        return;

    wxColour currentItemColor = item.Colour; 

    switch (item.State)
    {
    case TimelineElementState::Hover:
        currentItemColor = currentItemColor.ChangeLightness(130);
        break;
    case TimelineElementState::Pressed:
        currentItemColor = currentItemColor.ChangeLightness(150);
        break;
    case TimelineElementState::Disabled:
        currentItemColor = currentItemColor.ChangeLightness(90);
        break;
    default:
        break;
    }

    unsigned char finalAlpha = currentItemColor.Alpha(); 
    if (makeTransparentDueToOverlap)
    {
        if (isScrollerContext)
        {
            finalAlpha = 100; 
        }
        else
        {
            finalAlpha = 180;
        }
    }
    
    wxColour baseColor(currentItemColor.Red(), currentItemColor.Green(), currentItemColor.Blue(), finalAlpha);
    
    wxColour gradColor = baseColor.ChangeLightness(133); 
    gradColor.Set(gradColor.Red(), gradColor.Green(), gradColor.Blue(), finalAlpha);

    int radius = wxClip(m_Radius, 0, (r.width - 2) / 2);

    wxGCDC* gdc = wxDynamicCast(&dc, wxGCDC);
    if (gdc)
    {
        wxGraphicsContext* gc = gdc->GetGraphicsContext();
        if (gc)
        {
            wxGraphicsGradientStops stops(baseColor, gradColor);
            wxGraphicsBrush brush = gc->CreateLinearGradientBrush(
                r.x, r.GetBottom(), r.GetRight(), r.GetBottom(), stops);
            gc->SetBrush(brush);
            gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
        }
    }
    else
    {
        dc.SetDeviceClippingRegion(GetRoundRegion(r));
        dc.GradientFillLinear(r, gradColor, baseColor, wxDOWN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRoundedRectangle(r, radius);
        dc.DestroyClippingRegion();
    }

    wxString name = item.GetItemName();
    if (!name.IsEmpty())
    {
        wxRect textRect = r;
        textRect.Deflate(isScrollerContext ? 2 : 5, 0); 

        wxFont fontToUse = m_ItemNameFont;
        if (isScrollerContext)
        {
            fontToUse = wxFont(wxFontInfo(9).Family(wxFONTFAMILY_SWISS).Weight(wxFONTWEIGHT_NORMAL));
        }
        else
        {
            fontToUse = wxFont(wxFontInfo(12)
                               .Family(m_ItemNameFont.GetFamily())
                               .Weight(m_ItemNameFont.GetWeight())
                               .Style(m_ItemNameFont.GetStyle())
                               .Underlined(m_ItemNameFont.GetUnderlined())
                               .FaceName(m_ItemNameFont.GetFaceName()));
        }
        dc.SetFont(fontToUse);

        dc.SetTextForeground(*wxBLACK); 

        wxSize textSize = dc.GetTextExtent(name);

        bool canDrawText = (textRect.width >= textSize.x && textRect.height >= textSize.y);
        if (isScrollerContext && r.width < 10) 
        {
            canDrawText = false;
        }
        
        if (canDrawText)
        {
            textRect = textRect.Intersect(r);
            if (!textRect.IsEmpty())
            {
                 wxDCClipper clip(dc, textRect);
                 dc.DrawLabel(name, textRect, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
            }
        }
    }
}

#endif // _TIMELINEARTPROVIDER_H
