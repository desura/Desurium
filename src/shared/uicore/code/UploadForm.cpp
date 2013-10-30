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
#include "UploadForm.h"
#include "UploadPrompt.h"
#include "MainApp.h"

extern CVar gc_uploadprompt;

BEGIN_EVENT_TABLE( UploadMCFForm, gcFrame )
	EVT_CLOSE( UploadMCFForm::onFormClose )
END_EVENT_TABLE()

UploadMCFForm::UploadMCFForm( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcFrame( parent, id, title, pos, size, style )
{
	m_bsSizer = new wxBoxSizer( wxVERTICAL );
	this->SetSizer( m_bsSizer );
	this->Layout();

	m_pItemInfo = NULL;
	m_uiInternId = 0;
	m_pPage = NULL;
	m_bTrueClose = true;

	centerOnParent();
}

UploadMCFForm::~UploadMCFForm()
{
	if (GetUserCore())
		*GetUserCore()->getItemsAddedEvent() -= guiDelegate(this, &UploadMCFForm::updateInfo);
}

void UploadMCFForm::onFormClose( wxCloseEvent& event )
{
	Show(false);

	if (m_bTrueClose)	
		g_pMainApp->closeForm(this->GetId());
	else
		event.Veto();
}

void UploadMCFForm::updateInfo(uint32& itemId)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(m_uiInternId);
	if (item)
	{
		setTitle(item->getName());

		if (item->getIcon())
			setIcon(item->getIcon());

		if (m_pPage)
			m_pPage->setInfo(m_uiInternId);

		*GetUserCore()->getItemsAddedEvent() -= guiDelegate(this, &UploadMCFForm::updateInfo);
	}
}

void UploadMCFForm::setInfo(DesuraId id)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item)
	{	
		if (!GetUserCore()->isAdmin())
		{
			Close();
			return;
		}
		else
		{
			*GetUserCore()->getItemsAddedEvent() += guiDelegate(this, &UploadMCFForm::updateInfo);
			GetUserCore()->getItemManager()->retrieveItemInfoAsync(id);
		}
	}

	m_uiInternId = id;

	if (!item)
	{
		gcWString title(L"Upload MCF for {0} [Admin override]", id.getItem());
		this->SetTitle(title.c_str());
	}
	else
	{
		setTitle(item->getName());
	}
}

void UploadMCFForm::setTitle(const char* name)
{
	this->SetTitle(gcWString(L"Upload MCF: {0}", name));
}

void UploadMCFForm::setInfo_path(DesuraId id, const char* path)
{
	setInfo(id);
	m_szPath = gcString(path);
}

void UploadMCFForm::setInfo_key(DesuraId id,  const char* key)
{
	setInfo(id);
	m_szKey = gcString(key);
}

void UploadMCFForm::showInfo()
{
	cleanUpPages();

	UploadInfoPage *pPage = new UploadInfoPage( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	if (!m_szPath.empty() && m_szKey.empty())
	{
		pPage->setInfo_path(m_uiInternId, m_szPath.c_str());
	}
	else if (!m_szKey.empty() && m_szPath.empty())
	{
		pPage->setInfo_key(m_uiInternId, m_szKey.c_str());
	}
	else if (m_szKey.empty() && m_szPath.empty())
	{
		pPage->setInfo(m_uiInternId);
	}
	else
	{
		gcMessageBox(this, wxT("Both path and key are set. Only one should be set."), wxT("Upload MCF Error"));
		Close();
		return;
	}

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}

void UploadMCFForm::showProg(uint32 hash, uint32 start)
{
	m_bTrueClose = false;

	cleanUpPages();

	UploadProgPage *pPage = new UploadProgPage(this);
	pPage->setInfo(m_uiInternId, hash, start);

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}

void UploadMCFForm::run()
{
	showInfo();
}

void UploadMCFForm::cleanUpPages()
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