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
#include "CreateOVPage.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

BEGIN_EVENT_TABLE( CreateMCFOverview, BasePage )
	EVT_BUTTON( wxID_ANY, CreateMCFOverview::onButtonClick )
END_EVENT_TABLE()

CreateMCFOverview::CreateMCFOverview( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer2->AddGrowableRow( 2 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_labInfo = new gcStaticText( this, wxID_ANY, wxT("Creating MCF for [item] has completed."), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_labInfo->Wrap( -1 );
	fgSizer2->Add( m_labInfo, 1, wxALL|wxEXPAND, 5 );
	

	
	fgSizer2->Add( 0, 5, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 3, 3, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	gcStaticText* m_staticText2 = new gcStaticText( this, wxID_ANY,  Managers::GetString(L"Name"), wxDefaultPosition, wxSize( 30,-1 ), wxALIGN_RIGHT );
	m_staticText2->Wrap( -1 );
	fgSizer1->Add( m_staticText2, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_RIGHT, 3 );
	
	gcStaticText* m_staticText3 = new gcStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer1->Add( m_staticText3, 0, wxTOP, 3 );
	
	m_labName = new gcStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	m_labName->Wrap( -1 );
	fgSizer1->Add( m_labName, 0, wxLEFT|wxRIGHT|wxTOP, 3 );
	
	gcStaticText* m_staticText5 = new gcStaticText( this, wxID_ANY, Managers::GetString(L"Size"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticText5->Wrap( -1 );
	fgSizer1->Add( m_staticText5, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_RIGHT, 3 );
	
	gcStaticText* m_staticText6 = new gcStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer1->Add( m_staticText6, 0, wxTOP, 3 );
	
	m_labSize = new gcStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	m_labSize->Wrap( -1 );
	fgSizer1->Add( m_labSize, 0, wxLEFT|wxRIGHT|wxTOP, 3 );
	
	gcStaticText* m_staticText8 = new gcStaticText( this, wxID_ANY, Managers::GetString(L"Path"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticText8->Wrap( -1 );
	fgSizer1->Add( m_staticText8, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_RIGHT, 3 );
	
	gcStaticText* m_staticText9 = new gcStaticText( this, wxID_ANY, wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer1->Add( m_staticText9, 0, wxTOP, 3 );
	
	m_labPath = new gcStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	m_labPath->Wrap( -1 );
	fgSizer1->Add( m_labPath, 0, wxLEFT|wxRIGHT|wxTOP, 3 );
	
	fgSizer2->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	
	wxColor col = wxColor(GetGCThemeManager()->getColor("abouttext", "bg"));

	m_staticText2->SetForegroundColour(col);
	m_staticText3->SetForegroundColour(col);
	m_labName->SetForegroundColour(col);
	m_staticText5->SetForegroundColour(col);
	m_staticText6->SetForegroundColour(col);
	m_labSize->SetForegroundColour(col);
	m_staticText8->SetForegroundColour(col);
	m_staticText9->SetForegroundColour(col);
	m_labPath->SetForegroundColour(col);


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_butViewFile = new gcButton( this, wxID_ANY, Managers::GetString(L"#CF_VIEWFILES"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butViewFile, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butUpload = new gcButton( this, wxID_ANY, Managers::GetString(L"#UPLOAD"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butUpload, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butClose = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butClose, 0, wxALL, 5 );
	
	fgSizer2->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer2 );
	this->Layout();

	m_labPath->SetCursor(wxCURSOR_HAND);
	m_labPath->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CreateMCFOverview::onMouseDown ), NULL, this );
}

CreateMCFOverview::~CreateMCFOverview()
{
	dispose();
}

void CreateMCFOverview::dispose()
{
}

void CreateMCFOverview::onMouseDown( wxMouseEvent& event )
{
#ifdef WIN32
	ShellExecuteA(NULL, "explore" , m_szFolderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
	UTIL::OS::launchFolder(m_szFolderPath.c_str());
#endif
}

void CreateMCFOverview::onButtonClick( wxCommandEvent& event )
{
	if (event.GetId() == m_butUpload->GetId())
	{
		ut utInfo;
		utInfo.path = m_szPath;
		utInfo.id = this->getItemId();
		utInfo.caller = this;

		onUploadTriggerEvent(utInfo);	
		GetParent()->Close();
	}
	else if (event.GetId() == m_butViewFile->GetId())
	{
#ifdef WIN32
		ShellExecuteA(NULL, "explore" , m_szFolderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
	UTIL::OS::launchFolder(m_szFolderPath.c_str());
#endif
	}
	else
	{
		GetParent()->Close();
	}
}

void CreateMCFOverview::setInfo(DesuraId itemId, const char* szPath)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(itemId);
	
	if (!item && !GetUserCore()->isAdmin())
	{	
		GetParent()->Close();
		return;
	}

	BasePage::setInfo(itemId);
	
	if (item)
		m_labInfo->SetLabel(gcWString(Managers::GetString(L"#CF_COMPLETE"), item->getName()));

	UTIL::FS::Path path(szPath, "", true);
	uint64 filesize = UTIL::FS::getFileSize(path);

	m_szFolderPath = path.getFolderPath();
	m_szPath = path.getFullPath();
	m_pItem = item;

	m_labName->SetLabel(path.getFile().getFile());
	m_labSize->SetLabel(UTIL::MISC::niceSizeStr(filesize));
	m_labPath->SetLabel(path.getShortPath(45, true));
	m_labPath->SetToolTip(path.getFullPath());

	Layout();
}
