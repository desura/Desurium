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
#include "PreloadPage.h"


namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


PreloadPage::PreloadPage(wxWindow* parent, const char* action, const char* id) : BaseInstallPage(parent)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	gcWString lab;

	if (!action || !id)
		lab = gcWString(Managers::GetString(L"#IF_PREPARE_INFO_NOITEM"));
	else
		lab = gcWString(Managers::GetString(L"#IF_PREPARE_INFO"), action, id);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	gcStaticText *labInfo = new gcStaticText(this, wxID_ANY, lab, wxDefaultPosition, wxDefaultSize, 0 );
	gcSpinningBar *pbProgress = new gcSpinningBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1,22 ));

	fgSizer1->Add( labInfo, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	fgSizer1->Add( pbProgress, 1, wxEXPAND|wxALL, 5 );
	fgSizer1->Add(0, 0, 0, wxEXPAND, 5);
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	this->setParentSize(-1, 120);
}

}
}
}