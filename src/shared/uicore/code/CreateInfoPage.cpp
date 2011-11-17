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
#include "CreateInfoPage.h"

//something with the wxLog fucks the compile of this up.
#define _WX_LOG_H_

#include "CreateInfoPage.h"
#include "MainApp.h"
#include "MainForm.h"

#include "CreateForm.h"

BEGIN_EVENT_TABLE( CreateInfoPage, BasePage )
	EVT_BUTTON( wxID_ANY, CreateInfoPage::onButtonClicked )
	EVT_TEXT( wxID_ANY, CreateInfoPage::onTextChange )
END_EVENT_TABLE()



CreateInfoPage::CreateInfoPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	m_tbItemFiles = NULL;

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	
	wxBoxSizer* bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_labText = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#CF_PROMPT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labText->Wrap( -1 );
	bSizer5->Add( m_labText, 0, wxALIGN_BOTTOM|wxALL, 5 );
	
	

	wxBoxSizer* bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	m_tbItemFiles = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_tbItemFiles, 1, wxBOTTOM|wxLEFT, 5 );
	
	m_butFile = new gcButton(this, wxID_ANY, Managers::GetString(L"#BROWSE"));
	bSizer8->Add( m_butFile, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	



	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_butCreate = new gcButton( this, wxID_ANY, Managers::GetString(L"#OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butCreate, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butCancel, 0, wxALL, 5 );
	

	fgSizer1->Add( bSizer5, 0, wxEXPAND, 5 );
	fgSizer1->Add( bSizer8, 0, wxEXPAND, 5 );
	fgSizer1->Add(0, 0, 0, wxEXPAND, 5);
	fgSizer1->Add( bSizer2, 0, wxEXPAND, 5 );

	
	this->SetSizer( fgSizer1 );
	this->Layout();
}

CreateInfoPage::~CreateInfoPage()
{
	dispose();
}

void CreateInfoPage::dispose()
{

}

void CreateInfoPage::onTextChange( wxCommandEvent& event )
{
	validateInput();
}



void CreateInfoPage::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butCreate->GetId())
	{
		CreateMCFForm* temp = dynamic_cast<CreateMCFForm*>(GetParent());

		gcString path((const wchar_t*)m_tbItemFiles->GetValue().c_str());
		temp->showProg(path.c_str());
	}
	else if (event.GetId() == m_butCancel->GetId())
	{
		GetParent()->Close();
	}	
	else if (event.GetId() == m_butFile->GetId())
	{
		showDialog();
	}
}

void CreateInfoPage::showDialog()
{
	wxString curPath = m_tbItemFiles->GetValue();
    wxDirDialog *p = new wxDirDialog(this, Managers::GetString(L"#CF_DIRPROMPT"), curPath, wxDIRP_DIR_MUST_EXIST );

    if (p->ShowModal() == wxID_OK)
    {
        m_tbItemFiles->SetValue(p->GetPath());
    }

    wxDELETE(p);
}

void CreateInfoPage::validateInput()
{
	if (!m_tbItemFiles)
		return;

	bool state = true;

	if (!validatePath(m_tbItemFiles, TYPE_FOLDER))
		state = false;

	m_butCreate->Enable( state );
};

bool CreateInfoPage::validatePath(wxTextCtrl* ctrl, bool type)
{
	bool doesExsist = false;

	gcString dir((const wchar_t*)ctrl->GetValue().c_str());

	if (type == TYPE_FOLDER)
		doesExsist = UTIL::FS::isValidFolder(UTIL::FS::PathWithFile(dir));
	else
		doesExsist = UTIL::FS::isValidFile(UTIL::FS::PathWithFile(dir));

	if (doesExsist)
		ctrl->SetForegroundColour( *wxBLACK );
	else
		ctrl->SetForegroundColour( *wxRED );


	ctrl->Refresh();
	return doesExsist;
}


void CreateInfoPage::resetAllValues()
{
	//m_dpFiles->SetPath(wxT(""));
	m_butCreate->Enable( false );
}

void CreateInfoPage::setInfo(DesuraId id)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item && !GetUserCore()->isAdmin())
	{	
		GetParent()->Close();
		return;
	}

	BasePage::setInfo(id);
}
