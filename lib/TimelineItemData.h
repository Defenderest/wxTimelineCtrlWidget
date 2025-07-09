#ifndef _TIMELINEITEMDATA_H
#define _TIMELINEITEMDATA_H

#include <wx/datetime.h>

// A simple reusable structure for Timeline data
class TimelineItemData
{
public:
    TimelineItemData()
        : Start(0)
        , End(0)
    {
    }

    TimelineItemData(int startSeconds, int endSeconds)
        : Start(startSeconds)
        , End(endSeconds)
    {
    }

    TimelineItemData(wxTimeSpan startSpan, wxTimeSpan endSpan)
        : Start(startSpan.GetSeconds().ToLong())
        , End(endSpan.GetSeconds().ToLong())
    {
    }

    int GetStartTime() const { return Start; }
    int GetEndTime() const { return End; }
    int GetDuration() const { return End - Start; }

    void SetStartTime(int start) { Start = start; }
    void SetEndTime(int end) { End = end; }
    void SetDuration(int duration) { End = Start + duration; }

    virtual ~TimelineItemData() = default;

    virtual const wxString& GetName() const = 0;

    // Set the name of the item
    virtual void SetName(const wxString& name) = 0;

private:
    int Start;
    int End;
};

#endif // _TIMELINEITEMDATA_H