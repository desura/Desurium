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
#include "DesuraControl.h"

#include "MainApp.h"

#include "BaseTabPage.h"
#include "ButtonStrip.h"
#include "MainMenuButton.h"
#include "MenuStrip.h"
#include "UsernameBox.h"

#include "MenuStrip.h"
#include "ButtonStrip.h"
#include "MenuFiller.h"

#include "MainFormCustomFrame.h"
#include "MainFormLeftBorder.h"

#include "BaseToolBarControl.h"
#include "FrameButtons.h"

#include "wx_controls/wxEventDelegate.h"
#include "managers/CVar.h"

extern CVar gc_lastavatar;

class DesuraMenuFiller : public wxEventDelegateWrapper<MenuFiller>
{
public:
	DesuraMenuFiller(wxWindow *parent) : wxEventDelegateWrapper<MenuFiller>(parent, "#menu_bg", wxSize(25,38))
	{
		
	}

	void onActiveToggle(bool state)
	{
		m_imgHandle = GetGCThemeManager()->getImageHandle(state?"#menu_bg":"#menu_bg_nonactive");
		invalidatePaint();
	}
};

BEGIN_EVENT_TABLE( DesuraControl, gcPanel )
	EVT_MENU( wxID_ANY, DesuraControl::onMenuSelect )
	EVT_BUTTON( wxID_ANY, DesuraControl::onButtonClicked )
	EVT_SIZE( DesuraControl::onResize )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////

DesuraControl::DesuraControl(gcFrame* parent, bool offline) : gcPanel(parent)
{
	m_pMainCustomFrame = NULL;
	m_iIndex = -1;
	m_bDownloadingUpdate = false;
	m_uiUpdateProgress = 0;

	this->SetBackgroundColour( wxColor(GetGCThemeManager()->getColor("headerbottomborder", "bg")));
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	*GetUserCore()->getNewAvatarEvent() += guiDelegate(this, &DesuraControl::onNewAvatar);

	parent->onActiveEvent += guiDelegate(this, &DesuraControl::onActiveToggle);


	m_pMainMenuButton = new MainMenuButton(this, offline);

	m_pAvatar = new wxEventDelegateWrapper<gcImageButton>( this, wxID_ANY, wxDefaultPosition, wxSize( 66,66 ), wxTAB_TRAVERSAL );

#ifdef WIN32
	m_pAvatar->SetCursor(wxCURSOR_HAND);
#endif

	const char* szAvatar = GetUserCore()->getAvatar();
	if (szAvatar && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(szAvatar)))
	{
		gc_lastavatar.setValue(szAvatar);
		m_pAvatar->setDefaultImage(szAvatar);
	}
	else
	{
		m_pAvatar->setDefaultImage("#icon_avatar");
	}

	m_pFiller = new DesuraMenuFiller(this);
	m_pFiller->SetMinSize( wxSize( 25,38 ) );
	
	m_pUsernameBox = new UsernameBox(this, offline?"Offline":GetUserCore()->getUserName());
	m_pMenuStrip = new MenuStrip(this);
#ifdef WIN32
	m_pFrameButtons = new FrameButtons(this, parent);
	m_pLeftBorder = new MainFormLeftBorder(this);
#endif

#ifdef WIN32
	m_sizerContent = new wxFlexGridSizer( 1, 2, 0, 0 );
	m_sizerContent->AddGrowableCol( 1 );
#else
	m_sizerContent = new wxFlexGridSizer( 1, 1, 0, 0 );
	m_sizerContent->AddGrowableCol( 0 );
#endif
	m_sizerContent->AddGrowableRow( 0 );
	m_sizerContent->SetFlexibleDirection( wxBOTH );
	m_sizerContent->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxFlexGridSizer* fgSizer3;
#ifdef WIN32
	fgSizer3 = new wxFlexGridSizer( 1, 5, 0, 0 );
#else
	fgSizer3 = new wxFlexGridSizer( 1, 4, 0, 0 );
#endif

	fgSizer3->AddGrowableCol( 2 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer3->Add( m_pUsernameBox, 1, wxEXPAND, 5 );
	fgSizer3->Add( m_pMenuStrip, 1, wxEXPAND, 5 ); 	// main buttons across the top
	fgSizer3->Add( m_pFiller, 1, wxEXPAND, 5 );

#ifdef WIN32
	fgSizer3->Add( m_pFrameButtons, 1, wxEXPAND, 5 );
#endif

	
	m_sizerHeader = new wxBoxSizer( wxVERTICAL ); 
	



	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	if (!offline)
	{
		m_bButtonStrip = new ButtonStrip( this );
		fgSizer4->Add( m_bButtonStrip, 1, wxEXPAND|wxBOTTOM, 1 ); // mail buttons
	}
	else
	{
		m_bButtonStrip = NULL;
		fgSizer4->Add( 5, 0, 1, wxEXPAND, 5 );
	}

	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->Add( m_sizerHeader, 1, wxEXPAND, 5 ); // arrows, home button,


	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( fgSizer3, 1, wxEXPAND, 5 );
	fgSizer2->Add( fgSizer4, 1, wxEXPAND, 5 );


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer1->AddGrowableCol( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_pMainMenuButton, 1, 0, 5 );
	fgSizer1->Add( m_pAvatar, 1, wxSHAPED, 5 );
	fgSizer1->Add( fgSizer2, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer5->AddGrowableCol( 0 );
	fgSizer5->AddGrowableRow( 1 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer5->Add( fgSizer1, 1, wxEXPAND, 5 );
	fgSizer5->Add( m_sizerContent, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer5 );
	this->Layout();

	*GetUserCore()->getAppUpdateProgEvent() += guiDelegate(this, &DesuraControl::onDesuraUpdate);

	m_bOffline = offline;
}

DesuraControl::~DesuraControl()
{
	for (size_t x=0; x<m_vTabInfo.size(); x++)
	{
		if (m_vTabInfo[x]->page)
			m_vTabInfo[x]->page->Destroy();

		if (m_vTabInfo[x]->header)
			m_vTabInfo[x]->header->Destroy();

		safe_delete(m_vTabInfo[x]);
	}

	if (GetUserCore())
	{
		*GetUserCore()->getNewAvatarEvent() -= guiDelegate(this, &DesuraControl::onNewAvatar);
		*GetUserCore()->getAppUpdateProgEvent() -= guiDelegate(this, &DesuraControl::onDesuraUpdate);
	}
}

void DesuraControl::refreshSearch()
{
	wxSizeEvent sEvent;
	onResize(sEvent);
}

void DesuraControl::regCustomFrame(gcMainCustomFrameImpl* mcf)
{
	m_pAvatar->regCustomFrame(mcf);
	m_pUsernameBox->regCustomFrame(mcf);
	m_pMenuStrip->regCustomFrame(mcf);
#ifdef WIN32
	m_pLeftBorder->regCustomFrame(mcf);
	m_pFrameButtons->regCustomFrame(mcf);
#endif
	m_pFiller->regCustomFrame(mcf);
	m_pMainCustomFrame = mcf;
}

void DesuraControl::onActiveToggle(bool &state)
{
	this->Freeze();

	m_pMainMenuButton->onActiveToggle(state);
#ifdef WIN32
	m_pLeftBorder->onActiveToggle(state);
#endif
	m_pFiller->onActiveToggle(state);

	m_pUsernameBox->onActiveToggle(state);
	m_pMenuStrip->onActiveToggle(state);
#ifdef WIN32
	m_pFrameButtons->onActiveToggle(state);
	m_pLeftBorder->onActiveToggle(state);
#endif

	if (m_iIndex != UNKNOWN_ITEM && m_iIndex < m_vTabInfo.size())
	{	
		if (m_vTabInfo[m_iIndex]->header)
			m_vTabInfo[m_iIndex]->header->onActiveToggle(state);
	}

	this->Thaw();
	Refresh(false);
}

void DesuraControl::addPage(baseTabPage *page, const char* tabName)
{
	if (!page && !tabName)
		return;

	tabInfo_s *temp = new tabInfo_s;
	
	temp->page = page;
	temp->header = page->getToolBarControl();
	temp->id = m_pMenuStrip->addButton(tabName);

	m_vTabInfo.push_back(temp);

	temp->page->Show(false);
	temp->header->Show(false);

	this->Layout();
	page->progressUpdateEvent += delegate(this, &DesuraControl::onProgressUpdate);
}


void DesuraControl::setActivePage_ID(int32 id)
{
	if (id == 0)
		return;

	for (size_t x=0; x<m_vTabInfo.size(); x++)
	{
		if (m_vTabInfo[x] && m_vTabInfo[x]->id == id)
		{
			setActivePage((PAGE)x);
			break;
		}
	}
}

void DesuraControl::setActivePage(PAGE index, bool reset)
{
	if (index == UNKNOWN_ITEM || m_iIndex == index)
		return;

	if (m_vTabInfo.size() == 0 || (size_t)index > m_vTabInfo.size()-1)
		return;

	if (m_iIndex != UINT_MAX && m_vTabInfo[m_iIndex]->header)
		m_vTabInfo[m_iIndex]->header->Show(false);

	this->Freeze();

	m_pMenuStrip->setSelected(index);
	m_sizerContent->Clear();
	m_sizerHeader->Clear();

	if (m_iIndex != UINT_MAX)
	{
		unloadSearch(m_vTabInfo[m_iIndex]->page);
		m_vTabInfo[m_iIndex]->page->setSelected(false);
		m_vTabInfo[m_iIndex]->page->Show(false);
	}

	loadSearch(m_vTabInfo[index]->page);
	m_vTabInfo[index]->page->setSelected(true);
	m_vTabInfo[index]->page->SetFocus();
	m_vTabInfo[index]->page->Show(true);
	m_vTabInfo[index]->header->Show(true);

	if (reset)
		m_vTabInfo[index]->page->reset();
	
#ifdef WIN32
	m_sizerContent->Add(m_pLeftBorder, 1, wxEXPAND, 0);
#endif
	m_sizerContent->Add( m_vTabInfo[index]->page, 1, wxEXPAND|wxBOTTOM, 1 );
	m_sizerHeader->Add( m_vTabInfo[index]->header, 1, wxEXPAND|wxBOTTOM, 1 );

	this->Thaw();
	this->Layout();

	m_iIndex = index;
}	

void DesuraControl::showLeftBorder(bool state)
{
#ifdef WIN32
	m_pLeftBorder->Show(state);
	m_pFrameButtons->changeMaxButton(!state);
#endif
	Layout();
}

void DesuraControl::updateStatusBar(uint32 index)
{
}

void DesuraControl::unloadSearch(baseTabPage* page)
{
}

void DesuraControl::loadSearch(baseTabPage* page)
{
}

void DesuraControl::onButtonClicked( wxCommandEvent& event )
{
	if (m_bOffline)
		return;

	if (event.GetId() == m_pAvatar->GetId())
	{
		g_pMainApp->loadUrl(GetUserCore()->getProfileEditUrl(), COMMUNITY);
	}
	else if (event.GetId() == m_pUsernameBox->GetId())
	{	
		g_pMainApp->loadUrl(GetUserCore()->getProfileUrl(), COMMUNITY);
	}
	else
	{
		setActivePage_ID(event.GetId());
	}
}

void DesuraControl::onMenuSelect( wxCommandEvent& event )
{
	if (this->GetParent())
		GetParent()->GetEventHandler()->AddPendingEvent(event);
}

void DesuraControl::onResize( wxSizeEvent& event )
{
	event.Skip();
}

void DesuraControl::onProgressUpdate(uint32& idInt)
{
}

void DesuraControl::onNewAvatar(gcString& image)
{
	m_pAvatar->setDefaultImage(image.c_str());
	gc_lastavatar.setValue(image.c_str());
}

void DesuraControl::onDesuraUpdate(uint32& prog)
{
}

void DesuraControl::setBaseTabPage(PAGE pageId, baseTabPage *page)
{
	if ((size_t)pageId >= m_vTabInfo.size())
		return;

	bool shouldShowPage = false;
	bool shouldShowHeader = false;

	if (m_vTabInfo[pageId]->page)
	{
		shouldShowPage = m_vTabInfo[pageId]->page->IsShown();
		m_vTabInfo[pageId]->page->Show(false);
		m_vTabInfo[pageId]->page->Destroy();
	}

	if (m_vTabInfo[pageId]->page)
	{
		shouldShowHeader = m_vTabInfo[pageId]->header->IsShown();
		m_vTabInfo[pageId]->header->Show(false);
		m_vTabInfo[pageId]->header->Destroy();
	}

	m_vTabInfo[pageId]->page = page;

	if (page)
		m_vTabInfo[pageId]->header = page->getToolBarControl();
	else
		m_vTabInfo[pageId]->header = NULL;

	if (m_vTabInfo[pageId]->page)
		m_vTabInfo[pageId]->page->Show(shouldShowPage);

	if (m_vTabInfo[pageId]->header)
		m_vTabInfo[pageId]->header->Show(shouldShowHeader);
}

PAGE DesuraControl::getActivePage()
{
	return (PAGE)m_iIndex;
}
