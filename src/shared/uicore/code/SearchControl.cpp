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
#include "SearchControl.h"
#include "managers/CVar.h"


SearchControl::SearchControl(wxWindow* parent) : gcPanel(parent)
{
	m_szDefaultText = Managers::GetString(L"#SB_DEFAULT");

	m_butFullSearch = new gcImageButton(parent, wxID_ANY, wxDefaultPosition, wxSize( 19,19 ), 0 );
	m_butFullSearch->setDefaultImage(("#search_button"));
	m_butFullSearch->setHoverImage(("#search_button_hover"));
	m_butFullSearch->SetToolTip(Managers::GetString(L"#SB_FULL_SEARCH"));
	m_butFullSearch->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SearchControl::onButtonClick, this);


	wxPanel* bg = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 17));
	bg->SetBackgroundColour(wxColor(GetGCThemeManager()->getColor("searchBox", "sb-bg")));

	m_tbSearchBox = new wxTextCtrl( bg, wxID_ANY, m_szDefaultText, wxDefaultPosition, wxSize(150, 15), 0|wxBORDER_NONE|wxWANTS_CHARS|wxTE_PROCESS_ENTER );
	m_tbSearchBox->SetForegroundColour(wxColor(GetGCThemeManager()->getColor("searchBox", "sb-fg")));
	m_tbSearchBox->SetBackgroundColour(wxColor(GetGCThemeManager()->getColor("searchBox", "sb-bg")));

	m_tbSearchBox->Bind(wxEVT_KILL_FOCUS, &SearchControl::onBlur, this);
	m_tbSearchBox->Bind(wxEVT_SET_FOCUS, &SearchControl::onFocus, this);
	m_tbSearchBox->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SearchControl::onSearchChanged, this);
	m_tbSearchBox->Bind(wxEVT_COMMAND_TEXT_ENTER, &SearchControl::onDoSearch, this);
	m_tbSearchBox->Bind(wxEVT_KEY_DOWN, &SearchControl::onKeyDown, this);

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( 0, 2 );
	fgSizer2->Add( m_tbSearchBox, 0, wxLEFT|wxRIGHT, 2 );

	bg->SetSizer( fgSizer2 );
	bg->Layout();



	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add(bg, 0, wxALL, 1);
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	
	Managers::LoadTheme(this, "searchBox");

	m_DefaultCol = wxColor(GetThemeManager().getColor("formlogin", "tb-default-fg"));
	m_NormalCol = m_tbSearchBox->GetForegroundColour();

	m_tbSearchBox->SetForegroundColour(m_DefaultCol);
}

SearchControl::~SearchControl()
{
}

void SearchControl::focusSearch()
{
	m_tbSearchBox->SetFocusFromKbd();
}

void SearchControl::onKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_ESCAPE)
	{
		m_tbSearchBox->SetValue("");
	}

	event.Skip();
}

void SearchControl::onButtonClick(wxCommandEvent &e)
{
	if (m_tbSearchBox->GetValue() == m_szDefaultText)
		return;

	gcString txt((wchar_t*)m_tbSearchBox->GetValue().wchar_str());
	onFullSearchEvent(txt);
}

void SearchControl::onSearchChanged(wxCommandEvent& event)
{
	if (m_tbSearchBox->GetValue() == m_szDefaultText)
		return;

	gcString txt((wchar_t*)m_tbSearchBox->GetValue().wchar_str());
	onSearchEvent(txt);
}

void SearchControl::onDoSearch(wxCommandEvent& event)
{
	if (m_tbSearchBox->GetValue() == m_szDefaultText)
		return;

	gcString txt((wchar_t*)m_tbSearchBox->GetValue().wchar_str());
	onFullSearchEvent(txt);
}

void SearchControl::onBlur(wxFocusEvent& event)
{
	if (m_tbSearchBox->GetValue() == L"")
		m_tbSearchBox->SetValue(m_szDefaultText);

	m_tbSearchBox->SetForegroundColour(m_DefaultCol);

	event.Skip();
}

void SearchControl::onFocus(wxFocusEvent& event)
{
	gcString txt((wchar_t*)m_tbSearchBox->GetValue().wchar_str());

	if (txt == m_szDefaultText)
		m_tbSearchBox->SetValue(L"");
	else
		onSearchEvent(txt);

	m_tbSearchBox->SetForegroundColour(m_NormalCol);
	event.Skip();
}

gcImageButton* SearchControl::getButton()
{
	return m_butFullSearch;
}

void SearchControl::onActiveToggle(bool state)
{
	m_butFullSearch->setDefaultImage(state?"#search_button":"#search_button_nonactive");
}
