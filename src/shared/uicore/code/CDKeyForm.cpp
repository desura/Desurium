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
#include "CDKeyForm.h"
#include "MainApp.h"

#include "CDKInfo.h"
#include "CDKProgress.h"

CDKeyForm::CDKeyForm(wxWindow* parent, const char* exe, bool launch) : gcFrame(parent, wxID_ANY, wxT("[Item]: CD Key"), wxDefaultPosition, wxSize(370,100), wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL)
{
	m_bLaunch = launch;
	m_szExe = exe;

	m_pPage = new CDKProgress(this, m_bLaunch);

	m_bsSizer = new wxBoxSizer( wxVERTICAL );
	m_bsSizer->Add(m_pPage, 1, wxEXPAND, 0);

	this->SetSizer( m_bsSizer );
	this->Layout();
	this->Centre( wxBOTH );

	Bind(wxEVT_CLOSE_WINDOW, &CDKeyForm::onFormClose, this);

	centerOnParent();
}

CDKeyForm::~CDKeyForm()
{
}

void CDKeyForm::onFormClose( wxCloseEvent& event )
{
	g_pMainApp->closeForm(this->GetId());
	event.Skip();
}

void CDKeyForm::finish(const char* cdKey)
{
	cleanUpPages();

	CDKInfo *p =  new CDKInfo(this, m_szExe.c_str(), m_bLaunch);
	p->setInfo(m_ItemId, cdKey);

	m_pPage  = p;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 0 );
	Layout();
	Refresh();

	m_pPage->run();
}

void CDKeyForm::setInfo(DesuraId id)
{
	m_ItemId = id;

	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item)
	{	
		Close();
		return;
	}

	this->SetTitle(gcString("{0}: CD Key", item->getName()));

	m_pPage->setInfo(m_ItemId);
	m_pPage->run();
}

void CDKeyForm::cleanUpPages()
{
	m_bsSizer->Clear(false);

	if (m_pPage)
	{
		m_pPage->Show(false);
		m_pPage->Close();
		m_pPage->dispose();
		m_pPage = NULL;
	}
}

DesuraId CDKeyForm::getItemId()
{
	return m_ItemId;
}

