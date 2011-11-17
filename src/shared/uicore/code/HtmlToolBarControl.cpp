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
#include "HtmlToolBarControl.h"
#include "DispLoading.h"
#include "CrumbInfo.h"
#include "SearchControl.h"

HtmlToolBarControl::HtmlToolBarControl(wxWindow* parent) : BaseToolBarControl(parent)
{
	m_butBack = new gcImageButton( this, BUTTON_BACK, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butBack->setDefaultImage(("#browser_back"));
	m_butBack->setHoverImage(("#browser_back_hover"));
	m_butBack->SetToolTip(Managers::GetString(L"#BC_BACK"));

	m_butFoward = new gcImageButton( this, BUTTON_FOWARD, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butFoward->setDefaultImage(("#browser_forward"));
	m_butFoward->setHoverImage(("#browser_forward_hover"));
	m_butFoward->SetToolTip(Managers::GetString(L"#BC_FORWARD"));

	m_butHome = new gcImageButton( this, BUTTON_HOME, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butHome->setDefaultImage(("#browser_home"));
	m_butHome->setHoverImage(("#browser_home_hover"));
	m_butHome->SetToolTip(Managers::GetString(L"#BC_HOME"));

	m_pBreadCrumb = new BreadCrump( this, wxID_ANY);

	m_dispLoading = new DispLoading(this, Managers::GetString(L"#DL_LOADING"), wxSize(-1,19));
	m_dispLoading->Show(false);

	m_butStop = new gcImageButton( this, BUTTON_STOP, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butStop->setDefaultImage(("#browser_stop"));
	m_butStop->setHoverImage(("#browser_stop_hover"));
	m_butStop->SetToolTip(Managers::GetString(L"#BC_STOP"));
	m_butStop->Show(false);

	m_butRefresh = new gcImageButton( this, BUTTON_REFRESH, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butRefresh->setDefaultImage(("#browser_refresh"));
	m_butRefresh->setHoverImage(("#browser_refresh_hover"));
	m_butRefresh->SetToolTip(Managers::GetString(L"#BC_REFRESH"));

	m_butLaunch = new gcImageButton( this, BUTTON_LAUNCH, wxDefaultPosition, wxSize( 19,19 ), 0 );	
	m_butLaunch->setDefaultImage(("#browser_break"));
	m_butLaunch->setHoverImage(("#browser_break_hover"));
	m_butLaunch->SetToolTip(Managers::GetString(L"#BC_BREAK"));

#ifdef ENABLE_SEARCH
	m_pSearch = new SearchControl(this);
	m_pSearch->onSearchEvent += delegate(&onSearchEvent);
	m_pSearch->onFullSearchEvent += delegate(&onFullSearchEvent);
#else
	m_pSearch = NULL;
#endif

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 1, 11, 0, 0 );
	fgSizer1->AddGrowableCol( 4 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_butBack, 0, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( m_butFoward, 0, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( 3, 0, 1, 0, 4 );
	fgSizer1->Add( m_butHome, 0, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( m_pBreadCrumb, 0, wxTOP|wxBOTTOM|wxEXPAND, 4 );
	fgSizer1->Add( m_dispLoading, 1, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( m_butStop, 1, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( m_butRefresh, 0, wxTOP|wxBOTTOM, 4 );
	
#ifdef ENABLE_SEARCH	
	fgSizer1->Add( m_butLaunch, 0, wxTOP|wxBOTTOM, 4 );
	fgSizer1->Add( m_pSearch, 0, wxTOP|wxBOTTOM, 4);
	fgSizer1->Add( m_pSearch->getButton(), 0, wxTOP|wxBOTTOM|wxRIGHT, 4);
#else
	fgSizer1->Add( m_butLaunch, 0, wxTOP|wxBOTTOM|wxRIGHT, 4 );
#endif

	this->SetSizer( fgSizer1 );
	this->Layout();

	onPageStartLoadingEvent += guiDelegate(this, &HtmlToolBarControl::onPageStartLoading);
	onPageEndLoadingEvent += guiDelegate(this, &HtmlToolBarControl::onPageFinishLoading);

	m_NormColor = this->GetForegroundColour();
	m_NonActiveColor = wxColor(GetGCThemeManager()->getColor("label", "na-fg"));
}

HtmlToolBarControl::~HtmlToolBarControl()
{
	onPageStartLoadingEvent -= guiDelegate(this, &HtmlToolBarControl::onPageStartLoading);
	onPageEndLoadingEvent -= guiDelegate(this, &HtmlToolBarControl::onPageFinishLoading);

	safe_delete(m_vCrumbList);
}

void HtmlToolBarControl::focusSearch()
{
	if (m_pSearch)
		m_pSearch->focusSearch();
}

void HtmlToolBarControl::onActiveToggle(bool state)
{
	m_butBack->setDefaultImage(state?"#browser_back":"#browser_back_nonactive");
	m_butFoward->setDefaultImage(state?"#browser_forward":"#browser_forward_nonactive");
	m_butHome->setDefaultImage(state?"#browser_home":"#browser_home_nonactive");
	m_butStop->setDefaultImage(state?"#browser_stop":"#browser_stop_nonactive");
	m_butRefresh->setDefaultImage(state?"#browser_refresh":"#browser_refresh_nonactive");
	m_butLaunch->setDefaultImage(state?"#browser_break":"#browser_break_nonactive");

	m_dispLoading->SetForegroundColour(state?m_NormColor:m_NonActiveColor);
	m_dispLoading->Refresh(false);

	m_butBack->invalidatePaint();
	m_butFoward->invalidatePaint();
	m_butHome->invalidatePaint();
	m_butStop->invalidatePaint();
	m_butRefresh->invalidatePaint();
	m_butLaunch->invalidatePaint();

	if (m_pSearch)
		m_pSearch->onActiveToggle(state);
}

void HtmlToolBarControl::onPageStartLoading()
{
	showLoading(true);
}

void HtmlToolBarControl::onPageFinishLoading()
{
	showLoading(false);
}

void HtmlToolBarControl::showLoading(bool state)
{
	m_dispLoading->Show(state);
	m_butStop->Show(state);
	m_butRefresh->Show(!state);
	this->Layout();
}


void HtmlToolBarControl::addCrumb(const char* name, const char* url)
{
	gcWString wname = name;
	gcWString wurl = url;

	CrumbInfo* info = new CrumbInfo(wname.c_str(), wurl.c_str(), 0);
	info->crumbId = m_pBreadCrumb->addItem(name, (uint32)m_vCrumbList.size()-1);
	m_vCrumbList.push_back(info);
}

void HtmlToolBarControl::clearCrumbs()
{
	m_pBreadCrumb->clear();
	safe_delete(m_vCrumbList);
	m_vCrumbList.clear();
}

const wchar_t* HtmlToolBarControl::getCrumbUrl(int32 id)
{
	for (size_t x=0; x<m_vCrumbList.size(); x++)
	{
		if (id == m_vCrumbList[x]->crumbId)
		{
			return m_vCrumbList[x]->url.c_str();
			break;
		}
	}

	return NULL;
}