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
#include "BrowserTest.h"
#include "Browser.h"

BrowserTest::BrowserTest(wxWindow* parent) : gcFrame(parent, wxID_ANY, "Browser Test", wxDefaultPosition, wxSize( 800,800 ))
{
	InitWebControl();

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	m_tbName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_tbName, 1, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_tbValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_tbValue, 1, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butAdd = new wxButton( this, wxID_ANY, wxT("Add Cookie"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_butAdd, 0, wxALL, 5 );
	
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	
	m_panel1 = new Browser(this);
	fgSizer1->Add(m_panel1, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5);
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	
	// Connect Events
	m_butAdd->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BrowserTest::onClick, this);
}

BrowserTest::~BrowserTest()
{
	ShutdownWebControl();
}

void BrowserTest::onClick(wxCommandEvent& event)
{
	m_panel1->setCookie(m_tbName->GetValue(), m_tbValue->GetValue());
}
