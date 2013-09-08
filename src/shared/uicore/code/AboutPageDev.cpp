/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>

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
#include "AboutPageDev.h"

typedef struct
{
	const char* title;
	const char* name;
	const char* tool;
} AboutInfo_s;

AboutInfo_s g_AboutInfo[] = 
{
	{"Application Lead",		"Mark Chandler",	"Angry Drawf"},
	{"Website Lead",			"Scott Reismanis",	"Slave Driver"},
	{"Interface Design",		"Joshua Collie",	""},
	#ifdef DESURA_OFFICAL_BUILD
	{"Logo && Typography",		"Tim Wilden",		""},
	#else
	{"Logo && Typography",		"notshi",		""},
	#endif
	{"Network Administrator",	"Greg Macsok",		"Destroyer of Sleep"},
	{"Windows Game Guru",		"Dave Traeger",		""},
	{"Linux Game Guru",			"Tim Jung",			""},
};

AboutDevPage::AboutDevPage(wxWindow* parent) : gcPanel(parent, wxID_ANY)
{

	wxColor col = wxColor(GetGCThemeManager()->getColor("abouttext", "bg"));
	wxColor colInfo = wxColor(GetGCThemeManager()->getColor("abouttext", "fg"));

	m_labInfo = new wxStaticText( this, wxID_ANY, wxT("Thanks to all the indies and mods for distracting us. Without you Desura would have been released years ago."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_labInfo1 = new wxStaticText( this, wxID_ANY, wxT("And countless others who have provided feedback and lived to tell the tale. Not tested on animals, only gamers."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	
	m_labInfo->Wrap( 285 );
	m_labInfo1->Wrap( 285 );
	
	m_labInfo->SetForegroundColour( colInfo );
	m_labInfo1->SetForegroundColour( colInfo );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 7, 3, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	for (size_t x=0; x<7; x++)
	{
		wxStaticText* title = new wxStaticText( this, wxID_ANY, g_AboutInfo[x].title);
		wxStaticText* c = new wxStaticText( this, wxID_ANY, wxT(":"));
		wxStaticText* name = new wxStaticText( this, wxID_ANY, g_AboutInfo[x].name);

		name->SetToolTip(g_AboutInfo[x].tool);

		title->SetForegroundColour( col );
		c->SetForegroundColour( col );
		name->SetForegroundColour( col );

		fgSizer4->Add( title, 0, wxLEFT|wxALIGN_RIGHT, 5 );
		fgSizer4->Add( c, 0, wxRIGHT|wxLEFT, 5 );
		fgSizer4->Add( name, 0, wxRIGHT, 5 );
	}

	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer8->AddGrowableCol( 0 );
	fgSizer8->AddGrowableRow( 1 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer8->Add( m_labInfo, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	fgSizer8->Add( fgSizer4, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	fgSizer8->Add( m_labInfo1, 0, wxALL, 5 );
	
	this->SetSizer( fgSizer8 );
	this->Layout();
}

AboutDevPage::~AboutDevPage()
{
}
