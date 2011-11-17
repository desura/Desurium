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
#include "CreateForm.h"

#include "CreateInfoPage.h"
#include "CreateProgPage.h"
#include "CreateOVPage.h"


BEGIN_EVENT_TABLE( CreateMCFForm, gcFrame )
	EVT_CLOSE( CreateMCFForm::onFormClose )
END_EVENT_TABLE()

CreateMCFForm::CreateMCFForm( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcFrame( parent, id, title, pos, size, style )
{
	m_bsSizer = new wxBoxSizer( wxVERTICAL );
	this->SetSizer( m_bsSizer );
	this->Layout();

	m_pPage = NULL;

	m_bPromptClose = true;
	centerOnParent();
}

CreateMCFForm::~CreateMCFForm()
{
	if (GetUserCore())
	{
		*GetUserCore()->getItemsAddedEvent() -= guiDelegate(this, &CreateMCFForm::updateInfo);
	}
}

void CreateMCFForm::onFormClose( wxCloseEvent& event )
{
	if (m_bPromptClose)
	{
		if (gcMessageBox(this, Managers::GetString(L"#CONFIRM_PROMPT"), Managers::GetString(L"#CONFIRM"), wxICON_QUESTION|wxYES_NO) != wxYES)
		{
			event.Veto();
			return;
		}
	}

	g_pMainApp->closeForm(this->GetId());
}


void CreateMCFForm::updateInfo(uint32& count)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(m_uiInternId);
	if (item)
	{
		setTitle(item->getName());

		if (item->getIcon())
			setIcon(item->getIcon());

		if (m_pPage)
			m_pPage->setInfo(m_uiInternId);

		*GetUserCore()->getItemsAddedEvent() -= guiDelegate(this, &CreateMCFForm::updateInfo);
	}
}

void CreateMCFForm::setInfo(DesuraId id)
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
			*GetUserCore()->getItemsAddedEvent() += guiDelegate(this, &CreateMCFForm::updateInfo);
			GetUserCore()->getItemManager()->retrieveItemInfoAsync(id);
		}
	}

	m_uiInternId = id;

	if (!item)
	{
		SetTitle(gcWString(L"Create MCF for {0} [Admin override]", id.getItem()));
	}
	else
	{
		setTitle(item->getName());
	}
}

void CreateMCFForm::setTitle(const char* name)
{
	SetTitle(gcWString(L"Create MCF: {0}", name));
}

void CreateMCFForm::showInfo()
{
	cleanUpPages();

	CreateInfoPage *pPage = new CreateInfoPage( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pPage->setInfo(m_uiInternId);

	m_bPromptClose = false;

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}

void CreateMCFForm::showProg(const char* path)
{
	cleanUpPages();

	CreateProgPage* pPage = new CreateProgPage( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pPage->setInfo(m_uiInternId, path);

	m_bPromptClose = true;

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );
	Layout();
	m_pPage->run();
}

void CreateMCFForm::showOverView(const char* path)
{
	cleanUpPages();

	CreateMCFOverview* pPage = new CreateMCFOverview( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pPage->setInfo(m_uiInternId, path);
	pPage->onUploadTriggerEvent += delegate(&onUploadTriggerEvent);
	m_bPromptClose = false;

	m_pPage = pPage;
	m_bsSizer->Add( m_pPage, 1, wxEXPAND, 5 );

#ifdef WIN32
	this->SetSize(-1, -1, 370, 150);
#else
	this->SetSize(-1, -1, 415, 150);
#endif

	Layout();
}

void CreateMCFForm::run()
{
	showInfo();
}

void CreateMCFForm::cleanUpPages()
{
	m_bsSizer->Clear(false);

	if (m_pPage)
	{
		CreateMCFOverview* temp = dynamic_cast<CreateMCFOverview*>(m_pPage);
		if (temp)
			temp->onUploadTriggerEvent -= delegate(&onUploadTriggerEvent);

		m_pPage->Show(false);
		m_pPage->Close();
		m_pPage->dispose();
		m_pPage = NULL;
	}
}
