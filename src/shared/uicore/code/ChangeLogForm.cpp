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
#include "ChangeLogForm.h"
#include "MainApp.h"


ChangeLogForm::ChangeLogForm( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 400,300 ), wxDefaultSize );
	
	wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_ieBrowser = new gcMiscWebControl( this, "about:blank", "ChangeLogForm");
	m_butClose = new gcButton( this, wxID_ANY, Managers::GetString(L"#CLOSE"), wxDefaultPosition, wxDefaultSize, 0 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butClose, 0, wxALL, 5 );

	bSizer1->Add( m_ieBrowser, 1, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();

	m_uiItemId = 0;
	m_bApp = false;

	centerOnParent();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ChangeLogForm::onButClick, this);
	Bind(wxEVT_CLOSE_WINDOW, &ChangeLogForm::onFormClose, this);
}

ChangeLogForm::~ChangeLogForm()
{
}


void ChangeLogForm::onFormClose( wxCloseEvent& event )
{
	g_pMainApp->closeForm(this->GetId());
}


void ChangeLogForm::onButClick( wxCommandEvent& event )
{
	Close();
}

void ChangeLogForm::setInfo(UserCore::Item::ItemInfoI* item)
{
	if (!item)
		return;

	if (item->getIcon() && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(item->getIcon())))
		setIcon(item->getIcon());

	m_uiItemId = item->getId();

	gcWString title(Managers::GetString(L"#CL_TITLE"), item->getName());
	SetTitle(title);

	gcWString type = gcWString(m_uiItemId.getTypeString());
	gcWString shortName(item->getShortName());

	gcWString url(L"{0}/{1}/{2}/changelog", GetWebCore()->getUrl(WebCore::Root), type, shortName);

	if (item->getCurrentBranch())
		url += gcWString(L"/{0}", item->getCurrentBranch()->getBranchId());

	m_ieBrowser->loadUrl(url);

#ifdef NIX
	Refresh(false);
#endif
}

void ChangeLogForm::setInfo(uint32 version)
{
	m_bApp = true;
	m_uiItemId = 0;

	SetTitle(Managers::GetString(L"#CL_TITLE"));

	gcWString wUrl(L"{0}/{1}", GetWebCore()->getUrl(WebCore::AppChangeLog), version);
	m_ieBrowser->loadUrl(wUrl);

#ifdef NIX
	Refresh(false);
#endif
}
