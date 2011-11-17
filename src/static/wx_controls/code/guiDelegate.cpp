/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "Common.h"
#include "wx_controls/guiDelegate.h"

#include "boost/shared_ptr.hpp"


wxDEFINE_EVENT(wxEVT_GUIDELEGATE, wxGuiDelegateEvent);
IMPLEMENT_DYNAMIC_CLASS(wxGuiDelegateEvent, wxNotifyEvent)

wxGuiDelegateEvent::wxGuiDelegateEvent() : wxNotifyEvent(wxEVT_GUIDELEGATE, 0)
{
	m_pDelegate = NULL;
}

wxGuiDelegateEvent::wxGuiDelegateEvent(boost::shared_ptr<InvokeI> invoker, int winId) : wxNotifyEvent(wxEVT_GUIDELEGATE, winId)
{
	m_pDelegate = NULL;
	m_spDelegate = invoker;
}

wxGuiDelegateEvent::wxGuiDelegateEvent(InvokeI* invoker, int winId) : wxNotifyEvent(wxEVT_GUIDELEGATE, winId)
{
	m_pDelegate = invoker;
}

wxGuiDelegateEvent::wxGuiDelegateEvent(const wxGuiDelegateEvent& event) : wxNotifyEvent(event)
{
	m_spDelegate = event.m_spDelegate;
	m_pDelegate = event.m_pDelegate;
}

wxGuiDelegateEvent::~wxGuiDelegateEvent()
{
}

wxEvent *wxGuiDelegateEvent::Clone() const
{ 
	return new wxGuiDelegateEvent(*this); 
}

void wxGuiDelegateEvent::invoke()
{
	if (m_spDelegate.get())
		m_spDelegate->invoke();

	if (m_pDelegate)
		m_pDelegate->invoke();
}





