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
#include "UninstallForm.h"

#include "usercore/ItemInfoI.h"
#include "UninstallInfoPage.h"
#include "UninstallProgressPage.h"

BEGIN_EVENT_TABLE( UninstallForm, gcFrame )
	EVT_CLOSE( UninstallForm::onClose )
END_EVENT_TABLE()

UninstallForm::UninstallForm(wxWindow* parent) : gcFrame(parent, wxID_ANY, wxT("Uninstalling item"), wxDefaultPosition, wxSize(370, 140), wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxSYSTEM_MENU)
{
	m_bsSizer = new wxBoxSizer( wxVERTICAL );
	this->SetSizer( m_bsSizer );
	this->Layout();

	m_pPage = NULL;
	m_bCanClose = true;

	centerOnParent();
}

UninstallForm::~UninstallForm()
{
}

void UninstallForm::onClose( wxCloseEvent& event )
{
	if (!m_bCanClose)
	{
		event.Veto();
	}
	else
	{
		Show(false);
		g_pMainApp->closeForm(this->GetId());
	}
}

void UninstallForm::setInfo(DesuraId id)
{
	m_uiInternId = id;
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);
	
	if (!item)
	{
		Warning("Item was null for uninstall!");
		Close();
	}

	gcWString name(item->getName());

	gcWString title(255, L"{0} {0}", GETSTRING(L"#UNF_TITLE"), name.c_str());
	this->SetTitle(title.c_str());

	showInfo();
}

void UninstallForm::showInfo()
{

	cleanUpPages();

	UFInfoPage *pPage = new UFInfoPage( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pPage->setInfo(m_uiInternId);

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}


void UninstallForm::showProg(bool removeAll, bool removeAccount)
{
	m_bCanClose = false;

	cleanUpPages();

	UFProgressPage *pPage = new UFProgressPage( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pPage->setInfo(m_uiInternId, removeAll, removeAccount);

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}

void UninstallForm::run()
{

}

void UninstallForm::cleanUpPages()
{
	if (m_pPage)
	{
		m_pPage->Show(false);
		m_pPage->Close();
		m_pPage->dispose();
		m_pPage = NULL;
	}

	m_bsSizer->Clear(true);
}
