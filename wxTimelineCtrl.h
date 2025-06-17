#ifndef _WX_TIMELINE_CTRL_H
#define _WX_TIMELINE_CTRL_H

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/vector.h>
#include <wx/timer.h>
#include "TimelineArtProvider.h"
#include "TimelineItem.h"
#include <set>
#include <utility> 
#include <algorithm> 
#include <numeric> 
#include "FloatingItemPopupWindow.h"

template<typename Tval>
Tval wxClip(Tval value, Tval min_val, Tval max_val) 
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

wxDECLARE_EVENT(wxEVT_TIMELINE_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_TIMELINE_SELECTION, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_TIMELINE_ITEM_DELETED, wxCommandEvent);

enum
{
    ID_TIMELINE_DELETE = wxID_HIGHEST + 1000,
    ID_TIMELINE_DELETE_SCROLLER_ITEM
};

// --- Template Timeline Control ---
template<typename T>
class wxTimelineCtrl : public wxControl
{
public:
    using TimelineItemVector = wxVector< TimelineItem<T> >;
    static const int MAX_ITEMS = 65536;

    enum ElementType
    {
        ET_NONE = 0,
        ET_TIMELINE,
        ET_SCROLLER,
        ET_VISIBLE_FRAME,
        ET_VISIBLE_FRAME_LEFT,
        ET_VISIBLE_FRAME_RIGHT,
        ET_LEFT_ARROW,
        ET_RIGHT_ARROW,
        ET_SCROLLER_ITEM_DRAG,
        ET_TIMELINE_ITEM,
        ET_TIMELINE_ITEM_MAX = ET_TIMELINE_ITEM + MAX_ITEMS * 3,
        ET_TIMELINE_SELECTION,
        ET_MAX
    };

public:
    wxTimelineCtrl() { Init(); }
    wxTimelineCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
    {
        Init();
        Create(parent, id, pos, size);
    }

    virtual ~wxTimelineCtrl()
    {
        if (m_timerMove.IsRunning())
            m_timerMove.Stop();
        delete m_artProvider;
    }

    bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
    {
        if (!wxControl::Create(parent, id, pos, size, wxWANTS_CHARS | wxNO_BORDER))
            return false;
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetEventHandlers();
        m_artProvider->SetBackgroundColour(GetBackgroundColour());
        RecalcRects();
        return true;
    }

    void Init();
    
    void ZoomToSelection()
    {
        if (!m_selectionRect.IsEmpty() && m_selectionRect.width > 5)
        {
            int startTime = m_firstVisibleTime + TimelineCoordToTime(m_selectionRect.GetLeft() - m_rectTimelineTrack.x);
            int endTime = m_firstVisibleTime + TimelineCoordToTime(m_selectionRect.GetRight() - m_rectTimelineTrack.x);
            
            if (startTime < 0) startTime = 0;
            if (endTime > m_totalDuration) endTime = m_totalDuration;
            if (endTime <= startTime) endTime = startTime + m_minVisibleDuration;
            
            int newDuration = endTime - startTime;
            
            wxLogDebug("ZoomToSelection: startTime=%d, endTime=%d, newDuration=%d", 
                       startTime, endTime, newDuration);
            
            if (newDuration >= m_minVisibleDuration)
            {
                int padding = newDuration / 20;
                if (padding < 1) padding = 1;
                
                startTime = wxMax(0, startTime - padding);
                endTime = wxMin(m_totalDuration, endTime + padding);
                
                SetVisibleTimeRange(startTime, endTime);
                
                ClearSelection();
                
                wxCommandEvent event(wxEVT_TIMELINE_ZOOM, GetId());
                event.SetEventObject(this);
                event.SetInt(m_visibleDuration);
                ProcessWindowEvent(event);
                
                wxLogDebug("Zoom applied: firstVisible=%d, duration=%d", 
                          m_firstVisibleTime, m_visibleDuration);
            }
        }
    }
    
    void ZoomToTimePoint(int timePoint, int zoomLevel = -1)
    {
        if (zoomLevel < 0)
            zoomLevel = m_visibleDuration / 2;
        else
            zoomLevel = wxClip(zoomLevel, m_minVisibleDuration, m_maxVisibleDuration);
        
        timePoint = wxClip(timePoint, 0, m_totalDuration);
        
        int halfZoom = zoomLevel / 2;
        int startTime = wxMax(0, timePoint - halfZoom);
        int endTime = wxMin(m_totalDuration, timePoint + halfZoom);
        
        SetVisibleTimeRange(startTime, endTime);
        
        wxLogDebug("ZoomToTimePoint: timePoint=%d, zoomLevel=%d, visible=%d-%d", 
                  timePoint, zoomLevel, startTime, endTime);
                  
        wxCommandEvent event(wxEVT_TIMELINE_ZOOM, GetId());
        event.SetEventObject(this);
        event.SetInt(m_visibleDuration);
        ProcessWindowEvent(event);
    }
    
    void ClearSelection()
    {
        m_isSelecting = false;
        m_selectionRect = wxRect();
        m_selectedItems.clear();
        m_contextMenuItemIndex = -1;
        Refresh();
    }
    
    // Get selected items indices
    const wxVector<size_t>& GetSelectedItems() const
    {
        return m_selectedItems;
    }
    
    // Check if an item is selected
    bool IsItemSelected(size_t index) const
    {
        return std::find(m_selectedItems.begin(), m_selectedItems.end(), index) != m_selectedItems.end();
    }
    
    // Toggle item selection
    void ToggleItemSelection(size_t index)
    {
        auto it = std::find(m_selectedItems.begin(), m_selectedItems.end(), index);
        if (it != m_selectedItems.end())
            m_selectedItems.erase(it);
        else
            m_selectedItems.push_back(index);
        Refresh();
    }
    
    // Select an item
    void SelectItem(size_t index, bool clearPrevious = true)
    {
        wxLogDebug("SelectItem: index=%d, clearPrevious=%d", (int)index, clearPrevious);
        
        if (index >= m_items.size())
        {
            wxLogDebug("SelectItem: index %d out of range (size=%d)", (int)index, (int)m_items.size());
            return;
        }
        
        if (clearPrevious)
        {
            wxLogDebug("SelectItem: clearing previous selection");
            m_selectedItems.clear();
        }
            
        if (!IsItemSelected(index))
        {
            wxLogDebug("SelectItem: adding index %d to selection", (int)index);
            m_selectedItems.push_back(index);
            wxLogDebug("SelectItem: selection size now %d", (int)m_selectedItems.size());
        }
        
        wxLogDebug("Current selection after SelectItem:");
        for (size_t i = 0; i < m_selectedItems.size(); ++i) {
            wxLogDebug("  Selected item %d: index=%d", (int)i, (int)m_selectedItems[i]);
        }
            
        Refresh();
    }
    
    // Remove selected items
    void RemoveSelectedItems();
    void RemoveContextScrollerItem();
    
    int GetZoomLevel() const
    {
        return m_visibleDuration;
    }
    
    void SetZoomLevel(int zoomLevelSeconds)
    {
        zoomLevelSeconds = wxClip(zoomLevelSeconds, m_minVisibleDuration, m_maxVisibleDuration);
        
        if (zoomLevelSeconds == m_visibleDuration)
            return;
            
        int centerTime = m_firstVisibleTime + m_visibleDuration / 2;
        ZoomAtPosition(m_visibleDuration - zoomLevelSeconds, centerTime);
        
        wxCommandEvent event(wxEVT_TIMELINE_ZOOM, GetId());
        event.SetEventObject(this);
        event.SetInt(m_visibleDuration);
        ProcessWindowEvent(event);
    }
    
    void ShowAllTimeline()
    {
        SetVisibleTimeRange(0, m_totalDuration);
        
        wxLogDebug("ShowAllTimeline: firstVisible=%d, duration=%d", 
                  m_firstVisibleTime, m_visibleDuration);
    }

    // Add an item with a specific color
    void AddItem(T* data, const wxColour& colour = wxNullColour);
    
    // Set color for a specific item
    bool SetItemColor(size_t index, const wxColour& colour)
    {
        if (index >= m_items.size() || !colour.IsOk())
            return false;
            
        m_items[index].SetColour(colour);
        Refresh();
        return true;
    }
    
    // Set color for a specific item by data pointer
    bool SetItemColor(T* data, const wxColour& colour)
    {
        if (!data || !colour.IsOk())
            return false;
            
        auto it = FindItem(data);
        if (it != m_items.end())
        {
            it->SetColour(colour);
            Refresh();
            return true;
        }
        return false;
    }
    
    // Updates the total timeline duration based on added items
    void UpdateTotalDurationForItems()
    {
        if (m_items.empty()) {
            if (m_totalDuration != m_minVisibleDuration) {
                SetTotalDuration(m_minVisibleDuration);
            }
            return;
        }
        
        int maxEndTime = 0;
        
        for (auto& item : m_items)
        {
            if (item.Data)
            {
                int itemEndTime = item.Data->GetEndTime();
                if (itemEndTime > maxEndTime)
                    maxEndTime = itemEndTime;
            }
        }
        
        int newDuration = maxEndTime;
        
        if (newDuration != m_totalDuration)
        {
            SetTotalDuration(newDuration);
        }
    }

    void RemoveItem(T* data)
    {
        for (auto it = m_items.begin(); it != m_items.end(); ++it)
        {
            if (it->Data == data)
            {
                m_items.erase(it);
                break;
            }
        }
        
        UpdateTotalDurationForItems();
        
        if (m_firstVisibleTime + m_visibleDuration > m_totalDuration)
        {
            SetFirstVisibleTime(wxMax(0, m_totalDuration - m_visibleDuration));
        }
        
        TimeChanged();
        Refresh();
    }

    void ClearItems()
    {
        m_items.clear();
        TimeChanged();
        Refresh();
    }

    void SwapItems(T* item1, T* item2)
    {
        auto it1 = FindItem(item1);
        auto it2 = FindItem(item2);
        if (it1 != m_items.end() && it2 != m_items.end())
            std::swap(*it1, *it2);
        RecalcItems();
        Refresh();
    }

    wxColour GetItemColour(int index)
    {
        wxImage::HSVValue hsv(0, 1, 1);
        double dummy;
        hsv.hue = modf(index * 0.07, &dummy);

        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        wxColour color(rgb.red, rgb.green, rgb.blue);
        return color.ChangeLightness(90);
    }
    
    // Get the color of a specific item
    wxColour GetItemColor(size_t index) const
    {
        if (index < m_items.size())
            return m_items[index].Colour;
        return wxNullColour;
    }
    
    // Get the color of a specific item by data pointer
    wxColour GetItemColor(T* data) const
    {
        if (!data)
            return wxNullColour;
            
        for (const auto& item : m_items)
        {
            if (item.Data == data)
                return item.Colour;
        }
        return wxNullColour;
    }

    void SetStartTime(const wxDateTime& val) { m_startTime = val; }
    wxDateTime GetStartTime() const { return m_startTime; }

    void SetDefaultDuration(int seconds) { m_defaultDuration = seconds; }
    int GetDefaultDuration() const { return m_defaultDuration; }

    void SetTotalDuration(int seconds);
    int GetTotalDuration() const { return m_totalDuration; }

    void SetFirstVisibleTime(int seconds);


    int GetFirstVisibleTime() const { return m_firstVisibleTime; }
    int GetLastVisibleTime() const { return m_firstVisibleTime + m_visibleDuration; }
    
    void SetVisibleTimeRange(int startSeconds, int endSeconds)
    {
        if (startSeconds >= endSeconds)
            return;
            
        int newDuration = endSeconds - startSeconds;
        if (newDuration < m_minVisibleDuration)
            newDuration = m_minVisibleDuration;
            
        SetVisibleDuration(newDuration);
        
        SetFirstVisibleTime(startSeconds);
        
        RecalcVisibleFrame();
        RecalcItems();
        Refresh();
    }

    void SetVisibleDuration(int seconds);

    int GetVisibleDuration() const { return m_visibleDuration; }

    void ZoomAtPosition(int deltaSeconds, int fixedTimePosition = -1)
    {
        if (fixedTimePosition < 0)
            fixedTimePosition = m_firstVisibleTime + m_visibleDuration / 2;
            
        double relativePos = 0.5; 
        if (fixedTimePosition >= m_firstVisibleTime && fixedTimePosition <= GetLastVisibleTime())
            relativePos = (double)(fixedTimePosition - m_firstVisibleTime) / m_visibleDuration;
            
        int newDuration = m_visibleDuration - deltaSeconds;
        newDuration = wxClip(newDuration, m_minVisibleDuration, m_maxVisibleDuration);
        
        if (newDuration == m_visibleDuration)
            return;
            
        int newFirstVisible = fixedTimePosition - wxRound(relativePos * newDuration);
    
        m_visibleDuration = newDuration;
    
        if (newFirstVisible < 0)
            newFirstVisible = 0;
        if (newFirstVisible + m_visibleDuration > m_totalDuration)
            newFirstVisible = m_totalDuration - m_visibleDuration;
            
        m_firstVisibleTime = newFirstVisible;
        
        CalcArrowsState();
        RecalcVisibleFrame();
        RecalcItems();
        Refresh();
        
        wxCommandEvent event(wxEVT_TIMELINE_ZOOM, GetId());
        event.SetEventObject(this);
        event.SetInt(m_visibleDuration);
        ProcessWindowEvent(event);
    }
    
    void Zoom(int deltaSeconds)
    {
        if (abs(deltaSeconds) == 1)
            SetVisibleDuration(GetVisibleDuration() + deltaSeconds);
        else
        {
            ZoomAtPosition(deltaSeconds);
        }
    }
    
    enum ZoomPreset {
        ZOOM_SECONDS_10 = 10,
        ZOOM_SECONDS_30 = 30,
        ZOOM_MINUTE_1 = 60, 
        ZOOM_MINUTES_2 = 120,
        ZOOM_MINUTES_5 = 300,
        ZOOM_MINUTES_10 = 600, 
        ZOOM_ALL = -1 
    };
    
    void SetZoomPreset(ZoomPreset preset)
    {
        if (preset == ZOOM_ALL)
        {
            ShowAllTimeline();
            return;
        }
        
        int centerTime = m_firstVisibleTime + m_visibleDuration / 2;
        
        SetZoomLevel(preset);
        
        CenterOnTime(centerTime);
    }

    void CenterOnTime(int seconds)
    {
        SetFirstVisibleTime(seconds - m_visibleDuration / 2);
    }

    int ScrollerTimeToCoord(int time);

    int TimelineTimeToCoord(int time)
    {
        return wxRound((double)time * m_rectTimelineTrack.width / m_visibleDuration);
    }

    int ScrollerCoordToTime(int coord) const;

    int TimelineCoordToTime(int coord)
    {
        if (m_visibleDuration <= 0 || m_rectTimelineTrack.width <= 0)
                return 0;
        
        return (coord * m_visibleDuration) / m_rectTimelineTrack.width;
    }

    int ClampFirstVisibleTime(int first) const;

protected:
    // Drawing
    void OnPaint(wxPaintEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt);
    void Draw(wxDC& dc);
    void DrawTimeline(wxDC& dc);
    void DrawScroller(wxDC& dc);

    // Mouse & Key Events
    void SetEventHandlers();
    void OnMouse(wxMouseEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnTimer(wxTimerEvent& evt);
    void OnKeyDown(wxKeyEvent& evt);

    void OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);

    // Logic
    void RecalcRects();
    void RecalcVisibleFrame();
    void RecalcItems();
    void CalcMaxVisibleDuration();
    void CalcArrowsState();
    void TimeChanged();

    // Elements
    ElementType GetElementFromPos(const wxPoint& pos);
    wxCursor GetCursorFromType(ElementType type);
    void ChangeLastElement(ElementType type);
    void OnMouseDown(ElementType type, const wxPoint& pos);
    void OnMouseUp(ElementType type, const wxPoint& pos);
    void OnMouseDrag(ElementType type, const wxPoint& pos);
    void OnLeaveElement(ElementType type);
    void OnEnterElement(ElementType type);

    void OnTimelineDown(const wxPoint& pos, ElementType type);
    void OnTimelineUp(const wxPoint& pos, ElementType type);
    void OnTimelineDrag(const wxPoint& pos, ElementType type);

    void OnVisibleFrameDown(const wxPoint& pos, ElementType type);
    void OnVisibleFrameUp(const wxPoint& pos);
    void OnVisibleFrameDrag(const wxPoint& pos);

    void OnScrollerDown(const wxPoint& pos);
    void OnArrowDown(bool isLeft);
    void OnArrowUp(bool isLeft);

    void OnTimelineItemMove(const wxPoint& pos);
    void OnTimelineItemChangeLeft(const wxPoint& pos);
    void OnTimelineItemChangeRight(const wxPoint& pos);

    void ShowContextMenu(const wxPoint& pos);
    void SendItemEvent(wxEventType evtType, int index);

    typename TimelineItemVector::iterator FindItem(T* data)
    {
        for (auto it = m_items.begin(); it != m_items.end(); ++it)
        {
            if (it->Data == data)
                return it;
        }
        return m_items.end();
    }

    void AdjustMainViewToScrollerView();

private:
    TimelineArtProvider* m_artProvider;

    TimelineItemVector m_items;

    // Active task being dragged/hovered
    typename TimelineItemVector::iterator m_activeTask;
    typename TimelineItemVector::iterator m_lastTask;
    typename TimelineItemVector::iterator m_visibleItemBegin;
    typename TimelineItemVector::iterator m_visibleItemEnd;
    
    // Selected items
    wxVector<size_t> m_selectedItems;

    wxPoint m_dragStartPos;
    wxPoint m_dragCurrentPos;
    wxPoint m_ptStartPos;
    wxPoint m_ptEndPos;
    
    bool m_isSelecting;
    wxPoint m_selectionStart;
    wxPoint m_selectionEnd;
    wxRect m_selectionRect;
    
    wxString FormatTime(int seconds) const
    {
        int minutes = seconds / 60;
        int secs = seconds % 60;
        return wxString::Format("%d:%02d", minutes, secs);
    }

    wxBitmap m_buffer;

    wxTimer m_timerMove;

    int m_totalDuration;
    int m_firstVisibleTime;

    int m_scrollerFirstVisibleTime;
    int m_scrollerVisibleDuration;
    int m_minScrollerVisibleDuration;

    int m_visibleDuration;
    int m_defaultDuration;
    int m_totalTime;

    int m_minVisibleDuration;
    int m_maxVisibleDuration;

    wxDateTime m_startTime;

    wxRect m_rectBackground;
    wxRect m_rectTimeline;
    wxRect m_rectScroller;
    wxRect m_rectScrollerTrack;
    wxRect m_rectTimelineTrack;
    wxRect m_rectVisibleFrame;
    wxRect m_rectVisibleFrameLeft, m_rectVisibleFrameRight;
    wxRect m_rectLeftArrow, m_rectRightArrow;
    wxRect m_rectLeftArrowDraw, m_rectRightArrowDraw;
    wxRect m_rectTimelineMain;
    wxRect m_rectScrollerMain;
    wxRect m_rectScrollerTimeScale;
    wxRect m_rectTimelineTimeScale;
    ElementType m_selectedElement;
    ElementType m_lastElement;

    int m_colorCounter;

    bool m_mouseCaptured;
    bool m_mouseDown;

    int m_moveDirection;

    int m_dragFirstVisibleTime;
    int m_dragVisibleDuration;
    int m_dragPreviewTime;
    bool m_isSnapping;

    TimelineElementState m_stateLeftArrow = TimelineElementState::Normal;
    TimelineElementState m_stateRightArrow = TimelineElementState::Normal;
    TimelineElementState m_stateVisibleFrame = TimelineElementState::Normal;

    // State for scroller item dragging
    int m_hoveredScrollerItemIndex;
    int m_draggedScrollerItemIndex;
    int m_contextMenuItemIndex; 

    // Sizing constants
    int m_ScrollerHeight = 64;
    int m_GapHeight = 7;
    int m_ArrowWidth = 14;
    int m_ArrowDrawWidth = 6;
    int m_ArrowDrawHeight = 16;
    int m_ScrollerTimeScaleHeight = 28;
    int m_TimelineTimeScaleHeight = 18;
    int m_MinItemSize = 6;
    int m_TimelineVMargin = 9;
    int m_ScrollerVMargin = 3;
    int m_resizeBorder = 3;

    bool m_isDraggingDetachedItem;      
    TimelineItem<T> m_detachedDragItemVisual; 
    int m_detachedDragItemOriginalIndex; 
    wxPoint m_detachedDragItemScreenPos;  
    wxSize m_detachedDragItemSize;       

    bool m_showOriginalPositionPlaceholder;
    wxRect m_originalPositionPlaceholderRectScroller; 
    wxColour m_originalPositionPlaceholderColour;     

    wxPoint m_cursorToDetachedVisualOffset; 
    int m_dragScrollerItemInitialClickTimeOffset; 

    bool m_isDraggingFloatingItem;
    FloatingItemPopupWindow<T>* m_pFloatingItemWin;
    
    TimelineItem<T> m_floatingItemVisualData;
    int m_floatingItemOriginalIndex;
    wxSize m_floatingItemVisualSize;
    
    wxPoint m_cursorToFloatingWinOffset; 
    int m_clickToItemTimeOffset; 

    bool m_showDropTargetOnScroller; 
    wxRect m_dropTargetOnScrollerRect;

    wxRect m_dropIndicatorRect;
    wxRect m_dropIndicatorRectScroller;
};

#include "wxTimelineCtrl_impl.h"

#endif // _WX_TIMELINE_CTRL_H
