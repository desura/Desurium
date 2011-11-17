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
#include "MenuStrip.h"
#include "MainFormCustomFrame.h"
#include "DStripMenuControls.h"


MenuStrip::MenuStrip(wxWindow* parent) : gcPanel(parent)
{
	m_sizerMenu = new wxFlexGridSizer( 1, 50, 0, 0 );
	m_sizerMenu->AddGrowableRow( 0 );
	m_sizerMenu->SetFlexibleDirection( wxBOTH );
	m_sizerMenu->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	this->SetSizer( m_sizerMenu );
	this->Layout();
}

MenuStrip::~MenuStrip()
{
}

void MenuStrip::onActiveToggle(bool state)
{
	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		DStripMenuButton* smb = dynamic_cast<DStripMenuButton*>(m_vButtons[x]->getButton());

		if (smb)
			smb->setActive(state);

		DMenuSeperator* ms = dynamic_cast<DMenuSeperator*>(m_vButtons[x]->getButton());

		if (ms)
			ms->setActive(state);
	}
}

void MenuStrip::regCustomFrame(gcMainCustomFrameImpl* mcf)
{
	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		DStripMenuButton* smb = dynamic_cast<DStripMenuButton*>(m_vButtons[x]->getButton());

		if (smb)
			smb->regCustomFrame(mcf);

		DMenuSeperator* ms = dynamic_cast<DMenuSeperator*>(m_vButtons[x]->getButton());

		if (ms)
			ms->regCustomFrame(mcf);
	}
}

void MenuStrip::setSelected(uint32 count)
{
	uint32 index = count*2 + 1;
	
	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		m_vButtons[x]->setSelected(false);
	}

	m_vButtons[index-1]->setSelected(true, LEFT);
	m_vButtons[index]->setSelected(true);
	m_vButtons[index+1]->setSelected(true, RIGHT);
}

BaseMenuButton* MenuStrip::newSerperator()
{
	MenuSeperator *sep = new DMenuSeperator(this, wxSize(2,38));
	sep->setImages("#menu_separator_selected_right", "#menu_separator_selected_left");
	return sep;
}

int32 MenuStrip::addButton(const char* name)
{
#ifdef WIN32
	DStripMenuButton *temp = new DStripMenuButton(this, name, wxSize(-1, 38));
#else
	DStripMenuButton *temp = new DStripMenuButton(this, name, wxSize(-1, 36));
#endif

#ifdef WIN32
	temp->SetForegroundColour( wxColour(255,255,255) );
#endif
	temp->SetWindowStyle(wxALIGN_CENTER);
	temp->setBold(true);

	if (m_vButtons.size() == 0)
		m_vButtons.push_back(newSerperator());

	m_vButtons.push_back(temp);
	m_vButtons.push_back(newSerperator());

	m_sizerMenu->Clear();
	for (size_t x=0; x<m_vButtons.size(); x++)
	{
		m_sizerMenu->Add( m_vButtons[x]->getButton(), 1, wxEXPAND, 3 );

		if (m_sizerMenu->IsColGrowable(x))
			m_sizerMenu->RemoveGrowableCol(x);
	}

	m_sizerMenu->AddGrowableCol( m_vButtons.size() );
	return temp->GetId();
}
