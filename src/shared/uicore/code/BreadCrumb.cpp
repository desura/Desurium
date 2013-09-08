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
#include "BreadCrumb.h"
#include "Managers.h"


#include "StripMenuButton.h"
#include "MenuSeperator.h"
#include "StripMenuButton.h"
#include "MenuFiller.h"



BreadCrump::BreadCrump(wxPanel *parent, int id) : gcPanel(parent, id, wxDefaultPosition, wxSize(-1, 19), wxNO_BORDER)
{
	//("#breadcrumb_start")	

	m_pFiller = new MenuFiller(this, "#breadcrumb_bg", wxSize(-1,19));
	m_pStart = new MenuFiller(this, "#breadcrumb_start", wxSize(5, 19));

	m_sizerMenu = new wxFlexGridSizer( 1, 50, 0, 0 );
	m_sizerMenu->AddGrowableRow( 0 );
	m_sizerMenu->SetFlexibleDirection( wxBOTH );
	m_sizerMenu->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	this->SetSizer( m_sizerMenu );
	this->Layout();

	clear();
}

BreadCrump::~BreadCrump()
{
}

int BreadCrump::addItem(const char* text, uint32 id)
{
	gcWString name(text);

	StripMenuButton *temp = new StripMenuButton(this, name.c_str(), wxSize(-1, 19));
	temp->init("#breadcrumb_bg", NULL, 10);

	temp->setHoverCol( wxColor(GetGCThemeManager()->getColor("hyperlinkbreadcrumb", "hov-fg")));
	temp->setNormalCol( wxColor(GetGCThemeManager()->getColor("hyperlinkbreadcrumb", "fg")));
	temp->setOffset(5);

	if (m_vButtons.size() != 0)
		m_vButtons.push_back(new MenuSeperator(this, "#breadcrumb_separator", wxSize(7, 19)));

	m_vButtons.push_back(temp);
	
	m_sizerMenu->Clear();
	m_sizerMenu->Add(m_pStart, 0, 0, 3);
	m_pStart->Show(true);

	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		m_sizerMenu->Add( m_vButtons[x]->getButton(), 1, wxEXPAND, 3 );

		if (m_sizerMenu->IsColGrowable(x+1))
			m_sizerMenu->RemoveGrowableCol(x+1);
	}

	m_sizerMenu->Add(m_pFiller, 1, wxEXPAND, 3);


	m_sizerMenu->AddGrowableCol( m_vButtons.size() + 1);
	this->Layout();

	return temp->GetId();
}

void BreadCrump::clear()
{
	if (m_sizerMenu->IsColGrowable(m_vButtons.size() + 1))
		m_sizerMenu->RemoveGrowableCol(m_vButtons.size() + 1);

	m_sizerMenu->Clear();
	
	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		m_vButtons[x]->getButton()->Destroy();
		m_vButtons[x] = NULL;
	}

	m_vButtons.clear();

	m_pStart->Show(false);

	m_sizerMenu->Add(m_pStart, 0, wxLEFT, 1);
	m_sizerMenu->Add(m_pFiller, 1, wxEXPAND,3);
	m_sizerMenu->AddGrowableCol( 1 );
	this->Layout();
}

