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
#include "UDFSettingsPanel.h"

#include "wx_controls/gcManagers.h"

UDFSettingsPanel::UDFSettingsPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize( 495,204 ), wxTAB_TRAVERSAL)
{
	m_labUninstallInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL_INFO"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labUninstallInfo->Wrap( 475 );

	m_labUninstallTitle = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL_OPT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labUninstallTitle->Wrap( -1 );

	m_labULocTitle = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL_LOC"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labULocTitle->Wrap( -1 );

	m_cbRemoveDesura = new wxCheckBox( this, wxID_ANY, Managers::GetString(L"#DUN_REMOVE_DESURA"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRemoveDesura->SetToolTip(Managers::GetString(L"#DUN_REMOVE_DESURA_TT"));
	m_cbRemoveDesura->SetValue(true); 
	m_cbRemoveDesura->Enable(false);

	m_cbRemoveSettings = new wxCheckBox( this, wxID_ANY, Managers::GetString(L"#DUN_REMOVE_SETTINGS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRemoveSettings->SetToolTip(Managers::GetString(L"#DUN_REMOVE_SETTINGS_TT"));
	m_cbRemoveSettings->SetValue(true); 

	m_cbRemoveCache = new wxCheckBox( this, wxID_ANY, Managers::GetString(L"#DUN_REMOVE_CACHE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRemoveCache->SetToolTip(Managers::GetString(L"#DUN_REMOVE_CACHE_TT"));
	m_cbRemoveCache->SetValue(true); 

	m_cbRemoveSimple = new wxCheckBox( this, wxID_ANY, Managers::GetString(L"#DUN_REMOVE_SIMPLE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRemoveSimple->SetToolTip(Managers::GetString(L"#DUN_REMOVE_SIMPLE_TT"));
	m_cbRemoveSimple->SetValue(true); 

	m_cbRemoveComplex = new wxCheckBox( this, wxID_ANY, Managers::GetString(L"#DUN_REMOVE_COMPLEX"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRemoveComplex->SetToolTip(Managers::GetString(L"#DUN_REMOVE_COMPLEX_TT"));
	m_cbRemoveComplex->SetValue(true); 
	m_cbRemoveComplex->Enable(false);


	char path[255] = {0};
	GetModuleFileNameA(NULL, path, 255);

	m_tbUninstallLoc = new wxTextCtrl( this, wxID_ANY, UTIL::FS::PathWithFile(path).getFolderPath(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );


	wxBoxSizer* bSizer4 = new wxBoxSizer( wxVERTICAL );
	bSizer4->Add( m_cbRemoveDesura, 0, wxALL, 5 );
	bSizer4->Add( m_cbRemoveSettings, 0, wxALL, 5 );
	bSizer4->Add( m_cbRemoveCache, 0, wxALL, 5 );

	wxBoxSizer* bSizer5 = new wxBoxSizer( wxVERTICAL );
	bSizer5->Add( m_cbRemoveComplex, 0, wxALL, 5 );
	bSizer5->Add( m_cbRemoveSimple, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer4 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer4->Add( 20, 0, 1, wxEXPAND, 5 );
	fgSizer4->Add( bSizer4, 1, wxEXPAND, 5 );
	fgSizer4->Add( 20, 0, 1, wxEXPAND, 5 );
	fgSizer4->Add( bSizer5, 1, wxEXPAND, 5 );



	wxFlexGridSizer* fgSizer6 = new wxFlexGridSizer( 8, 1, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer6->AddGrowableCol( 0 );
	
	fgSizer6->Add( 0, 10, 1, wxEXPAND, 5 );
	fgSizer6->Add( m_labUninstallInfo, 0, wxALL, 5 );
	fgSizer6->Add( 0, 10, 1, wxEXPAND, 5 );
	fgSizer6->Add( m_labUninstallTitle, 0, wxRIGHT|wxLEFT, 5 );
	
	fgSizer6->Add( fgSizer4, 1, wxEXPAND, 5 );
	fgSizer6->Add( 0, 10, 1, wxEXPAND, 5 );
	fgSizer6->Add( m_labULocTitle, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	fgSizer6->Add( m_tbUninstallLoc, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	

	this->SetSizer( fgSizer6 );
	this->Layout();
}

UDFSettingsPanel::~UDFSettingsPanel()
{
}

uint32 UDFSettingsPanel::getFlags()
{
	uint32 res = 0;

	if (m_cbRemoveSimple->IsChecked())
		res |= REMOVE_SIMPLE;

	if (m_cbRemoveCache->IsChecked())
		res |= REMOVE_CACHE;

	if (m_cbRemoveSettings->IsChecked())
		res |= REMOVE_SETTINGS;

	return res;
}