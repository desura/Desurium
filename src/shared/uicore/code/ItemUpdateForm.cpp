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
#include "ItemUpdateForm.h"
#include "MainApp.h"

BEGIN_EVENT_TABLE( UpdateInfoForm, gcFrame )
	EVT_BUTTON( wxID_ANY, UpdateInfoForm::onButtonPressed )
	EVT_CLOSE( UpdateInfoForm::onFormClose )
END_EVENT_TABLE()

UpdateInfoForm::UpdateInfoForm(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : gcFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 400,250 ), wxDefaultSize );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 1 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#UI_LABEL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap( -1 );
	fgSizer2->Add( m_labInfo, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	
	m_ieBrowser = new gcMiscWebControl( this, "about:blank", "UpdateInfoForm");
	fgSizer2->Add( m_ieBrowser, 1, wxALL|wxEXPAND, 5 );
	

	m_cbReminder = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#UI_NO_REMINDER"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_cbReminder, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );


	m_butUpdate = new gcButton( this, wxID_ANY, Managers::GetString(L"#UPDATE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butLaunch = new gcButton( this, wxID_ANY, Managers::GetString(L"#LAUNCH"), wxDefaultPosition, wxDefaultSize, 0 );
	nm_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );


	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butUpdate, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer2->Add( m_butLaunch, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer2->Add( nm_butCancel, 0, wxALL, 5 );
	
	fgSizer2->Add( bSizer2, 0, wxEXPAND, 5 );
	

	this->SetSizer( fgSizer2 );
	this->Layout();

	m_uiInternId = 0;
	m_bLaunch = true;

	centerOnParent();
}

UpdateInfoForm::~UpdateInfoForm()
{
	

}

void UpdateInfoForm::onFormClose( wxCloseEvent& event )
{
	g_pMainApp->closeForm(this->GetId());
}

void UpdateInfoForm::setInfo(DesuraId id, bool launch)
{
	m_bLaunch = launch;
	m_uiInternId = id;

	m_cbReminder->Enable(m_bLaunch);
	m_cbReminder->Show(m_bLaunch);

	

	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( id );

	if (!item)
	{
		Warning("Cant find item for update form.\n");
		return;
	}

	if (item->getIcon() && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(item->getIcon())))
		setIcon(item->getIcon());

	m_cbReminder->SetValue(m_bLaunch && item->getOptions() & UserCore::Item::ItemInfoI::OPTION_NOTREMINDUPDATE);

	SetLabel(gcWString(L"{0} {1}", Managers::GetString(L"#UI_TITLE"), item->getName()));

	m_ieBrowser->loadUrl(gcWString(L"http://www.desura.com/{0}/{1}/changelog", id.getTypeString(), item->getShortName()));
}

void UpdateInfoForm::onButtonPressed(wxCommandEvent& event)
{
	Close();

	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( m_uiInternId );

	if (!item)
		return;

	if (m_bLaunch)
	{
		if (item)
		{
			if (m_cbReminder->IsChecked())
				item->addOFlag(UserCore::Item::ItemInfoI::OPTION_NOTREMINDUPDATE);
			else
				item->delOFlag(UserCore::Item::ItemInfoI::OPTION_NOTREMINDUPDATE);
		}
	}

	if (event.GetId() == m_butUpdate->GetId())
	{
		g_pMainApp->handleInternalLink(m_uiInternId, ACTION_UPDATE, FormatArgs("show=true"));
	}
	else if (event.GetId() == m_butLaunch->GetId())
	{
		g_pMainApp->handleInternalLink(m_uiInternId, ACTION_LAUNCH, FormatArgs("noupdate"));
	}
}