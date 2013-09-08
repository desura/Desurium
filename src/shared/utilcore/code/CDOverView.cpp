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

#include "Common.h"
#include "CDOverView.h"

CDOverView::CDOverView(wxWindow* parent) : gcPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
	setParentSize(370, 230);

	m_butGenerate = new gcButton( this, CD_GENERATE, Managers::GetString(L"#CRASH_BUTGENERATE"), wxDefaultPosition, wxSize( 150,-1 ), 0 );
	m_butUpload = new gcButton( this, CD_UPLOAD, Managers::GetString(L"#CRASH_BUTUPLOAD"), wxDefaultPosition, wxSize( 150,-1 ), 0 );
	m_butCancel = new gcButton( this, CD_CLOSE, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel->SetDefault(); 

	m_labGenerate = new wxStaticText(this, wxID_ANY, Managers::GetString(L"#CRASH_LABGENERATE"));
	m_labUpload = new wxStaticText(this, wxID_ANY, Managers::GetString(L"#CRASH_LABUPLOAD"));
	
	m_labGenerate->Wrap( 175 );
	m_labUpload->Wrap( 175 );


	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer2->Add( m_butGenerate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10 );
	fgSizer2->Add( m_labGenerate, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	fgSizer2->Add( 0, 15, 1, wxEXPAND, 5 );
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer2->Add( m_butUpload, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10 );
	fgSizer2->Add( m_labUpload, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );


	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butCancel, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 0 );
	fgSizer3->AddGrowableRow( 2 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer3->Add( fgSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer3->Add( bSizer2, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer3 );
	this->Layout();
}

CDOverView::~CDOverView()
{
}


