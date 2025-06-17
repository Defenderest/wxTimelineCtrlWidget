#ifndef _TIMELINEITEM_H
#define _TIMELINEITEM_H

#include <wx/string.h>
#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <type_traits>
#include "TimelineItemData.h"

// Visual states
enum class TimelineElementState
{
    Normal,
    Hover,
    Pressed,
    Disabled
};

// Timeline item template
template<typename T>
class TimelineItem
{
    // Compile-time check: T must inherit from NamedObject
    static_assert(std::is_base_of<TimelineItemData, T>::value, "TimelineItem<T>: T must derive from NamedObject");

public:
    TimelineItem(T* data = nullptr,
        TimelineElementState state = TimelineElementState::Normal,
        const wxColour& colour = *wxWHITE,
        const wxRect& rect = wxRect())
        : Data(data), State(state), Colour(colour), Rect(rect), m_displayLane(0) 
    {
    }
    
    // Set a new color for the item
    void SetColour(const wxColour& colour)
    {
        Colour = colour;
    }

    TimelineItem(const TimelineItem& other)
        : Data(other.Data), State(other.State), Colour(other.Colour), Rect(other.Rect), m_displayLane(other.m_displayLane)
    {
    }

    // Get name safely
    wxString GetItemName() const
    {
        if (Data)
            return Data->GetName();
        return wxEmptyString;
    }

public:
    T* Data;
    TimelineElementState State;
    wxColour Colour;
    wxRect Rect;
    int m_displayLane;
};

#endif // _TIMELINEITEM_H
