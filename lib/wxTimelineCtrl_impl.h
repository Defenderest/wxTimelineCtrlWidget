#include "wxTimelineCtrl.h"

template<typename T>
void wxTimelineCtrl<T>::Init()
{
    m_artProvider = new TimelineArtProvider();

    m_defaultDuration = 10;
    m_startTime = wxDateTime::Today();
    m_totalDuration = 120;
    m_totalTime = m_totalDuration;
    m_visibleDuration = 60;

    m_minVisibleDuration = 10;
    m_maxVisibleDuration = m_totalDuration;
    m_firstVisibleTime = 0;

    m_minScrollerVisibleDuration = m_minVisibleDuration;
    int defaultScrollerView = wxMax(m_visibleDuration + 30, m_visibleDuration * 2);
    defaultScrollerView = wxMin(defaultScrollerView, m_totalDuration);
    defaultScrollerView = wxMax(defaultScrollerView, m_minScrollerVisibleDuration);
    m_scrollerVisibleDuration = defaultScrollerView;
    m_scrollerFirstVisibleTime = 0;
    AdjustMainViewToScrollerView();

    m_stateLeftArrow = TimelineElementState::Normal;
    m_stateRightArrow = TimelineElementState::Normal;
    m_stateVisibleFrame = TimelineElementState::Normal;

    m_mouseDown = false;
    m_mouseCaptured = false;
    m_moveDirection = 0;

    m_isSelecting = false;
    m_selectionRect = wxRect();

    m_ptStartPos = wxDefaultPosition;
    m_ptEndPos = wxDefaultPosition;

    m_selectedElement = ET_NONE;
    m_lastElement = ET_NONE;

    m_colorCounter = 0;

    m_visibleItemBegin = m_items.end();
    m_visibleItemEnd = m_items.end();
    m_lastTask = m_items.end();
    m_activeTask = m_items.end();
    m_contextMenuItemIndex = -1;

    m_timerMove.SetOwner(this, wxID_ANY);

    m_isDraggingDetachedItem = false;
    m_detachedDragItemOriginalIndex = -1;
    m_showOriginalPositionPlaceholder = false;
    m_dragScrollerItemInitialClickTimeOffset = 0;
    m_pFloatingItemWin = nullptr;

    m_dropIndicatorRect = wxRect();
    m_dropIndicatorRectScroller = wxRect();

    m_isSnapping = false;
}

template<typename T>
void wxTimelineCtrl<T>::SetEventHandlers()
{
    Bind(wxEVT_PAINT, &wxTimelineCtrl<T>::OnPaint, this);
    Bind(wxEVT_SIZE, &wxTimelineCtrl<T>::OnSize, this);
    Bind(wxEVT_ERASE_BACKGROUND, &wxTimelineCtrl<T>::OnEraseBackground, this);

    Bind(wxEVT_LEFT_DOWN, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_LEFT_UP, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_LEFT_DCLICK, &wxTimelineCtrl<T>::OnMouse, this);

    Bind(wxEVT_RIGHT_DOWN, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_RIGHT_UP, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_RIGHT_DCLICK, &wxTimelineCtrl<T>::OnMouse, this);

    Bind(wxEVT_MIDDLE_DOWN, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_MIDDLE_UP, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_MIDDLE_DCLICK, &wxTimelineCtrl<T>::OnMouse, this);

    Bind(wxEVT_MOTION, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_MOUSEWHEEL, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_LEAVE_WINDOW, &wxTimelineCtrl<T>::OnMouse, this);
    Bind(wxEVT_ENTER_WINDOW, &wxTimelineCtrl<T>::OnMouse, this);

    Bind(wxEVT_MOUSE_CAPTURE_LOST, &wxTimelineCtrl<T>::OnMouseCaptureLost, this);

    Bind(wxEVT_KEY_DOWN, &wxTimelineCtrl<T>::OnKeyDown, this);

    Bind(wxEVT_TIMER, &wxTimelineCtrl<T>::OnTimer, this);

    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if (event.GetId() == ID_TIMELINE_DELETE)
        {
            RemoveSelectedItems();
        }
        else if (event.GetId() == ID_TIMELINE_DELETE_SCROLLER_ITEM)
        {
            RemoveContextScrollerItem();
        }
        });
}

template<typename T>
void wxTimelineCtrl<T>::OnPaint(wxPaintEvent&)
{
    wxBufferedPaintDC dc(this, m_buffer);
    wxGCDC gdc(dc);
    Draw(gdc);
}

template<typename T>
void wxTimelineCtrl<T>::OnEraseBackground(wxEraseEvent&) {}

template<typename T>
void wxTimelineCtrl<T>::Draw(wxDC& dc)
{
    m_artProvider->DrawBackground(dc, m_rectBackground);
    DrawTimeline(dc); 
    DrawScroller(dc); 
    m_artProvider->DrawGap(dc, m_rectTimelineTrack, m_rectVisibleFrame);

    if (m_isDraggingDetachedItem && m_detachedDragItemVisual.Data && m_pFloatingItemWin == nullptr) 
    {
        TimelineItem<T> itemToDraw = m_detachedDragItemVisual; 
        itemToDraw.Colour = itemToDraw.Colour.ChangeLightness(120);
        wxColour c = itemToDraw.Colour;
        itemToDraw.Colour.Set(c.Red(), c.Green(), c.Blue(), 180); 
        itemToDraw.State = TimelineElementState::Pressed; 

        wxRect detachedItemRectOnScreen(m_detachedDragItemScreenPos, m_detachedDragItemSize);
        
        m_artProvider->DrawItem(dc, detachedItemRectOnScreen, GetClientRect() , itemToDraw, false , false );
    }
}

template<typename T>
void wxTimelineCtrl<T>::DrawTimeline(wxDC& dc)
{
    m_artProvider->DrawTimelineBackground(dc, m_rectTimeline);

    const int visibleStart = m_firstVisibleTime;
    const int visibleEnd = m_firstVisibleTime + m_visibleDuration;

    if (visibleEnd <= visibleStart)
    {
        return;
    }

    m_artProvider->DrawTimelineTrack(dc, m_rectTimelineMain);

    const int timeBuffer = 1;
    wxDateTime adjustedStart = m_startTime + wxTimeSpan::Seconds(wxMax(0, visibleStart - timeBuffer));
    wxDateTime adjustedEnd = m_startTime + wxTimeSpan::Seconds(visibleEnd + timeBuffer);

    wxRect clipRectTimeScale = m_rectTimelineTimeScale;
    clipRectTimeScale.Inflate(10, 0);
    wxDCClipper timeScaleClipper(dc, clipRectTimeScale);

    m_artProvider->DrawTimeScale(
        dc,
        m_rectTimelineTimeScale,
        adjustedStart,
        adjustedEnd
    );

    wxDCClipper mainTimelineClipper(dc, m_rectTimelineMain);

    wxString rangeText = FormatTime(visibleStart) + " - " + FormatTime(visibleEnd);
    wxString zoomText = wxString::Format("Zoom: %s", FormatTime(m_visibleDuration));

    dc.SetTextForeground(wxColour(80, 80, 80));
    dc.SetFont(wxFont(wxFontInfo(8).Family(wxFONTFAMILY_DEFAULT).Style(wxFONTSTYLE_NORMAL).Weight(wxFONTWEIGHT_NORMAL)));
    dc.DrawText(rangeText, m_rectTimelineMain.x + 5, m_rectTimelineMain.y + 5);
    dc.DrawText(zoomText, m_rectTimelineMain.x + 5, m_rectTimelineMain.y + 20);

    if (m_visibleItemBegin == m_items.end() && m_visibleItemEnd == m_items.end())
    {
        return;
    }

    for (size_t index = 0; index < m_items.size(); ++index)
    {
        auto& item = m_items[index];
        if (!item.Data) {
            continue;
        }
        if (item.Rect.width <= 0) {
            continue;
        }

        try {
            m_artProvider->DrawItem(dc, item.Rect, m_rectTimelineTrack, item, false, false);
        }
        catch (const std::exception& e) {
        }
    }

    if (m_isDraggingDetachedItem)
    {
        if (!m_dropIndicatorRect.IsEmpty() && m_detachedDragItemOriginalIndex != -1)
        {
            wxDCClipper clip(dc, m_rectTimelineTrack);
            TimelineItem<T> previewItem = m_items[m_detachedDragItemOriginalIndex];
            if (m_isSnapping)
            {
                previewItem.Colour.Set(0, 255, 0, 128); // Green for snap
            }
            else
            {
                wxColour c = previewItem.Colour;
                previewItem.Colour.Set(c.Red(), c.Green(), c.Blue(), 128); // semi-transparent
            }
            m_artProvider->DrawItem(dc, m_dropIndicatorRect, m_rectTimelineTrack, previewItem, false, false);
        }
    }


    if (!m_selectionRect.IsEmpty())
    {
        wxColour selectionColor(0, 120, 215, 80);
        dc.SetBrush(wxBrush(selectionColor));
        dc.SetPen(wxPen(wxColour(0, 120, 215), 1));

        wxRect clippedSelectionRect = m_selectionRect.Intersect(m_rectTimelineTrack);
        if (!clippedSelectionRect.IsEmpty())
        {
            dc.DrawRectangle(clippedSelectionRect);

            if (clippedSelectionRect.width > 20)
            {
                int selStartTime = m_firstVisibleTime + TimelineCoordToTime(clippedSelectionRect.GetLeft() - m_rectTimelineTrack.x);
                int selEndTime = m_firstVisibleTime + TimelineCoordToTime(clippedSelectionRect.GetRight() - m_rectTimelineTrack.x);
                wxString selTimeText = FormatTime(selStartTime) + " - " + FormatTime(selEndTime);

                dc.SetTextForeground(wxColour(0, 0, 0));
                dc.SetFont(wxFont(wxFontInfo(8).Family(wxFONTFAMILY_DEFAULT)));

                wxSize textSize = dc.GetTextExtent(selTimeText);
                int textX = clippedSelectionRect.x + (clippedSelectionRect.width - textSize.GetWidth()) / 2;
                int textY = clippedSelectionRect.y + (clippedSelectionRect.height - textSize.GetHeight()) / 2;

                if (textY >= clippedSelectionRect.y && (textY + textSize.GetHeight()) <= clippedSelectionRect.GetBottom())
                {
                    dc.DrawText(selTimeText, textX, textY);
                }
            }
        }
    }
}

template<typename T>
void wxTimelineCtrl<T>::DrawScroller(wxDC& dc)
{
    m_artProvider->DrawScrollerBackground(dc, m_rectScroller);
    m_artProvider->DrawScrollerTrack(dc, m_rectScrollerMain);
    m_artProvider->DrawLeftArrow(dc, m_rectLeftArrowDraw, m_stateLeftArrow);
    m_artProvider->DrawRightArrow(dc, m_rectRightArrowDraw, m_stateRightArrow);
    m_artProvider->DrawTimeScale(dc, m_rectScrollerTimeScale,
        m_startTime + wxTimeSpan::Seconds(m_scrollerFirstVisibleTime),
        m_startTime + wxTimeSpan::Seconds(m_scrollerFirstVisibleTime + m_scrollerVisibleDuration)
    );
    wxDCClipper clip(dc, m_rectScrollerTrack);
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_isDraggingDetachedItem && m_showOriginalPositionPlaceholder && (int)i == m_detachedDragItemOriginalIndex)
        {
            continue; 
        }

        auto& item = m_items[i]; 
        if (!item.Data)
            continue;
        int itemStartTime = item.Data->GetStartTime();
        int itemEndTime = item.Data->GetEndTime();
        if (itemEndTime <= m_scrollerFirstVisibleTime || itemStartTime >= m_scrollerFirstVisibleTime + m_scrollerVisibleDuration)
            continue;

        int itemX_start_in_track = ScrollerTimeToCoord(itemStartTime);
        int itemX_end_in_track = ScrollerTimeToCoord(itemEndTime);

        itemX_start_in_track = wxMax(0, itemX_start_in_track);
        itemX_end_in_track = wxMin(m_rectScrollerTrack.width, itemX_end_in_track);

        if (itemX_end_in_track > itemX_start_in_track)
        {
            wxRect itemRectInScroller = m_rectScrollerTrack;
            itemRectInScroller.x = m_rectScrollerTrack.x + itemX_start_in_track;
            itemRectInScroller.width = itemX_end_in_track - itemX_start_in_track;

            m_artProvider->DrawItem(dc, itemRectInScroller, m_rectScrollerTrack, item, true, false);

            if ((int)i == m_contextMenuItemIndex)
            {
                wxPen oldPen = dc.GetPen();
                wxBrush oldBrush = dc.GetBrush();

                dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), 2, wxPENSTYLE_SOLID));
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                wxRect highlightRect = itemRectInScroller;
                dc.DrawRectangle(highlightRect);

                dc.SetPen(oldPen);
                dc.SetBrush(oldBrush);
            }
        }
    }

    if (m_isDraggingDetachedItem)
    {
        if (!m_dropIndicatorRectScroller.IsEmpty() && m_detachedDragItemOriginalIndex != -1)
        {
            wxDCClipper clip(dc, m_rectScrollerTrack);
            TimelineItem<T> previewItem = m_items[m_detachedDragItemOriginalIndex];
            if (m_isSnapping)
            {
                previewItem.Colour.Set(0, 255, 0, 128); // Green for snap
            }
            else
            {
                wxColour c = previewItem.Colour;
                previewItem.Colour.Set(c.Red(), c.Green(), c.Blue(), 128); // semi-transparent
            }
            m_artProvider->DrawItem(dc, m_dropIndicatorRectScroller, m_rectScrollerTrack, previewItem, true, false);
        }
    }
    
    if (m_isDraggingDetachedItem && m_showOriginalPositionPlaceholder && m_originalPositionPlaceholderRectScroller.width > 0)
    {
        wxColour placeholderFillColour = m_originalPositionPlaceholderColour;
        placeholderFillColour.Set(placeholderFillColour.Red(), placeholderFillColour.Green(), placeholderFillColour.Blue(), 30); 
        
        dc.SetPen(wxPen(m_originalPositionPlaceholderColour, 1, wxPENSTYLE_DOT)); 
        dc.SetBrush(wxBrush(placeholderFillColour));
        dc.DrawRectangle(m_originalPositionPlaceholderRectScroller);
    }
            
    m_artProvider->DrawVisibleFrame(dc, m_rectVisibleFrame, m_stateVisibleFrame);
}

template<typename T>
void wxTimelineCtrl<T>::OnSize(wxSizeEvent&)
{
    m_buffer.Create(GetSize(), 32);
    RecalcRects();
    RecalcItems();
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::RecalcRects()
{
    m_rectBackground = GetClientRect();

    int timelineHeight = 80;
    int scrollerHeight = 60;
    int gapHeight = 8;


    m_rectTimeline = m_rectBackground;
    m_rectTimeline.height -= scrollerHeight + gapHeight;

    m_rectTimelineTimeScale = wxRect(m_rectTimeline.x, m_rectTimeline.GetBottom() - 20, m_rectTimeline.width, 20);
    m_rectTimelineMain = wxRect(m_rectTimeline.x, m_rectTimeline.y + 5, m_rectTimeline.width, m_rectTimeline.height - 25);
    m_rectTimelineTrack = m_rectTimelineMain.Deflate(4, 4);

    m_rectScroller = wxRect(m_rectTimeline.x, m_rectTimeline.GetBottom() + gapHeight, m_rectTimeline.width, scrollerHeight);
    m_rectScrollerMain = wxRect(m_rectScroller.x + 20, m_rectScroller.y + 5, m_rectScroller.width - 40, scrollerHeight - 25);
    m_rectScrollerTrack = m_rectScrollerMain.Deflate(4, 4);

    m_rectLeftArrow = wxRect(m_rectScroller.x, m_rectScroller.y + 10, 20, 40);
    m_rectRightArrow = wxRect(m_rectScroller.GetRight() - 20, m_rectScroller.y + 10, 20, 40);

    m_rectLeftArrowDraw = m_rectLeftArrow.Deflate(5);
    m_rectRightArrowDraw = m_rectRightArrow.Deflate(5);

    m_rectScrollerTimeScale = wxRect(m_rectScrollerMain.x, m_rectScrollerMain.GetBottom() + 2, m_rectScrollerMain.width, 20);

    RecalcVisibleFrame();
}

template<typename T>
void wxTimelineCtrl<T>::RecalcVisibleFrame()
{
    if (m_scrollerVisibleDuration <= 0 || m_rectScrollerTrack.width <= 0)
    {
        m_rectVisibleFrame = wxRect();
        m_rectVisibleFrameLeft = wxRect();
        m_rectVisibleFrameRight = wxRect();
        return;
    }

    int trackX = m_rectScrollerTrack.x;
    int trackW = m_rectScrollerTrack.width;

    double mainViewStartRelativeToScroller = 0.0;
    double mainViewEndRelativeToScroller = 1.0;

    if (m_scrollerVisibleDuration > 0) {
        mainViewStartRelativeToScroller = (double)(m_firstVisibleTime - m_scrollerFirstVisibleTime) / m_scrollerVisibleDuration;
        mainViewEndRelativeToScroller = (double)(m_firstVisibleTime + m_visibleDuration - m_scrollerFirstVisibleTime) / m_scrollerVisibleDuration;
    }

    mainViewStartRelativeToScroller = wxMax(0.0, mainViewStartRelativeToScroller);
    mainViewEndRelativeToScroller = wxMin(1.0, mainViewEndRelativeToScroller);
    if (mainViewEndRelativeToScroller < mainViewStartRelativeToScroller) mainViewEndRelativeToScroller = mainViewStartRelativeToScroller;

    int x1 = trackX + wxRound(mainViewStartRelativeToScroller * trackW);
    int x2 = trackX + wxRound(mainViewEndRelativeToScroller * trackW);

    x1 = wxMax(trackX, x1);
    x2 = wxMin(trackX + trackW, x2);
    if (x2 < x1) x2 = x1;

    m_rectVisibleFrame = wxRect(
        wxPoint(x1, m_rectScrollerTrack.y - 5),
        wxPoint(x2, m_rectScrollerTrack.GetBottom() + 5)
    );

    int gripSize = 4;
    m_rectVisibleFrameLeft = wxRect(
        m_rectVisibleFrame.x - gripSize,
        m_rectVisibleFrame.y,
        2 * gripSize,
        m_rectVisibleFrame.height
    );
    m_rectVisibleFrameRight = wxRect(
        m_rectVisibleFrame.GetRight() - gripSize,
        m_rectVisibleFrame.y,
        2 * gripSize,
        m_rectVisibleFrame.height
    );
}


template<typename T>
void wxTimelineCtrl<T>::RecalcItems()
{
        m_firstVisibleTime, m_visibleDuration, m_rectTimelineTrack.width, m_rectTimelineTrack.y, m_rectTimelineTrack.height;

    int visibleStart = m_firstVisibleTime;
    int visibleDuration = m_visibleDuration;
    int width = m_rectTimelineTrack.width;

    m_visibleItemBegin = m_items.end();
    m_visibleItemEnd = m_items.end();

    for (auto& item : m_items)
    {
        item.Rect = wxRect();
        item.m_displayLane = 0;
    }

    if (m_items.empty())
    {
        return;
    }

    std::vector<size_t> sorted_indices(m_items.size());
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);

    std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
        [&](size_t a_idx, size_t b_idx) {
            const auto& item_a_data = m_items[a_idx].Data;
            const auto& item_b_data = m_items[b_idx].Data;

            if (!item_a_data && !item_b_data) return false;
            if (!item_a_data) return false;
            if (!item_b_data) return true;

            if (item_a_data->GetStartTime() != item_b_data->GetStartTime()) {
                return item_a_data->GetStartTime() < item_b_data->GetStartTime();
            }
            return item_a_data->GetEndTime() < item_b_data->GetEndTime(); 
        });

    std::vector<int> lane_end_times;
    int max_lane_used = -1;

    for (size_t item_original_index : sorted_indices)
    {
        TimelineItem<T>& current_item = m_items[item_original_index];
        if (!current_item.Data) continue;

        int item_start_time = current_item.Data->GetStartTime();
        int item_end_time = current_item.Data->GetEndTime();

        if (item_start_time >= item_end_time) {
            current_item.m_displayLane = 0; 
                item_original_index, item_start_time, item_end_time;
            continue;
        }

        int assigned_lane = -1;
        for (size_t lane_idx = 0; lane_idx < lane_end_times.size(); ++lane_idx)
        {
            if (item_start_time >= lane_end_times[lane_idx])
            {
                assigned_lane = lane_idx;
                lane_end_times[lane_idx] = item_end_time;
                break;
            }
        }

        if (assigned_lane == -1)
        {
            assigned_lane = lane_end_times.size();
            lane_end_times.push_back(item_end_time);
        }
        current_item.m_displayLane = assigned_lane;
        if (assigned_lane > max_lane_used)
        {
            max_lane_used = assigned_lane;
        }
    }

    int num_lanes = m_items.empty() ? 1 : (max_lane_used + 1);
    int lane_height = m_rectTimelineTrack.height;

    if (num_lanes > 0 && m_rectTimelineTrack.height > 0)
    {
        lane_height = m_rectTimelineTrack.height / num_lanes;
    }
    if (lane_height < 1 && m_rectTimelineTrack.height > 0) lane_height = 1;
    if (m_rectTimelineTrack.height <= 0) lane_height = 0;

        max_lane_used, num_lanes, lane_height, m_rectTimelineTrack.height;

    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
        size_t currentIndex = std::distance(m_items.begin(), it);
        if (!it->Data) {
            continue;
        }

        int itemStart = it->Data->GetStartTime();
        int itemEnd = it->Data->GetEndTime();

        bool isVisible = !(itemEnd <= visibleStart || itemStart >= visibleStart + visibleDuration);

        if (isVisible)
        {
            int relativeStart = itemStart - visibleStart;
            int relativeEnd = itemEnd - visibleStart;

            if (relativeStart < 0) relativeStart = 0;
            if (relativeEnd > visibleDuration) relativeEnd = visibleDuration;

            if (visibleDuration <= 0 || width <= 0) {
                continue;
            }

            int xStart = (relativeStart * width) / visibleDuration;
            int xEnd = (relativeEnd * width) / visibleDuration;

            if (xEnd < xStart) xEnd = xStart;

            int item_rect_y = m_rectTimelineTrack.y + it->m_displayLane * lane_height;
            int item_rect_height = lane_height;

            if (lane_height > 0 && it->m_displayLane == num_lanes - 1) {
                 item_rect_height = (m_rectTimelineTrack.y + m_rectTimelineTrack.height) - item_rect_y;
            }
            if (item_rect_height < 0) item_rect_height = 0;


            wxRect rect(
                m_rectTimelineTrack.x + xStart,
                item_rect_y,
                xEnd - xStart,
                item_rect_height
            );
            
            if (rect.height <= 0 && m_rectTimelineTrack.height > 0 && num_lanes > 0) {
            }


            it->Rect = rect;
                currentIndex, itemStart, itemEnd, it->m_displayLane,
                rect.x, rect.y, rect.width, rect.height;

            if (m_visibleItemBegin == m_items.end())
                m_visibleItemBegin = it;
            m_visibleItemEnd = it + 1;
        }
        else
        {
                currentIndex, itemStart, itemEnd;
        }
    }
}


template<typename T>
void wxTimelineCtrl<T>::OnMouse(wxMouseEvent& event)
{
    const wxPoint pos = event.GetPosition();
    ElementType currentHoverType = ET_NONE;

    if (m_isDraggingDetachedItem) {
        currentHoverType = ET_SCROLLER_ITEM_DRAG;
    } else if (!m_mouseCaptured) {
        currentHoverType = GetElementFromPos(pos);
    } else {
        if (m_mouseDown && m_selectedElement != ET_NONE) {
            currentHoverType = m_selectedElement;
        } else {
            currentHoverType = m_lastElement;
        }
    }

    if (event.Moving() && !m_isDraggingDetachedItem)
    {
        if (currentHoverType != m_lastElement)
        {
            OnLeaveElement(m_lastElement);
            OnEnterElement(currentHoverType);
        }
    }

    if (event.IsButton())
        SetFocus();

    if (event.LeftDown())
    {
        m_contextMenuItemIndex = -1;

        if (event.ControlDown())
        {
            if (currentHoverType == ET_SCROLLER_ITEM_DRAG && m_hoveredScrollerItemIndex != -1 && !m_isDraggingDetachedItem)
            {
                m_mouseDown = true;
                m_selectedElement = ET_SCROLLER_ITEM_DRAG;

                m_isDraggingDetachedItem = true;
                m_detachedDragItemOriginalIndex = m_hoveredScrollerItemIndex;
                m_dragPreviewTime = -1;

                TimelineItem<T>& originalItem = m_items[m_detachedDragItemOriginalIndex];
                if (!originalItem.Data) {
                    m_isDraggingDetachedItem = false;
                    return;
                }

                m_showOriginalPositionPlaceholder = true;
                m_originalPositionPlaceholderColour = originalItem.Colour;

                int itemStartTime = originalItem.Data->GetStartTime();
                int itemEndTime = originalItem.Data->GetEndTime();
                int x1_track = ScrollerTimeToCoord(itemStartTime);
                int x2_track = ScrollerTimeToCoord(itemEndTime);
                
                wxLogDebug("OnMouse LeftDown Ctrl: itemStartTime=%d, itemEndTime=%d, duration=%d", itemStartTime, itemEndTime, itemEndTime - itemStartTime);
                wxLogDebug("OnMouse LeftDown Ctrl: m_scrollerFirstVisibleTime=%d, m_scrollerVisibleDuration=%d, m_rectScrollerTrack.width=%d", m_scrollerFirstVisibleTime, m_scrollerVisibleDuration, m_rectScrollerTrack.width);
                wxLogDebug("OnMouse LeftDown Ctrl: x1_track=%d, x2_track=%d, pixel_width_on_scroller=%d", x1_track, x2_track, x2_track - x1_track);

                m_originalPositionPlaceholderRectScroller = wxRect(
                    m_rectScrollerTrack.x + wxClip(x1_track, 0, m_rectScrollerTrack.width),
                    m_rectScrollerTrack.y,
                    wxClip(x2_track, 0, m_rectScrollerTrack.width) - wxClip(x1_track, 0, m_rectScrollerTrack.width),
                    m_rectScrollerTrack.height
                );
                
                if (m_originalPositionPlaceholderRectScroller.width < 0) {
                     m_originalPositionPlaceholderRectScroller.width = 0;
                }
                wxLogDebug("OnMouse LeftDown Ctrl: m_originalPositionPlaceholderRectScroller.width=%d, height=%d", m_originalPositionPlaceholderRectScroller.width, m_originalPositionPlaceholderRectScroller.height);


                m_detachedDragItemVisual = originalItem;

                int placeholderWidth = m_originalPositionPlaceholderRectScroller.width;
                wxLogDebug("OnMouse LeftDown Ctrl: placeholderWidth before adjustment = %d", placeholderWidth);

                if (placeholderWidth <= 0 && (itemEndTime - itemStartTime > 0)) {
                    placeholderWidth = 1; 
                    wxLogDebug("OnMouse LeftDown Ctrl: placeholderWidth corrected to 1 due to zero/negative pixel width but positive duration");
                } else if (placeholderWidth < 0) {
                    placeholderWidth = 0;
                     wxLogDebug("OnMouse LeftDown Ctrl: placeholderWidth corrected to 0 due to negative pixel width");
                }


                int finalPopupWindowWidth = placeholderWidth;
                if (finalPopupWindowWidth < 30 && placeholderWidth > 0) { 
                    finalPopupWindowWidth = 30;
                    wxLogDebug("OnMouse LeftDown Ctrl: finalPopupWindowWidth adjusted to 30 (minimum for visible placeholder)");
                } else if (finalPopupWindowWidth < 5 && placeholderWidth == 0 && (itemEndTime - itemStartTime > 0)) {
                    finalPopupWindowWidth = 30; // If item has duration but placeholder is 0, make popup 30
                    wxLogDebug("OnMouse LeftDown Ctrl: finalPopupWindowWidth set to 30 (item has duration, placeholder 0)");
                } else if (finalPopupWindowWidth == 0 && (itemEndTime - itemStartTime == 0)) {
                    finalPopupWindowWidth = 30; // Zero duration item, popup 30
                     wxLogDebug("OnMouse LeftDown Ctrl: finalPopupWindowWidth set to 30 (zero duration item)");
                }
                 else {
                    wxLogDebug("OnMouse LeftDown Ctrl: finalPopupWindowWidth set to placeholderWidth = %d", placeholderWidth);
                }
                if (finalPopupWindowWidth <=0 ) finalPopupWindowWidth = 30; // Absolute minimum fallback

                m_detachedDragItemSize.SetWidth(finalPopupWindowWidth);
                m_detachedDragItemSize.SetHeight(wxMax(15, m_originalPositionPlaceholderRectScroller.height > 4 ? m_originalPositionPlaceholderRectScroller.height - 2 * m_ScrollerVMargin : 15));
                
                wxLogDebug("OnMouse LeftDown Ctrl: m_detachedDragItemSize.width=%d, height=%d", m_detachedDragItemSize.GetWidth(), m_detachedDragItemSize.GetHeight());

                if (m_originalPositionPlaceholderRectScroller.width > 0) {
                    double relativeX = static_cast<double>(pos.x - m_originalPositionPlaceholderRectScroller.GetLeft()) / m_originalPositionPlaceholderRectScroller.GetWidth();
                    m_cursorToDetachedVisualOffset.x = static_cast<int>(round(relativeX * m_detachedDragItemSize.GetWidth()));
                } else {
                    m_cursorToDetachedVisualOffset.x = m_detachedDragItemSize.GetWidth() / 2;
                }

                if (m_originalPositionPlaceholderRectScroller.height > 0) {
                    double relativeY = static_cast<double>(pos.y - m_originalPositionPlaceholderRectScroller.GetTop()) / m_originalPositionPlaceholderRectScroller.GetHeight();
                    m_cursorToDetachedVisualOffset.y = static_cast<int>(round(relativeY * m_detachedDragItemSize.GetHeight()));
                } else {
                    m_cursorToDetachedVisualOffset.y = m_detachedDragItemSize.GetHeight() / 2;
                }

                wxPoint screenMousePos = ClientToScreen(pos);
                m_detachedDragItemScreenPos = screenMousePos - m_cursorToDetachedVisualOffset;

                int mouseTimeInScroller = ScrollerCoordToTime(pos.x - m_rectScrollerTrack.x);
                m_dragScrollerItemInitialClickTimeOffset = mouseTimeInScroller - itemStartTime;

                if (m_pFloatingItemWin) {
                    m_pFloatingItemWin->Destroy();
                    m_pFloatingItemWin = nullptr;
                }

                m_pFloatingItemWin = new FloatingItemPopupWindow<T>(this,
                                                                m_detachedDragItemVisual,
                                                                m_detachedDragItemSize,
                                                                m_artProvider);
                if (m_pFloatingItemWin) {
                    m_pFloatingItemWin->Move(m_detachedDragItemScreenPos);
                    m_pFloatingItemWin->Show();
                }

                if (!HasCapture()) { CaptureMouse(); m_mouseCaptured = true; }
            }
            else if ((event.ShiftDown() || event.AltDown()) && m_rectTimelineTrack.Contains(pos) && !m_isDraggingDetachedItem)
            {
                m_isSelecting = true;
                m_selectionStart = pos;
                m_selectionEnd = pos;
                m_selectionRect = wxRect(pos, wxSize(1, m_rectTimelineTrack.height));
                if (!HasCapture()) { CaptureMouse(); m_mouseCaptured = true; }
            }
            else if (!m_isDraggingDetachedItem)
            {
                OnMouseDown(currentHoverType, pos);
            }
        }
        else if (!m_isDraggingDetachedItem)
        {
            if ((event.ShiftDown() || event.AltDown()) && m_rectTimelineTrack.Contains(pos))
            {
                m_isSelecting = true;
                m_selectionStart = pos;
                m_selectionEnd = pos;
                m_selectionRect = wxRect(pos, wxSize(1, m_rectTimelineTrack.height));
                if (!HasCapture()) { CaptureMouse(); m_mouseCaptured = true; }
            }
            else
            {
                if (!event.ShiftDown() &&
                    !(currentHoverType >= ET_TIMELINE_ITEM && currentHoverType <= ET_TIMELINE_ITEM_MAX))
                {
                     ClearSelection();
                }
                OnMouseDown(currentHoverType, pos);
            }
        }
    }
    else if (event.RightDown() && !m_isDraggingDetachedItem)
    {
        if (currentHoverType == ET_SCROLLER_ITEM_DRAG && m_hoveredScrollerItemIndex != -1)
        {
            m_contextMenuItemIndex = m_hoveredScrollerItemIndex;
            // Если были выделены элементы на основной временной шкале, очистим это выделение,
            // так как сейчас контекст переключается на элемент в полосе прокрутки.
            // Важно не вызывать ClearSelection(), так как он сбрасывает m_contextMenuItemIndex.
            if (!m_selectedItems.empty())
            {
                m_selectedItems.clear();
                Refresh(); // Обновляем отображение, чтобы снять выделение с элементов временной шкалы
            }
            ShowContextMenu(ClientToScreen(pos));
        }
        else
        {
            m_contextMenuItemIndex = -1;
            if (currentHoverType >= ET_TIMELINE_ITEM && currentHoverType <= ET_TIMELINE_ITEM_MAX)
            {
                size_t index = std::distance(m_items.begin(), m_activeTask);
                if (index < m_items.size())
                {
                    if (!IsItemSelected(index))
                    {
                        if (event.ControlDown()) ToggleItemSelection(index);
                        else SelectItem(index, true);
                    }
                    else if (!event.ControlDown() && m_selectedItems.size() > 1)
                    {
                        SelectItem(index, true);
                    }
                    SendItemEvent(wxEVT_TIMELINE_SELECTION, index);
                }
            }
            if (!m_selectedItems.empty())
            {
                ShowContextMenu(ClientToScreen(pos));
            }
        }
    }
    else if (event.LeftUp())
    {
        if (m_isDraggingDetachedItem) {
            OnMouseUp(ET_SCROLLER_ITEM_DRAG, pos);
        }
        else if (m_isSelecting)
        {
            m_isSelecting = false;
            if (m_selectionRect.width < 5) ClearSelection();
            else if (event.AltDown()) ZoomToSelection();
            if (m_mouseCaptured && HasCapture()) { ReleaseMouse(); m_mouseCaptured = false;}
        }
        else if (m_selectedElement != ET_NONE)
        {
            OnMouseUp(m_selectedElement, pos);
        }
        else {
             if (m_mouseCaptured && HasCapture()) { ReleaseMouse(); m_mouseCaptured = false; }
        }

        if (!m_isDraggingDetachedItem) {
            ElementType newHoverTypeAfterUp = GetElementFromPos(pos);
            if (newHoverTypeAfterUp != m_lastElement) {
                OnLeaveElement(m_lastElement);
                OnEnterElement(newHoverTypeAfterUp);
            }
        }
    }
    else if (event.RightUp() && !m_isDraggingDetachedItem)
    {
        ElementType newHoverTypeAfterRightUp = GetElementFromPos(pos);
        if (newHoverTypeAfterRightUp != m_lastElement) {
            OnLeaveElement(m_lastElement);
            OnEnterElement(newHoverTypeAfterRightUp);
        }
    }
    else if (event.Dragging())
    {
        if (m_isDraggingDetachedItem) {
             OnMouseDrag(ET_SCROLLER_ITEM_DRAG, pos);
        }
        else if (m_isSelecting)
        {
            m_selectionEnd = pos;
            int left = wxMin(m_selectionStart.x, m_selectionEnd.x);
            int right = wxMax(m_selectionStart.x, m_selectionEnd.x);
            left = wxMax(left, m_rectTimelineTrack.x);
            right = wxMin(right, m_rectTimelineTrack.x + m_rectTimelineTrack.width);
            m_selectionRect = wxRect(left, m_rectTimelineTrack.y, right - left, m_rectTimelineTrack.height);
        }
        else if (m_mouseDown)
        {
            OnMouseDrag(m_selectedElement, pos);
        }
    }

    if (event.GetWheelRotation() != 0 && !m_isDraggingDetachedItem)
    {
        if (m_contextMenuItemIndex != -1) {
            m_contextMenuItemIndex = -1;
        }

        int deltaSteps = event.GetWheelRotation() / event.GetWheelDelta();
        if (event.GetModifiers() == wxMOD_CONTROL)
        {
            if (m_rectScroller.Contains(pos) && m_totalDuration > m_minScrollerVisibleDuration)
            {
                // Scroller zoom is disabled
            }
        }
        else
        {
            int scrollDeltaTime = deltaSteps * (event.GetModifiers() == wxMOD_SHIFT ? GetVisibleDuration() / 4
                : wxMin(15, GetVisibleDuration() / 10));
            if (scrollDeltaTime == 0 && deltaSteps != 0) scrollDeltaTime = deltaSteps > 0 ? 1 : -1;
            SetFirstVisibleTime(GetFirstVisibleTime() - scrollDeltaTime);
        }
    }
    if (event.Dragging() || event.IsButton() || event.GetWheelRotation() != 0) {
        Refresh();
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnEnterElement(ElementType type)
{
    wxLogDebug(wxT(">[OnEnterElement] Trying to enter Type: %d. m_selectedElement: %d"), type, m_selectedElement);
    switch (type)
    {
    case ET_VISIBLE_FRAME:
    case ET_VISIBLE_FRAME_LEFT:
    case ET_VISIBLE_FRAME_RIGHT:
        if (m_selectedElement != type) m_stateVisibleFrame = TimelineElementState::Hover;
        break;
    case ET_LEFT_ARROW:
        if (m_stateLeftArrow != TimelineElementState::Disabled && m_selectedElement != ET_LEFT_ARROW)
            m_stateLeftArrow = TimelineElementState::Hover;
        break;
    case ET_RIGHT_ARROW:
        if (m_stateRightArrow != TimelineElementState::Disabled && m_selectedElement != ET_RIGHT_ARROW)
            m_stateRightArrow = TimelineElementState::Hover;
        break;
    case ET_TIMELINE:
        m_activeTask = m_items.end();
        break;
    default:
        break;
    }

    if (type >= ET_TIMELINE_ITEM && type <= ET_TIMELINE_ITEM_MAX)
    {
        if (m_activeTask != m_items.end() && m_selectedElement != type)
        {
             m_activeTask->State = TimelineElementState::Hover;
             wxLogDebug(wxT(">[OnEnterElement] TIMELINE_ITEM family (%d) hovered."), type);
        }
        else if (m_activeTask == m_items.end())
        {
            wxLogDebug(wxT(">[OnEnterElement] WARNING: type is TIMELINE_ITEM family but m_activeTask is end(). Type: %d"), type);
        }
    }
    ChangeLastElement(type);
    Refresh();
    wxLogDebug(wxT("<[OnEnterElement] Entered Type: %d. m_lastElement is now: %d"), type, m_lastElement);
}

template<typename T>
void wxTimelineCtrl<T>::OnLeaveElement(ElementType type)
{
    wxLogDebug(wxT(">[OnLeaveElement] Leaving Type: %d. m_selectedElement: %d"), type, m_selectedElement);
    switch (type)
    {
    case ET_VISIBLE_FRAME:
    case ET_VISIBLE_FRAME_LEFT:
    case ET_VISIBLE_FRAME_RIGHT:
        if (m_selectedElement != type) m_stateVisibleFrame = TimelineElementState::Normal;
        break;
    case ET_LEFT_ARROW:
        if (m_stateLeftArrow != TimelineElementState::Disabled && m_selectedElement != ET_LEFT_ARROW)
            m_stateLeftArrow = TimelineElementState::Normal;
        break;
    case ET_RIGHT_ARROW:
        if (m_stateRightArrow != TimelineElementState::Disabled && m_selectedElement != ET_RIGHT_ARROW)
            m_stateRightArrow = TimelineElementState::Normal;
        break;
    default:
        break;
    }
    if (type >= ET_TIMELINE_ITEM && type <= ET_TIMELINE_ITEM_MAX)
    {
        if (m_activeTask != m_items.end() && m_selectedElement != type)
        {
            m_activeTask->State = TimelineElementState::Normal;
            wxLogDebug(wxT(">[OnLeaveElement] TIMELINE_ITEM family (%d) unhovered."), type);
        }
    }
    Refresh();
    wxLogDebug(wxT("<[OnLeaveElement] Left Type: %d."), type);
}

template<typename T>
void wxTimelineCtrl<T>::OnMouseCaptureLost(wxMouseCaptureLostEvent&)
{
    if (m_mouseCaptured)
    {
        ReleaseMouse();
        m_mouseCaptured = false;
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineDown(const wxPoint& pos, ElementType type)
{
    if (m_activeTask == m_items.end()) return;
    if (m_activeTask < m_items.begin() || m_activeTask >= m_items.end()) { m_activeTask = m_items.end(); return; }
    size_t index = std::distance(m_items.begin(), m_activeTask);
    if (index >= m_items.size()) return;
    if (!m_activeTask->Data) return;

    bool isSelectionOperation = wxGetKeyState(WXK_CONTROL);
    if (isSelectionOperation)
    {
        ToggleItemSelection(index);
        SendItemEvent(wxEVT_TIMELINE_SELECTION, index);
        return;
    }
    else
    {
        if (!IsItemSelected(index)) SelectItem(index);
        else if (m_selectedItems.size() > 1) SelectItem(index);
        SendItemEvent(wxEVT_TIMELINE_SELECTION, index);
    }

    m_mouseDown = true;
    m_selectedElement = type;
    if (m_activeTask != m_items.end() && m_activeTask->Data)
    {
        m_activeTask->State = TimelineElementState::Pressed;
        m_dragFirstVisibleTime = m_activeTask->Data->GetStartTime();
        m_dragVisibleDuration = m_activeTask->Data->GetDuration();
        m_ptStartPos = pos;
    }
    if (!HasCapture())
    {
        CaptureMouse();
        m_mouseCaptured = true;
    }
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineUp(const wxPoint& pos, ElementType type)
{
    if (m_activeTask != m_items.end())
    {
        m_activeTask->State = TimelineElementState::Normal;
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineDrag(const wxPoint& pos, ElementType type)
{
    OnTimelineItemMove(pos);
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineItemMove(const wxPoint& pos)
{
    if (m_activeTask == m_items.end() || !m_activeTask->Data) return;
    int dx = pos.x - m_ptStartPos.x;
    int dt = TimelineCoordToTime(dx);
    int newStart = m_dragFirstVisibleTime + dt;
    int duration = m_dragVisibleDuration;

    // Определяем направление движения от исходной позиции
    bool movingRight = (dt > 0);

    if (movingRight)
    {
        int collisionLimit = m_totalDuration;
        for (auto it = m_items.begin(); it != m_items.end(); ++it)
        {
            if (it == m_activeTask || !it->Data) continue;

            // Рассматриваем элементы, которые являются потенциальными препятствиями при движении вправо
            if (it->Data->GetStartTime() >= m_dragFirstVisibleTime)
            {
                // Если мы бы пересеклись с элементом
                if (newStart + duration > it->Data->GetStartTime())
                {
                    collisionLimit = wxMin(collisionLimit, it->Data->GetStartTime());
                }
            }
        }

        if (newStart + duration > collisionLimit)
        {
            if (collisionLimit < m_totalDuration) // Ограничено другим элементом
            {
                newStart = collisionLimit - duration;
            }
            else // Ограничено концом временной шкалы, можно расширять
            {
                SetTotalDuration(newStart + duration);
            }
        }
    }
    else // движемся влево или не движемся
    {
        int collisionLimit = 0;
        for (auto it = m_items.begin(); it != m_items.end(); ++it)
        {
            if (it == m_activeTask || !it->Data) continue;

            // Рассматриваем элементы, которые являются потенциальными препятствиями при движении влево
            if (it->Data->GetEndTime() <= m_dragFirstVisibleTime)
            {
                // Если мы бы пересеклись с элементом
                if (newStart < it->Data->GetEndTime())
                {
                    collisionLimit = wxMax(collisionLimit, it->Data->GetEndTime());
                }
            }
        }
        if (newStart < collisionLimit)
        {
            newStart = collisionLimit;
        }
    }

    // Ограничиваем позицию границами временной шкалы
    if (newStart < 0) newStart = 0;
    if (newStart + duration > m_totalDuration)
    {
        newStart = m_totalDuration - duration;
    }
    if (newStart < 0) newStart = 0; // Повторная проверка, если длительность > общей длительности

    if (newStart != m_activeTask->Data->GetStartTime())
    {
        m_activeTask->Data->SetStartTime(newStart);
        m_activeTask->Data->SetDuration(duration);
        SendItemEvent(wxEVT_SCROLL_THUMBTRACK, std::distance(m_items.begin(), m_activeTask));
        RecalcItems();
        Refresh();
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineItemChangeLeft(const wxPoint& pos)
{
    if (m_activeTask == m_items.end() || !m_activeTask->Data) return;
    int dx = pos.x - m_ptStartPos.x;
    int dt = TimelineCoordToTime(dx);
    int newStart = m_dragFirstVisibleTime + dt;
    int end = m_dragFirstVisibleTime + m_dragVisibleDuration;
    if (newStart < 0) newStart = 0;
    if (newStart > end - 1) newStart = end - 1;
    if (m_activeTask != m_items.begin())
    {
        auto prev = m_activeTask - 1;
        if (prev->Data && newStart < prev->Data->GetStartTime() + prev->Data->GetDuration())
            newStart = prev->Data->GetStartTime() + prev->Data->GetDuration();
    }
    if (newStart != m_activeTask->Data->GetStartTime())
    {
        m_activeTask->Data->SetStartTime(newStart);
        m_activeTask->Data->SetDuration(end - newStart);
        SendItemEvent(wxEVT_SCROLL_THUMBTRACK, std::distance(m_items.begin(), m_activeTask));
        RecalcItems();
        Refresh();
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnTimelineItemChangeRight(const wxPoint& pos)
{
    if (m_activeTask == m_items.end() || !m_activeTask->Data) return;
    int dx = pos.x - m_ptStartPos.x;
    int dt = TimelineCoordToTime(dx);
    int newDuration = m_dragVisibleDuration + dt;
    if (newDuration < 1) newDuration = 1;
    if (m_activeTask + 1 != m_items.end())
    {
        auto next = m_activeTask + 1;
        if (next->Data && m_activeTask->Data->GetStartTime() + newDuration > next->Data->GetStartTime())
            newDuration = next->Data->GetStartTime() - m_activeTask->Data->GetStartTime();
    }
    if (newDuration != m_activeTask->Data->GetDuration())
    {
        m_activeTask->Data->SetDuration(newDuration);
        SendItemEvent(wxEVT_SCROLL_THUMBTRACK, std::distance(m_items.begin(), m_activeTask));
        RecalcItems();
        Refresh();
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnTimer(wxTimerEvent& event)
{
    if (event.GetId() == m_timerMove.GetId())
    {
        if (!m_mouseDown || (m_selectedElement != ET_LEFT_ARROW && m_selectedElement != ET_RIGHT_ARROW))
        {
            m_timerMove.Stop();
            m_moveDirection = 0;
            return;
        }

        if (m_timerMove.IsOneShot())
        {
            m_timerMove.Start(50);
        }

        SetFirstVisibleTime(GetFirstVisibleTime() + m_moveDirection);

        const int maxScrollSpeed = GetVisibleDuration() / 4;
        if (m_moveDirection > 0)
        {
            m_moveDirection = wxMin(m_moveDirection + 1, maxScrollSpeed > 0 ? maxScrollSpeed : 15);
        }
        else
        {
            m_moveDirection = wxMax(m_moveDirection - 1, -(maxScrollSpeed > 0 ? maxScrollSpeed : 15));
        }
    }
    else
    {
        event.Skip();
    }
}

template<typename T>
void wxTimelineCtrl<T>::ShowContextMenu(const wxPoint& screenPos)
{
    wxMenu menu;
    bool menuHasItems = false;

    if (m_contextMenuItemIndex != -1)
    {
        if (m_contextMenuItemIndex >= 0 && m_contextMenuItemIndex < (int)m_items.size())
        {
            menu.Append(ID_TIMELINE_DELETE_SCROLLER_ITEM, wxString::Format("Delete Element \"%s\"", m_items[m_contextMenuItemIndex].GetItemName()));
            menuHasItems = true;
        }
        else 
        {
            m_contextMenuItemIndex = -1; 
        }
    }
    else if (!m_selectedItems.empty())
    {
        if (m_selectedItems.size() == 1)
        {
            size_t idx = m_selectedItems[0];
            if (idx < m_items.size())
            {
                menu.Append(ID_TIMELINE_DELETE, wxString::Format("Удалить элемент \"%s\"", m_items[idx].GetItemName()));
            }
            else
            {
                menu.Append(ID_TIMELINE_DELETE, "Удалить выбранный элемент");
            }
        }
        else
        {
            menu.Append(ID_TIMELINE_DELETE, wxString::Format("Удалить %zu элемента(ов)", m_selectedItems.size()));
        }
        menuHasItems = true;
    }


    if (menuHasItems)
    {
        PopupMenu(&menu, ScreenToClient(screenPos));
    }
}


template<typename T>
void wxTimelineCtrl<T>::OnKeyDown(wxKeyEvent& event)
{
    int timeDelta = 0;
    bool processed = true;

    switch (event.GetKeyCode())
    {
    case WXK_TAB:
        Navigate(!event.ShiftDown());
        break;

    case WXK_LEFT:
    case WXK_RIGHT:
    {
        int direction = (event.GetKeyCode() == WXK_LEFT) ? -1 : 1;
        int stepSize;

        if (event.GetModifiers() == wxMOD_CONTROL) {
            stepSize = 1;
        }
        else if (event.GetModifiers() == wxMOD_SHIFT) {
            stepSize = wxMax(1, GetVisibleDuration() / 4);
        }
        else {
            stepSize = wxMax(1, GetVisibleDuration() / 10);
            stepSize = wxMin(stepSize, 15);
        }
        timeDelta = direction * stepSize;
    }
    break;

    case WXK_PAGEUP:
        timeDelta = -GetVisibleDuration();
        break;
    case WXK_PAGEDOWN:
        timeDelta = GetVisibleDuration();
        break;

    case WXK_HOME:
        SetFirstVisibleTime(0);
        break;
    case WXK_END:
    {
        int targetFirstVisibleTime = m_totalDuration - m_visibleDuration;
        SetFirstVisibleTime(targetFirstVisibleTime);
    }
    break;

    case WXK_NUMPAD_SUBTRACT:
    case '-':
    case WXK_NUMPAD_ADD:
    case '=':
    case '+':
    {
        int zoomDirection = (event.GetKeyCode() == WXK_NUMPAD_ADD || event.GetKeyCode() == '=' || event.GetKeyCode() == '+') ? -1 : 1;
        int zoomAmount;
        if (event.GetModifiers() == wxMOD_SHIFT)
            zoomAmount = wxMax(1, GetVisibleDuration() / 2);
        else if (event.GetModifiers() == wxMOD_CONTROL)
            zoomAmount = wxMax(1, GetVisibleDuration() / 20);
        else
            zoomAmount = wxMax(1, GetVisibleDuration() / 10);

        zoomAmount = wxMax(1, zoomAmount);
        Zoom(zoomDirection * zoomAmount);
    }
    break;

    case 'Z':
        if (event.GetModifiers() == wxMOD_CONTROL) ZoomToSelection();
        else processed = false;
        break;
    case 'A':
        if (event.GetModifiers() == wxMOD_CONTROL) ShowAllTimeline();
        else processed = false;
        break;
    case '1': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_SECONDS_10); else processed = false; break;
    case '2': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_SECONDS_30); else processed = false; break;
    case '3': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_MINUTE_1); else processed = false; break;
    case '4': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_MINUTES_2); else processed = false; break;
    case '5': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_MINUTES_5); else processed = false; break;
    case '6': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_MINUTES_10); else processed = false; break;
    case '0': if (event.GetModifiers() == wxMOD_CONTROL) SetZoomPreset(ZOOM_ALL); else processed = false; break;

    case WXK_ESCAPE:
        ClearSelection();
        break;
    case WXK_DELETE:
        if (!m_selectedItems.empty()) RemoveSelectedItems();
        break;

    default:
        processed = false;
        break;
    }

    if (timeDelta != 0)
    {
        SetFirstVisibleTime(GetFirstVisibleTime() + timeDelta);
    }

    if (!processed)
    {
        event.Skip();
    }
}

template<typename T>
void wxTimelineCtrl<T>::CalcArrowsState()
{
    if (m_firstVisibleTime <= 0) m_stateLeftArrow = TimelineElementState::Disabled;
    else if (m_stateLeftArrow == TimelineElementState::Disabled)
        m_stateLeftArrow = m_lastElement == ET_LEFT_ARROW ? TimelineElementState::Hover : TimelineElementState::Normal;
    if (GetLastVisibleTime() >= m_totalDuration) m_stateRightArrow = TimelineElementState::Disabled;
    else if (m_stateRightArrow == TimelineElementState::Disabled)
        m_stateRightArrow = m_lastElement == ET_RIGHT_ARROW ? TimelineElementState::Hover : TimelineElementState::Normal;
}

template<typename T>
void wxTimelineCtrl<T>::TimeChanged()
{
    RecalcVisibleFrame();
    RecalcItems();
    Refresh();
}


template<typename T>
typename wxTimelineCtrl<T>::ElementType wxTimelineCtrl<T>::GetElementFromPos(const wxPoint& pos)
{
    ElementType type = ET_NONE;
    m_hoveredScrollerItemIndex = -1;

    if (m_rectScroller.Contains(pos))
    {
        type = ET_SCROLLER;
        if (m_rectLeftArrow.Contains(pos)) type = ET_LEFT_ARROW;
        else if (m_rectRightArrow.Contains(pos)) type = ET_RIGHT_ARROW;
        else if (m_rectVisibleFrameLeft.Contains(pos)) type = ET_VISIBLE_FRAME_LEFT;
        else if (m_rectVisibleFrameRight.Contains(pos)) type = ET_VISIBLE_FRAME_RIGHT;
        else if (m_rectVisibleFrame.Contains(pos)) type = ET_VISIBLE_FRAME;
        else if (m_rectScrollerTrack.Contains(pos))
        {
            for (size_t i = 0; i < m_items.size(); ++i)
            {
                auto& item = m_items[i];
                if (!item.Data) continue;
                int itemStartTime = item.Data->GetStartTime();
                int itemEndTime = item.Data->GetEndTime();
                if (itemEndTime <= m_scrollerFirstVisibleTime || itemStartTime >= m_scrollerFirstVisibleTime + m_scrollerVisibleDuration)
                    continue;
                int itemX_start_in_track = ScrollerTimeToCoord(itemStartTime);
                int itemX_end_in_track = ScrollerTimeToCoord(itemEndTime);
                itemX_start_in_track = wxMax(0, itemX_start_in_track);
                itemX_end_in_track = wxMin(m_rectScrollerTrack.width, itemX_end_in_track);
                if (itemX_end_in_track > itemX_start_in_track)
                {
                    wxRect itemRectInScroller = m_rectScrollerTrack;
                    itemRectInScroller.x = m_rectScrollerTrack.x + itemX_start_in_track;
                    itemRectInScroller.width = itemX_end_in_track - itemX_start_in_track;
                    if (itemRectInScroller.Contains(pos))
                    {
                        m_hoveredScrollerItemIndex = i;
                        type = ET_SCROLLER_ITEM_DRAG;
                        break;
                    }
                }
            }
            if (type != ET_SCROLLER_ITEM_DRAG)
            {
                type = ET_SCROLLER;
            }
        }
    }
    else if (m_rectTimeline.Contains(pos))
    {
        type = ET_TIMELINE;
        if (!m_items.empty())
        {
            int i = ET_TIMELINE_ITEM;
            m_activeTask = m_items.end();
            for (auto it = m_items.begin(); it != m_items.end(); ++it, i += 3)
            {
                if (it->Data && !it->Rect.IsEmpty() && it->Rect.Contains(pos))
                {
                    m_activeTask = it;
                    return static_cast<ElementType>(i);
                }
            }
        }
    }
    return type;
}

template<typename T>
void wxTimelineCtrl<T>::ChangeLastElement(ElementType type)
{
    m_lastElement = type;
    SetCursor(GetCursorFromType(type));
}

template<typename T>
void wxTimelineCtrl<T>::OnMouseDown(ElementType type, const wxPoint& pos)
{
    wxLogDebug(wxT(">>[OnMouseDown] Type: %d at (%d,%d). m_mouseCaptured: %d, m_selectedElement (before): %d"), type, pos.x, pos.y, m_mouseCaptured, m_selectedElement);
    m_dragCurrentPos = m_dragStartPos = pos;
    m_mouseDown = false;
    m_selectedElement = type;

    switch (type)
    {
    case ET_VISIBLE_FRAME:
    case ET_VISIBLE_FRAME_LEFT:
    case ET_VISIBLE_FRAME_RIGHT:
        m_mouseDown = true;
        OnVisibleFrameDown(pos, type);
        break;
    case ET_LEFT_ARROW:
        m_mouseDown = true;
        OnArrowDown(true);
        break;
    case ET_RIGHT_ARROW:
        m_mouseDown = true;
        OnArrowDown(false);
        break;
    case ET_SCROLLER:
        OnScrollerDown(pos);
        break;
    case ET_TIMELINE:
        ClearSelection();
        wxLogDebug(wxT(">>[OnMouseDown] Click on empty TIMELINE area. Selection Cleared."));
        break;
    default:
        if (type >= ET_TIMELINE_ITEM && type <= ET_TIMELINE_ITEM_MAX)
        {
            OnTimelineDown(pos, type);
        }
        break;
    }
    Refresh();
    wxLogDebug(wxT("<<[OnMouseDown] END. m_selectedElement: %d, m_mouseDown: %d, m_mouseCaptured: %d"), m_selectedElement, m_mouseDown, m_mouseCaptured);
}


template<typename T>
void wxTimelineCtrl<T>::OnMouseDrag(ElementType type, const wxPoint& pos)
{
    m_dragCurrentPos = pos; 

    if (m_isDraggingDetachedItem)
    {
        wxPoint screenMousePos = ClientToScreen(pos);
        m_detachedDragItemScreenPos = screenMousePos - m_cursorToDetachedVisualOffset;

        if (m_pFloatingItemWin) {
            m_pFloatingItemWin->Move(m_detachedDragItemScreenPos);
        }

        m_dropIndicatorRect = wxRect();
        m_dropIndicatorRectScroller = wxRect();
        m_dragPreviewTime = -1;
        m_isSnapping = false;

        if (m_detachedDragItemOriginalIndex < 0 || m_detachedDragItemOriginalIndex >= (int)m_items.size() || !m_items[m_detachedDragItemOriginalIndex].Data)
        {
            Refresh();
            return;
        }
        int duration = m_items[m_detachedDragItemOriginalIndex].Data->GetDuration();

        int mouseTime = -1;
        if (m_rectScrollerTrack.Contains(pos)) {
            mouseTime = ScrollerCoordToTime(pos.x - m_rectScrollerTrack.x);
        } else if (m_rectTimelineTrack.Contains(pos)) {
            mouseTime = TimelineCoordToTime(pos.x - m_rectTimelineTrack.x) + m_firstVisibleTime;
        }

        if (mouseTime != -1)
        {
            int rawStartTime = mouseTime - m_dragScrollerItemInitialClickTimeOffset;
            int previewStartTime = rawStartTime;

            // Snapping logic to any item's start or end
            const int snapThresholdPixels = 10;
            int snapThresholdTime = TimelineCoordToTime(snapThresholdPixels);
            int bestSnapTime = -1;
            int minSnapDist = snapThresholdTime + 1;

            for (size_t i = 0; i < m_items.size(); ++i)
            {
                if ((int)i == m_detachedDragItemOriginalIndex || !m_items[i].Data) continue;
                
                int itemStart = m_items[i].Data->GetStartTime();
                int itemEnd = m_items[i].Data->GetEndTime();

                // Snap to start of other item
                if (abs(previewStartTime - itemStart) < minSnapDist) {
                    minSnapDist = abs(previewStartTime - itemStart);
                    bestSnapTime = itemStart;
                }
                // Snap to end of other item
                if (abs(previewStartTime - itemEnd) < minSnapDist) {
                    minSnapDist = abs(previewStartTime - itemEnd);
                    bestSnapTime = itemEnd;
                }
                // Snap end of dragged item to start of other item
                if (abs((previewStartTime + duration) - itemStart) < minSnapDist) {
                    minSnapDist = abs((previewStartTime + duration) - itemStart);
                    bestSnapTime = itemStart - duration;
                }
                // Snap end of dragged item to end of other item
                if (abs((previewStartTime + duration) - itemEnd) < minSnapDist) {
                    minSnapDist = abs((previewStartTime + duration) - itemEnd);
                    bestSnapTime = itemEnd - duration;
                }
            }

            if (bestSnapTime != -1) {
                previewStartTime = bestSnapTime;
                m_isSnapping = true;
            }

            // Final clamping to total duration (for preview only)
            previewStartTime = wxMax(0, previewStartTime);
            if (previewStartTime + duration > m_totalDuration) {
                previewStartTime = m_totalDuration - duration;
            }
            if (previewStartTime < 0) previewStartTime = 0;

            m_dragPreviewTime = previewStartTime;
            int previewEndTime = previewStartTime + duration;

            // Calculate indicator for main timeline
            int visibleStart = m_firstVisibleTime;
            int visibleEnd = m_firstVisibleTime + m_visibleDuration;
            if (!(previewEndTime <= visibleStart || previewStartTime >= visibleEnd))
            {
                int x1 = TimelineTimeToCoord(previewStartTime - visibleStart);
                int x2 = TimelineTimeToCoord(previewEndTime - visibleStart);
                if (x2 <= x1) x2 = x1 + 1;
                m_dropIndicatorRect = wxRect(m_rectTimelineTrack.x + x1, m_rectTimelineTrack.y, x2 - x1, m_rectTimelineTrack.height);
            }

            // Calculate indicator for scroller
            int scrollerVisibleStart = m_scrollerFirstVisibleTime;
            int scrollerVisibleEnd = m_scrollerFirstVisibleTime + m_scrollerVisibleDuration;
            if (!(previewEndTime <= scrollerVisibleStart || previewStartTime >= scrollerVisibleEnd))
            {
                int sx1 = ScrollerTimeToCoord(previewStartTime);
                int sx2 = ScrollerTimeToCoord(previewEndTime);
                if (sx2 <= sx1) sx2 = sx1 + 1;
                m_dropIndicatorRectScroller = wxRect(m_rectScrollerTrack.x + sx1, m_rectScrollerTrack.y, sx2 - sx1, m_rectScrollerTrack.height);
            }
        }

        Refresh();
        return;
    }

    switch (type)
    {
    case ET_VISIBLE_FRAME:
    case ET_VISIBLE_FRAME_LEFT:
    case ET_VISIBLE_FRAME_RIGHT: 
        OnVisibleFrameDrag(pos); 
        break;
    default: break;
    }
    if (type >= ET_TIMELINE_ITEM && type <= ET_TIMELINE_ITEM_MAX) OnTimelineDrag(pos, type);
    // Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::OnMouseUp(ElementType typeFromMouseDown, const wxPoint& pos)
{
    if (m_isDraggingDetachedItem)
    {
        if (m_pFloatingItemWin) {
            m_pFloatingItemWin->Destroy();
            m_pFloatingItemWin = nullptr;
        }

        if (m_detachedDragItemOriginalIndex != -1 && m_dragPreviewTime != -1)
        {
            TimelineItem<T>& draggedItem = m_items[m_detachedDragItemOriginalIndex];
            if (draggedItem.Data)
            {
                int duration = draggedItem.Data->GetDuration();
                int newStartTime = m_dragPreviewTime;
                int newEndTime = newStartTime + duration;

                // Find first conflict to determine shift
                int firstConflictStartTime = -1;
                for (size_t i = 0; i < m_items.size(); ++i)
                {
                    if ((int)i == m_detachedDragItemOriginalIndex || !m_items[i].Data) continue;
                    
                    int itemStart = m_items[i].Data->GetStartTime();
                    int itemEnd = m_items[i].Data->GetEndTime();

                    // Check for overlap
                    if (itemStart < newEndTime && itemEnd > newStartTime)
                    {
                        if (firstConflictStartTime == -1 || itemStart < firstConflictStartTime)
                        {
                            firstConflictStartTime = itemStart;
                        }
                    }
                }

                if (firstConflictStartTime != -1)
                {
                    int shiftAmount = newEndTime - firstConflictStartTime;
                    if (shiftAmount > 0)
                    {
                        // Collect items to shift
                        wxVector<size_t> itemsToShiftIndices;
                        for (size_t i = 0; i < m_items.size(); ++i)
                        {
                            if ((int)i != m_detachedDragItemOriginalIndex && m_items[i].Data && m_items[i].Data->GetStartTime() >= firstConflictStartTime)
                            {
                                itemsToShiftIndices.push_back(i);
                            }
                        }

                        // Sort them by start time to shift correctly
                        std::sort(itemsToShiftIndices.begin(), itemsToShiftIndices.end(), [&](size_t a, size_t b){
                            return m_items[a].Data->GetStartTime() < m_items[b].Data->GetStartTime();
                        });

                        // Shift items
                        for (size_t index : itemsToShiftIndices)
                        {
                            m_items[index].Data->SetStartTime(m_items[index].Data->GetStartTime() + shiftAmount);
                            m_items[index].Data->SetEndTime(m_items[index].Data->GetEndTime() + shiftAmount);
                        }
                    }
                }
                
                draggedItem.Data->SetStartTime(newStartTime);
                draggedItem.Data->SetEndTime(newEndTime);
                UpdateTotalDurationForItems();
            }
        }

        m_isDraggingDetachedItem = false;
        m_detachedDragItemOriginalIndex = -1;
        m_showOriginalPositionPlaceholder = false;
        m_dragPreviewTime = -1;
        m_dropIndicatorRect = wxRect();
        m_dropIndicatorRectScroller = wxRect();

        if (m_mouseCaptured && HasCapture()) { ReleaseMouse(); m_mouseCaptured = false; }
        m_mouseDown = false;
        m_selectedElement = ET_NONE;

        RecalcItems();
        Refresh();

        ElementType typeUnderMouseNow = GetElementFromPos(pos);
        if (typeUnderMouseNow != m_lastElement) {
            OnLeaveElement(m_lastElement);
            OnEnterElement(typeUnderMouseNow);
        } else if (m_lastElement != ET_NONE) {
            OnLeaveElement(m_lastElement);
            OnEnterElement(typeUnderMouseNow);
        }
        return;
    }


    switch (typeFromMouseDown)
    {
    case ET_VISIBLE_FRAME:
    case ET_VISIBLE_FRAME_LEFT:
    case ET_VISIBLE_FRAME_RIGHT:
        OnVisibleFrameUp(pos);
        break;
    case ET_LEFT_ARROW:
        OnArrowUp(true);
        break;
    case ET_RIGHT_ARROW:
        OnArrowUp(false);
        break;
    case ET_TIMELINE:
        break;
    default:
        if (typeFromMouseDown >= ET_TIMELINE_ITEM && typeFromMouseDown <= ET_TIMELINE_ITEM_MAX)
        {
            OnTimelineUp(pos, typeFromMouseDown);
            UpdateTotalDurationForItems();
        }
        break;
    }

    if (m_mouseCaptured && HasCapture())
    {
        ReleaseMouse();
        m_mouseCaptured = false;
    }
    else if (HasCapture()) {
        ReleaseMouse();
    }
    
    m_mouseDown = false;


    ElementType previousSelectedElement = m_selectedElement;
    m_selectedElement = ET_NONE;

    ElementType typeUnderMouseNow = GetElementFromPos(pos);
    if (typeUnderMouseNow != m_lastElement)
    {
        OnEnterElement(typeUnderMouseNow);
    }
    else if (m_lastElement != ET_NONE && m_lastElement != ET_SCROLLER_ITEM_DRAG && !(m_lastElement >= ET_TIMELINE_ITEM && m_lastElement <= ET_TIMELINE_ITEM_MAX && m_activeTask == m_items.end()))
    {
        OnLeaveElement(m_lastElement);
        OnEnterElement(typeUnderMouseNow);
    }
}


template<typename T>
wxCursor wxTimelineCtrl<T>::GetCursorFromType(ElementType type)
{
    switch (type)
    {
    case ET_SCROLLER: return wxCURSOR_HAND;
    case ET_VISIBLE_FRAME: return wxCURSOR_SIZING;
    case ET_VISIBLE_FRAME_LEFT: case ET_VISIBLE_FRAME_RIGHT: return wxCURSOR_SIZEWE;
    case ET_SCROLLER_ITEM_DRAG: return wxCURSOR_HAND;
    case ET_RIGHT_ARROW: break;
    case ET_TIMELINE: break;
    default: break;
    }
    if (type >= ET_TIMELINE_ITEM && type <= ET_TIMELINE_ITEM_MAX)
    {
        return wxCURSOR_SIZING;
    }
    return wxCURSOR_ARROW;
}

template<typename T>
void wxTimelineCtrl<T>::OnVisibleFrameUp(const wxPoint& pos)
{
    if (m_mouseCaptured && HasCapture())
    {
        ReleaseMouse();
        m_mouseCaptured = false;
    }
    ElementType currentElement = GetElementFromPos(pos);
    if (currentElement == ET_VISIBLE_FRAME || currentElement == ET_VISIBLE_FRAME_LEFT || currentElement == ET_VISIBLE_FRAME_RIGHT)
        m_stateVisibleFrame = TimelineElementState::Hover;
    else m_stateVisibleFrame = TimelineElementState::Normal;
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::OnVisibleFrameDown(const wxPoint& pos, ElementType type)
{
    m_mouseDown = true;
    m_selectedElement = type;
    m_stateVisibleFrame = TimelineElementState::Pressed;
    m_dragFirstVisibleTime = m_firstVisibleTime;
    m_dragVisibleDuration = m_visibleDuration;
    m_ptStartPos = pos;
    m_ptEndPos = pos;
    if (!HasCapture())
    {
        CaptureMouse();
        m_mouseCaptured = true;
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnVisibleFrameDrag(const wxPoint& pos)
{
    m_ptEndPos = pos;
    int dx = m_ptEndPos.x - m_ptStartPos.x;
    int dt = 0;

    if (m_rectScrollerTrack.width > 0 && m_scrollerVisibleDuration > 0) {
        dt = (dx * m_scrollerVisibleDuration) / m_rectScrollerTrack.width;
    }

    int originalMainFirstVisibleTime = m_firstVisibleTime;
    int originalMainVisibleDuration = m_visibleDuration;

    int newCalcFirstVisibleTime = m_firstVisibleTime;
    int newCalcVisibleDuration = m_visibleDuration;

    switch (m_selectedElement)
    {
    case ET_VISIBLE_FRAME:
    {
        newCalcFirstVisibleTime = m_dragFirstVisibleTime + dt;
    }
    break;

    case ET_VISIBLE_FRAME_LEFT:
    {
        int tentativeNewStartTime = m_dragFirstVisibleTime + dt;
        int fixedEndTime = m_dragFirstVisibleTime + m_dragVisibleDuration;

        if (fixedEndTime - tentativeNewStartTime < m_minVisibleDuration)
        {
            tentativeNewStartTime = fixedEndTime - m_minVisibleDuration;
        }
        
        tentativeNewStartTime = wxMax(0, tentativeNewStartTime);
        tentativeNewStartTime = wxMin(tentativeNewStartTime, m_totalDuration - m_minVisibleDuration);

        newCalcFirstVisibleTime = tentativeNewStartTime;
        newCalcVisibleDuration = fixedEndTime - tentativeNewStartTime;
    }
    break;

    case ET_VISIBLE_FRAME_RIGHT:
    {
        int fixedStartTime = m_dragFirstVisibleTime;
        int tentativeNewEndTime = (m_dragFirstVisibleTime + m_dragVisibleDuration) + dt;

        if (tentativeNewEndTime - fixedStartTime < m_minVisibleDuration)
        {
            tentativeNewEndTime = fixedStartTime + m_minVisibleDuration;
        }

        tentativeNewEndTime = wxMin(tentativeNewEndTime, m_totalDuration);
        tentativeNewEndTime = wxMax(tentativeNewEndTime, m_minVisibleDuration);

        newCalcFirstVisibleTime = fixedStartTime;
        newCalcVisibleDuration = tentativeNewEndTime - fixedStartTime;
    }
    break;

    default:
        return;
    }

    newCalcVisibleDuration = wxMax(m_minVisibleDuration, newCalcVisibleDuration);
    newCalcVisibleDuration = wxMin(newCalcVisibleDuration, m_maxVisibleDuration);
    newCalcVisibleDuration = wxMin(newCalcVisibleDuration, m_totalDuration);

    if (newCalcFirstVisibleTime < 0)
    {
        newCalcFirstVisibleTime = 0;
    }
    if (newCalcFirstVisibleTime + newCalcVisibleDuration > m_totalDuration)
    {
        if (m_selectedElement == ET_VISIBLE_FRAME || m_selectedElement == ET_VISIBLE_FRAME_RIGHT)
        {
            newCalcFirstVisibleTime = m_totalDuration - newCalcVisibleDuration;
        }
    }
    if (newCalcFirstVisibleTime < 0) newCalcFirstVisibleTime = 0;

    m_firstVisibleTime = newCalcFirstVisibleTime;
    m_visibleDuration = newCalcVisibleDuration;

    bool scrollerViewChanged = false;
    int newScrollerFirstVisibleTime = m_scrollerFirstVisibleTime;

    if (m_firstVisibleTime < m_scrollerFirstVisibleTime)
    {
        newScrollerFirstVisibleTime = m_firstVisibleTime;
        scrollerViewChanged = true;
    }
    else if (m_firstVisibleTime + m_visibleDuration > m_scrollerFirstVisibleTime + m_scrollerVisibleDuration)
    {
        newScrollerFirstVisibleTime = (m_firstVisibleTime + m_visibleDuration) - m_scrollerVisibleDuration;
        scrollerViewChanged = true;
    }

    if (scrollerViewChanged)
    {
        if (newScrollerFirstVisibleTime < 0) newScrollerFirstVisibleTime = 0;
        if (newScrollerFirstVisibleTime + m_scrollerVisibleDuration > m_totalDuration)
        {
            newScrollerFirstVisibleTime = m_totalDuration - m_scrollerVisibleDuration;
        }
        if (newScrollerFirstVisibleTime < 0) newScrollerFirstVisibleTime = 0;
        m_scrollerFirstVisibleTime = newScrollerFirstVisibleTime;
    }

    bool mainViewActuallyChanged = (m_firstVisibleTime != originalMainFirstVisibleTime || m_visibleDuration != originalMainVisibleDuration);

    if (mainViewActuallyChanged || scrollerViewChanged)
    {
        AdjustMainViewToScrollerView();
        CalcArrowsState();
        RecalcVisibleFrame();
        RecalcItems();
        Refresh();
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnScrollerDown(const wxPoint& pos)
{
    // Do nothing to prevent teleporting and dragging the visible frame on scroller track click.
    // The frame can still be dragged by clicking on it directly.
}

template<typename T>
void wxTimelineCtrl<T>::OnArrowDown(bool isLeft)
{
    TimelineElementState& state = isLeft ? m_stateLeftArrow : m_stateRightArrow;

    if (state != TimelineElementState::Disabled)
    {
        state = TimelineElementState::Pressed;

        if (!HasCapture())
        {
            CaptureMouse();
            m_mouseCaptured = true;
        }

        m_moveDirection = isLeft ? -1 : 1;
        SetFirstVisibleTime(GetFirstVisibleTime() + m_moveDirection);
        m_timerMove.Start(250, wxTIMER_ONE_SHOT);
    }
}

template<typename T>
void wxTimelineCtrl<T>::OnArrowUp(bool isLeft)
{
    TimelineElementState& state = isLeft ? m_stateLeftArrow : m_stateRightArrow;

    if (state == TimelineElementState::Pressed)
    {
    }

    m_moveDirection = 0;
    m_timerMove.Stop();
}

template<typename T>
void wxTimelineCtrl<T>::SendItemEvent(wxEventType eventType, int index)
{
    wxCommandEvent event(eventType, GetId());
    event.SetEventObject(this);
    event.SetInt(index);
    event.SetClientData(index >= 0 && index < static_cast<int>(m_items.size()) ? m_items[index].Data : nullptr);
    ProcessWindowEvent(event);
}

template<typename T>
void wxTimelineCtrl<T>::CalcMaxVisibleDuration()
{
    if (m_items.empty()) {
        m_maxVisibleDuration = m_totalDuration;
        return;
    }
    int minDuration = m_totalDuration;
    for (auto& item : m_items)
    {
        if (item.Data && minDuration > item.Data->GetDuration())
            minDuration = item.Data->GetDuration();
    }
    m_maxVisibleDuration = wxMin(minDuration * m_rectTimelineTrack.width / (m_MinItemSize / 2), m_totalDuration);
    if (m_visibleDuration > m_maxVisibleDuration) SetVisibleDuration(m_maxVisibleDuration);
}

template<typename T>
void wxTimelineCtrl<T>::SetFirstVisibleTime(int seconds)
{
    int oldMainFirstVisibleTime = m_firstVisibleTime;
    int oldScrollerFirstVisibleTime = m_scrollerFirstVisibleTime;

    m_firstVisibleTime = seconds;

    bool scrollerViewNeedsAdjustment = false;
    int targetScrollerFirstVisibleTime = m_scrollerFirstVisibleTime;

    if (m_firstVisibleTime < m_scrollerFirstVisibleTime)
    {
        targetScrollerFirstVisibleTime = m_firstVisibleTime;
        scrollerViewNeedsAdjustment = true;
    }
    else if (m_firstVisibleTime + m_visibleDuration > m_scrollerFirstVisibleTime + m_scrollerVisibleDuration)
    {
        targetScrollerFirstVisibleTime = (m_firstVisibleTime + m_visibleDuration) - m_scrollerVisibleDuration;
        scrollerViewNeedsAdjustment = true;
    }

    if (scrollerViewNeedsAdjustment)
    {
        if (targetScrollerFirstVisibleTime < 0)
        {
            targetScrollerFirstVisibleTime = 0;
        }
        if (targetScrollerFirstVisibleTime + m_scrollerVisibleDuration > m_totalDuration)
        {
            targetScrollerFirstVisibleTime = m_totalDuration - m_scrollerVisibleDuration;
            if (targetScrollerFirstVisibleTime < 0) targetScrollerFirstVisibleTime = 0;
        }
        m_scrollerFirstVisibleTime = targetScrollerFirstVisibleTime;
    }

    AdjustMainViewToScrollerView();
    CalcArrowsState();
    RecalcVisibleFrame();
    RecalcItems();
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::SetVisibleDuration(int seconds)
{
    if (seconds <= 0)
        seconds = m_minVisibleDuration;

    int targetMainVisibleDuration = wxClip(seconds, m_minVisibleDuration, m_maxVisibleDuration);
    targetMainVisibleDuration = wxMin(targetMainVisibleDuration, m_totalDuration);

    int currentMainViewCenterTime = m_firstVisibleTime + m_visibleDuration / 2;

    bool scrollerViewChanged = false;
    int oldScrollerVisibleDuration = m_scrollerVisibleDuration;
    int oldScrollerFirstVisibleTime = m_scrollerFirstVisibleTime;

    if (targetMainVisibleDuration > m_scrollerVisibleDuration)
    {
        int newScrollerDuration = targetMainVisibleDuration;
        newScrollerDuration = wxMin(newScrollerDuration, m_totalDuration);
        newScrollerDuration = wxMax(newScrollerDuration, m_minScrollerVisibleDuration);
        newScrollerDuration = wxMax(newScrollerDuration, targetMainVisibleDuration);

        if (newScrollerDuration != m_scrollerVisibleDuration) {
            int currentScrollerCenterTime = m_scrollerFirstVisibleTime + m_scrollerVisibleDuration / 2;
            m_scrollerVisibleDuration = newScrollerDuration;
            m_scrollerFirstVisibleTime = currentScrollerCenterTime - m_scrollerVisibleDuration / 2;

            m_scrollerFirstVisibleTime = wxClip(m_scrollerFirstVisibleTime, 0, m_totalDuration - m_scrollerVisibleDuration);
            if (m_scrollerFirstVisibleTime < 0) m_scrollerFirstVisibleTime = 0;
            scrollerViewChanged = true;
        }
    }

    m_visibleDuration = targetMainVisibleDuration;
    m_firstVisibleTime = currentMainViewCenterTime - m_visibleDuration / 2;
    AdjustMainViewToScrollerView();
    CalcArrowsState();
    RecalcVisibleFrame();
    RecalcItems();
    Refresh();

    if (scrollerViewChanged && (m_scrollerVisibleDuration != oldScrollerVisibleDuration || m_scrollerFirstVisibleTime != oldScrollerFirstVisibleTime))
    {
    }
}

template<typename T>
int wxTimelineCtrl<T>::ScrollerCoordToTime(int coordInTrack) const
{
    if (m_rectScrollerTrack.width <= 0 || m_scrollerVisibleDuration <= 0)
    {
        return m_scrollerFirstVisibleTime;
    }
    double timePerPixel = static_cast<double>(m_scrollerVisibleDuration) / m_rectScrollerTrack.width;
    return m_scrollerFirstVisibleTime + static_cast<int>(round(coordInTrack * timePerPixel));
}

template<typename T>
int wxTimelineCtrl<T>::ScrollerTimeToCoord(int time)
{
    if (m_scrollerVisibleDuration <= 0 || m_rectScrollerTrack.width <= 0) 
    {
        return 0;
    }
    int timeRelativeToScrollerStart = time - m_scrollerFirstVisibleTime;
    double pixelsPerTime = static_cast<double>(m_rectScrollerTrack.width) / m_scrollerVisibleDuration;
    return static_cast<int>(round(timeRelativeToScrollerStart * pixelsPerTime));
}

template<typename T>
void wxTimelineCtrl<T>::SetTotalDuration(int seconds)
{
    int minAllowableDuration = wxMax(m_minScrollerVisibleDuration, m_minVisibleDuration);
    minAllowableDuration = wxMax(minAllowableDuration, 1);
    seconds = wxMax(seconds, minAllowableDuration);

    if (seconds == m_totalDuration)
        return;

    int oldTotalDuration = m_totalDuration;
    m_totalDuration = seconds;
    m_totalTime = m_totalDuration;

    if (m_scrollerVisibleDuration > m_totalDuration || m_scrollerFirstVisibleTime + m_scrollerVisibleDuration > m_totalDuration)
    {
        m_scrollerVisibleDuration = wxMin(m_scrollerVisibleDuration, m_totalDuration);
        m_scrollerVisibleDuration = wxMax(m_minScrollerVisibleDuration, m_scrollerVisibleDuration);
        m_scrollerFirstVisibleTime = wxMin(m_scrollerFirstVisibleTime, m_totalDuration - m_scrollerVisibleDuration);
        if (m_scrollerFirstVisibleTime < 0) m_scrollerFirstVisibleTime = 0;
    }
    else if (oldTotalDuration > 0 && m_scrollerVisibleDuration == oldTotalDuration && m_totalDuration > oldTotalDuration)
    {
         m_scrollerVisibleDuration = m_totalDuration;
         m_scrollerFirstVisibleTime = 0;
    }

    if (m_maxVisibleDuration > m_totalDuration) {
         m_maxVisibleDuration = m_totalDuration;
    }
    m_maxVisibleDuration = wxMax(m_minVisibleDuration, m_maxVisibleDuration);


    int desiredVisibleDuration = m_visibleDuration;
    desiredVisibleDuration = wxMin(desiredVisibleDuration, m_totalDuration);
    desiredVisibleDuration = wxMin(desiredVisibleDuration, m_maxVisibleDuration);
    desiredVisibleDuration = wxMax(desiredVisibleDuration, m_minVisibleDuration);

    m_visibleDuration = desiredVisibleDuration;

    if (m_firstVisibleTime + m_visibleDuration > m_totalDuration)
    {
        m_firstVisibleTime = m_totalDuration - m_visibleDuration;
    }
    if (m_firstVisibleTime < 0)
    {
        m_firstVisibleTime = 0;
    }
    AdjustMainViewToScrollerView();

    CalcMaxVisibleDuration();
    if (m_visibleDuration > m_maxVisibleDuration) {
        m_visibleDuration = m_maxVisibleDuration;
        if (m_firstVisibleTime + m_visibleDuration > m_totalDuration) {
            m_firstVisibleTime = m_totalDuration - m_visibleDuration;
        }
        if (m_firstVisibleTime < 0) m_firstVisibleTime = 0;
        AdjustMainViewToScrollerView();
    }

    CalcArrowsState();
    RecalcVisibleFrame();
    RecalcItems();
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::AdjustMainViewToScrollerView()
{
    m_visibleDuration = wxMax(m_minVisibleDuration, m_visibleDuration);
    m_visibleDuration = wxMin(m_visibleDuration, m_maxVisibleDuration);
    m_visibleDuration = wxMin(m_visibleDuration, m_totalDuration);

    m_visibleDuration = wxMin(m_visibleDuration, m_scrollerVisibleDuration);

    m_firstVisibleTime = wxMax(m_firstVisibleTime, m_scrollerFirstVisibleTime);
    if (m_firstVisibleTime + m_visibleDuration > m_scrollerFirstVisibleTime + m_scrollerVisibleDuration) {
        m_firstVisibleTime = (m_scrollerFirstVisibleTime + m_scrollerVisibleDuration) - m_visibleDuration;
    }

    m_firstVisibleTime = wxMax(0, m_firstVisibleTime);
    if (m_firstVisibleTime + m_visibleDuration > m_totalDuration) {
        m_firstVisibleTime = m_totalDuration - m_visibleDuration;
        if (m_firstVisibleTime < 0) {
            m_firstVisibleTime = 0;
            m_visibleDuration = wxMin(m_visibleDuration, m_totalDuration);
            m_visibleDuration = wxMax(m_visibleDuration, m_minVisibleDuration);
        }
    }

    if (m_visibleDuration > m_totalDuration) {
        m_visibleDuration = m_totalDuration;
        m_visibleDuration = wxMax(m_visibleDuration, m_minVisibleDuration);
    }
    if (m_firstVisibleTime + m_visibleDuration > m_totalDuration) {
        m_firstVisibleTime = m_totalDuration - m_visibleDuration;
        if (m_firstVisibleTime < 0) m_firstVisibleTime = 0;
    }
    if (m_firstVisibleTime < 0) m_firstVisibleTime = 0;
}

template<typename T>
void wxTimelineCtrl<T>::AddItem(T* data, const wxColour& colour)
{
    if (!data)
    {
        wxLogError("wxTimelineCtrl::AddItem - Attempted to add nullptr data.");
        return;
    }

    wxColour itemColor = colour.IsOk() ? colour : GetItemColour(m_colorCounter++);

    m_items.push_back(TimelineItem<T>(data, TimelineElementState::Normal, itemColor));

    UpdateTotalDurationForItems();

    CalcMaxVisibleDuration();
    RecalcItems();
    Refresh();
}

template<typename T>
int wxTimelineCtrl<T>::ClampFirstVisibleTime(int first) const
{
    if (first < 0) first = 0;
    int maxFirst = m_totalDuration - m_visibleDuration;
    if (maxFirst < 0) maxFirst = 0;
    if (first > maxFirst) return maxFirst;
    return first;
}

template<typename T>
void wxTimelineCtrl<T>::RemoveSelectedItems()
{
    if (m_selectedItems.empty()) return;
    wxVector<size_t> selectedItemsCopy = m_selectedItems;
    for (auto it = selectedItemsCopy.begin(); it != selectedItemsCopy.end();)
    {
        if (*it >= m_items.size()) it = selectedItemsCopy.erase(it);
        else ++it;
    }
    if (selectedItemsCopy.empty()) { m_selectedItems.clear(); return; }
    std::sort(selectedItemsCopy.rbegin(), selectedItemsCopy.rend());
    wxVector<T*> itemsToDelete;
    for (size_t index : selectedItemsCopy)
    {
        if (index < m_items.size() && m_items[index].Data) itemsToDelete.push_back(m_items[index].Data);
    }
    for (size_t i = 0; i < itemsToDelete.size(); ++i)
    {
        wxCommandEvent event(wxEVT_TIMELINE_ITEM_DELETED, GetId());
        event.SetEventObject(this);
        event.SetInt(static_cast<int>(i));
        event.SetClientData(itemsToDelete[i]);
        ProcessWindowEvent(event);
    }
    for (size_t index : selectedItemsCopy)
    {
        if (index < m_items.size()) m_items.erase(m_items.begin() + index);
    }
    m_activeTask = m_items.end();
    m_lastTask = m_items.end();
    m_visibleItemBegin = m_items.end();
    m_visibleItemEnd = m_items.end();
    m_selectedItems.clear();
    m_contextMenuItemIndex = -1;
    UpdateTotalDurationForItems();
    if (m_firstVisibleTime + m_visibleDuration > m_totalDuration)
        SetFirstVisibleTime(wxMax(0, m_totalDuration - m_visibleDuration));
    RecalcItems();
    Refresh();
}

template<typename T>
void wxTimelineCtrl<T>::RemoveContextScrollerItem()
{
    if (m_contextMenuItemIndex < 0 || m_contextMenuItemIndex >= (int)m_items.size())
    {
        m_contextMenuItemIndex = -1;
        return;
    }

    T* itemData = m_items[m_contextMenuItemIndex].Data;

    wxCommandEvent event(wxEVT_TIMELINE_ITEM_DELETED, GetId());
    event.SetEventObject(this);
    event.SetInt(m_contextMenuItemIndex); 
    event.SetClientData(itemData);      
    ProcessWindowEvent(event);

    m_items.erase(m_items.begin() + m_contextMenuItemIndex);

    m_contextMenuItemIndex = -1; 

    if (m_activeTask != m_items.end() && std::distance(m_items.begin(), m_activeTask) >= (int)m_items.size()) {
        m_activeTask = m_items.end(); 
    }
    if (m_lastTask != m_items.end() && std::distance(m_items.begin(), m_lastTask) >= (int)m_items.size()) {
        m_lastTask = m_items.end();
    }

    ClearSelection(); 

    UpdateTotalDurationForItems();
    if (m_firstVisibleTime + m_visibleDuration > m_totalDuration)
    {
        SetFirstVisibleTime(wxMax(0, m_totalDuration - m_visibleDuration));
    }
    else
    {
        RecalcItems();
    }
    Refresh();
}
