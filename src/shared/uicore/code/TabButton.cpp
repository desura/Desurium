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
#include "TabButton.h"
#include "TabControl.h"
#include "Managers.h"

#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE( tabButton, wxPanel )
	EVT_ENTER_WINDOW( tabButton::onMouseOver )
	EVT_LEAVE_WINDOW( tabButton::onMouseOut )
END_EVENT_TABLE()

#define SELECTED	wxColor(GetGCThemeManager()->getColor("tabbutton", "selected-bg"))
#define NORMAL		wxColor(GetGCThemeManager()->getColor("tabbutton", "bg"))
#define MOUSEOVER	wxColor(GetGCThemeManager()->getColor("tabbutton", "selected-bg")) 

tabButton::tabButton( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetBackgroundColour( NORMAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer4->Add( 0, 30, 1, wxEXPAND, 5 );

	m_labTitle = new wxStaticText( this, wxID_ANY, wxT("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labTitle->Wrap( -1 );

	m_labTitle->SetForegroundColour( wxColour( 255, 255, 255 ) );
	
	bSizer4->Add( m_labTitle, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	

	Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tabButton::onMouseDown ) );
	Connect( wxEVT_MOTION, wxMouseEventHandler( tabButton::onMouseEvent ));

	m_labTitle->Connect( wxEVT_MOTION, wxMouseEventHandler( tabButton::onMouseEvent ), NULL, this );
	m_labTitle->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tabButton::onMouseOver ), NULL, this );
	m_labTitle->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tabButton::onMouseOut ), NULL, this );
	m_labTitle->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tabButton::onMouseDown ), NULL, this );
	
	parent->Connect( wxEVT_MOTION, wxMouseEventHandler( tabButton::onMouseEvent ), NULL, this);

	this->SetSizer( bSizer4 );
	this->Layout();

	m_iId = 0;
	m_bSelected = false;
}

tabButton::~tabButton()
{
}

void tabButton::setText(const char* text)
{
	gcWString txt(text);
	m_labTitle->SetLabel(txt.c_str());
}

void tabButton::setSelected(bool state)
{
	if (m_bSelected == state)
		return;

	m_bSelected = state;

	if (m_bSelected)
	{
		this->SetBackgroundColour( SELECTED );
	}
	else
	{
		this->SetBackgroundColour( NORMAL );
	}

	this->Refresh();
}




void tabButton::updateMouse(wxMouseEvent& event)
{
	if (m_bSelected)
		return;

	wxRect panelRec = GetScreenRect(); 
	panelRec.x += 2;
	panelRec.y += 2;
	panelRec.width -= 4;
	panelRec.height -= 4;

	wxPoint mousePoint = wxGetMousePosition();

	bool t1 = panelRec.x <= mousePoint.x;
	bool t2 = panelRec.y <= mousePoint.y;
	bool t3 = (panelRec.x + panelRec.width ) >= mousePoint.x;
	bool t4 = (panelRec.y + panelRec.height) >= mousePoint.y;

	if (t1 && t2 && t3 && t4)
	{
		this->SetBackgroundColour( MOUSEOVER );
	}
	else
	{
		this->SetBackgroundColour( NORMAL );
	}
		
	this->Refresh();
}

void tabButton::onMouseOver( wxMouseEvent& event )
{
	updateMouse(event);
	event.Skip();
}

void tabButton::onMouseOut( wxMouseEvent& event )
{
	updateMouse(event);
	event.Skip();
}

void tabButton::onMouseDown( wxMouseEvent& event )
{
	if (m_bSelected)
		return;

	TabControl* temp = dynamic_cast<TabControl*>(GetParent());

	if (temp)
	{
		temp->setActivePage_ID(this->GetId());
	}

	event.Skip();
}

void tabButton::onMouseEvent( wxMouseEvent& event )
{
	updateMouse(event);
	event.Skip();
}
