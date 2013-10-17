/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#include "common.h"
#include "HeaderButton.h"
#include "Managers.h"

#define UNSORT_IMG		("#columnsort")		//".\\data\\images\\unsorted.jpg"
#define SORT_DESC_IMG	("#column_sort_desend")	//".\\data\\images\\sort_desend.jpg"
#define SORT_ASC_IMG	("#column_sort_asend")	//".\\data\\images\\sort_asend.jpg"

headerButton::headerButton( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : gcPanel( parent, id, pos, size, style )
{
	SetMinSize(size);
	SetBackgroundColour( wxColour( 44, 44, 44 ) );

	m_labTitle = new gcStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_labTitle->Wrap( -1 );
	m_labTitle->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );

	m_imgSortDir = new gcImageControl( this, wxID_ANY, wxDefaultPosition, wxSize( 9,9 ), 0 );
	m_imgSortDir->setImage(SORT_ASC_IMG);
	m_imgSortDir->Show(false);


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( 5, 0, 1, 0, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( m_labTitle, 0, wxALIGN_CENTER_VERTICAL, 5 );
	fgSizer2->Add( m_imgSortDir, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	fgSizer1->Add( fgSizer2, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();


	// Connect Events
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ) );
	m_imgSortDir->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ), NULL, this );
	m_labTitle->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ), NULL, this );

	m_bSelected = false;
}

headerButton::~headerButton()
{
	// Disconnect Events
	this->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ) );
	m_imgSortDir->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ), NULL, this );
	m_labTitle->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( headerButton::OnMouseClick ), NULL, this );
}

void headerButton::OnMouseClick( wxMouseEvent& event )
{ 
	int32 id = this->GetId();
	onClickedEvent(id);
	event.Skip(); 
}

void headerButton::setText(const char* text)
{
	gcWString txt(text);
	m_labTitle->SetLabel(txt);
}


void headerButton::setSelected(bool state)
{
	m_bSelected = state;
		
	if (m_bSelected)
	{
		if (m_iSortMode & 0x1)
			m_imgSortDir->setImage(SORT_ASC_IMG);
		else
			m_imgSortDir->setImage(SORT_DESC_IMG);

		m_imgSortDir->Show(true);
		this->Layout();
	}
	else
	{
		m_imgSortDir->Show(false);
		this->Layout();
	}

	Refresh();
}

void headerButton::inverseSortDir()
{
	m_iSortMode = (~m_iSortMode & 0x1) + (m_iSortMode & ~0x1);

	if (m_iSortMode & 0x1)
		m_imgSortDir->setImage(SORT_ASC_IMG);
	else
		m_imgSortDir->setImage(SORT_DESC_IMG);
}

