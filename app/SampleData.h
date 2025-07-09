#pragma once

#include <wx/wx.h>
#include "TimelineItemData.h"

class SampleData : public TimelineItemData
{
public:
	SampleData(const wxString& name, int startSeconds, int endSeconds) 
		: TimelineItemData(startSeconds, endSeconds), m_Name(name) {}
	virtual const wxString& GetName() const { return m_Name; }
	virtual void SetName(const wxString& name) { m_Name = name; }
private:
	wxString m_Name;
};
