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
#include "NewsForm.h"
#include "MainApp.h"

#include <wx/uri.h>


NewsForm::NewsForm(wxWindow* parent) : gcFrame(parent, wxID_ANY, "[News Form]", wxDefaultPosition, wxSize( 670,600 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL)
{
	Bind(wxEVT_CLOSE_WINDOW, &NewsForm::onFormClose, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &NewsForm::onButClick, this);

	this->SetSizeHints( wxSize( 400,300 ), wxDefaultSize );

	m_szTitle = Managers::GetString(L"#NF_TITLE");

	m_ieBrowser = new NewsBrowser(this, "about:blank", "NewsForm");
	m_butPrev = new gcButton(this, wxID_ANY, Managers::GetString(L"#NF_PREV"));
	m_butNext = new gcButton(this, wxID_ANY, Managers::GetString(L"#NF_NEXT"));
	m_butClose = new gcButton(this, wxID_ANY, Managers::GetString(L"#CLOSE"));

	
	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( m_butClose, 0, wxALL, 5 );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butPrev, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer4->Add( m_butNext, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer2->Add( m_ieBrowser, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	fgSizer2->Add( bSizer4, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer2 );
	this->Layout();

	m_uiSelected = 0;
	centerOnParent();

	m_szLoadingUrl = gcString(GetGCThemeManager()->getWebPage("loading"));
}

NewsForm::~NewsForm()
{
	safe_delete(m_vItemList);
}

void NewsForm::setAsGift()
{
	m_szTitle = Managers::GetString(L"#GF_TITLE");
	SetTitle(m_szTitle.c_str());
}

void NewsForm::loadNewsItems(std::vector<UserCore::Misc::NewsItem*> &itemList)
{
	for (size_t x=0; x<itemList.size(); x++)
	{
		if (!itemList[x])
			continue;

		UserCore::Misc::NewsItem* temp = new UserCore::Misc::NewsItem(itemList[x]);
		m_vItemList.push_back(temp);
	}

	loadSelection();
}

void NewsForm::onFormClose( wxCloseEvent& event )
{
	g_pMainApp->closeForm(this->GetId());
}

void NewsForm::onButClick( wxCommandEvent& event )
{
	if (event.GetId() == m_butPrev->GetId())
	{
		if (m_uiSelected > 0)
			m_uiSelected--;

		loadSelection();
	}	
	else if (event.GetId() == m_butNext->GetId())
	{
		if (m_uiSelected < m_vItemList.size()-1)
			m_uiSelected++;

		loadSelection();
	}
	else if (event.GetId() == m_butClose->GetId())
	{
		Close();
	}
}

void NewsForm::loadSelection()
{
	if (m_uiSelected >=  m_vItemList.size())
		m_uiSelected = 0;

	gcWString url(L"{0}?url={1}", m_szLoadingUrl, UTIL::STRING::urlEncode(m_vItemList[m_uiSelected]->szUrl));
	m_ieBrowser->loadUrl(url.c_str());
	
	Layout();

	SetTitle(gcWString(L"{0} {1}", m_szTitle, m_vItemList[m_uiSelected]->szTitle));

	if (m_vItemList.size() == 1)
	{
		m_butPrev->Enable(false);
		m_butNext->Enable(false);
	}
	else
	{
		if (m_uiSelected == 0)
		{
			m_butPrev->Enable(false);
			m_butNext->Enable(true);
		}
		else if (m_uiSelected == m_vItemList.size()-1)
		{
			m_butPrev->Enable(true);
			m_butNext->Enable(false);
		}
		else
		{
			m_butPrev->Enable(true);
			m_butNext->Enable(true);
		}
	}
}


