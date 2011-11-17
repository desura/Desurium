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
#include "TabControl.h"
#include "MainForm.h"


///////////////////////////////////////////////////////////////////////////

TabControl::TabControl( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : gcPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	bSizerTabButtons = new wxBoxSizer( wxHORIZONTAL );

	fgSizer3->Add( bSizerTabButtons, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer3 );
	this->Layout();

	m_iIndex = 0;
}

TabControl::~TabControl()
{
}

tabButton* TabControl::getTab(int32 id)
{
	for (size_t x=0; x<m_vTabInfo.size(); x++)
	{
		if (m_vTabInfo[x] && m_vTabInfo[x]->GetId() == id)
		{
			return m_vTabInfo[x];
		}
	}

	return NULL;
}

void TabControl::addPage(const char* tabName, int32 id)
{
	if (!tabName)
		return;

	tabButton *but = new tabButton(this);
	but->SetMinSize(wxSize(100,-1));
	but->setText(tabName);
	but->setPageId(id);

	m_vTabInfo.push_back(but);

	if (m_vTabInfo.size() == 1)
	{
		but->setSelected(true);
		but->SetSize(wxSize(100,30));
		bSizerTabButtons->Add( but, 0, wxTOP|wxEXPAND, 3 );
	}
	else
	{
		but->SetSize(wxSize(100,27));
		bSizerTabButtons->Add( but, 0, wxTOP|wxLEFT|wxBOTTOM, 3 );
	}

	this->Layout();
}

void TabControl::setActivePage_ID(int32 id)
{
	for (size_t x=0; x<m_vTabInfo.size(); x++)
	{
		if (m_vTabInfo[x] && m_vTabInfo[x]->GetId() == id)
		{
			setActivePage((uint32)x);
			break;
		}
	}
}

void TabControl::setActivePage(uint32 index)
{
	if (index > m_vTabInfo.size()-1)
		return;

	int32 id = 0;

	this->Freeze();

	bSizerTabButtons->Clear(false);
	bSizerTabButtons->Add(0,30,0,0,0);

	for (size_t x=0; x<m_vTabInfo.size(); x++)
	{
		if (!m_vTabInfo[x])
			continue;
		
		int style = wxTOP|wxEXPAND;

		if (x != 0)
			style |= wxLEFT;

		if (x == index)
		{
			id = m_vTabInfo[x]->getPageId();
			m_vTabInfo[x]->setSelected(true);
			m_vTabInfo[x]->SetSize(wxSize(100,30));
			bSizerTabButtons->Add( m_vTabInfo[x], 0, style, 3 );
		}
		else
		{
			style |= wxBOTTOM;

			m_vTabInfo[x]->setSelected(false);
			m_vTabInfo[x]->SetSize(wxSize(100,27));
			bSizerTabButtons->Add( m_vTabInfo[x], 0, style, 3 );
		}
	}

	this->Thaw();

	onSelectPageEvent(id);
}	


