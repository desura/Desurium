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
#include "CDKInfo.h"
#include "MainApp.h"

#include <wx/clipbrd.h>
#include <branding/branding.h>

class SplitInfo
{
public:
	SplitInfo()
	{
		start = -1;
		end = -1;
	}

	gcString text;
	int32 start;
	int32 end;
};


CDKInfo::CDKInfo(wxWindow* parent, const char* exe, bool launch) : BasePage(parent)
{
	m_butActivate = NULL;
	m_szExe = exe;
	m_bLaunch = launch;

	m_labInfo = new wxStaticText(this, wxID_ANY, Managers::GetString(L"#CDK_INFO"));

	if (launch)
		m_butLaunch = new gcButton(this, wxID_ANY, Managers::GetString(L"#CDK_LAUNCHNOW"));
	else
		m_butLaunch = new gcButton(this, wxID_ANY, Managers::GetString(L"#LAUNCH"));

	m_imgCopyPart = new gcImageButton(this, wxID_ANY, wxDefaultPosition, wxSize(21,21));
	m_imgCopyAll = new gcImageButton(this, wxID_ANY, wxDefaultPosition, wxSize(21,21));

	m_imgCopyPart->setDefaultImage("#button_copy_part");
	m_imgCopyPart->setHoverImage("#button_copy_part_hover");
	m_imgCopyPart->setFocusImage("#button_copy_part_focus");
	m_imgCopyPart->setDisabledImage("#button_copy_part_hover_nonactive");
	m_imgCopyPart->SetToolTip(Managers::GetString(L"#CDK_COPYPART"));

	m_imgCopyAll->setDefaultImage("#button_copy");
	m_imgCopyAll->setHoverImage("#button_copy_hover");
	m_imgCopyAll->setFocusImage("#button_copy_focus");
	m_imgCopyAll->setDisabledImage("#button_copy_hover_nonactive");
	m_imgCopyAll->SetToolTip(Managers::GetString(L"#CDK_COPYALL"));

	m_butClose = new gcButton(this, wxID_ANY, Managers::GetString("#CLOSE"));

	m_tbCdKey = new gcTextCtrl(this, wxID_ANY, "[Insert Cd Key Here]", wxDefaultPosition, wxSize(-1,-1), wxTE_CENTRE|wxTE_NOHIDESEL);
	m_tbCdKey->SetEditable(false);


	m_pButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	m_pButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	m_pButtonSizer->Add(m_butLaunch, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	m_pButtonSizer->Add(m_butClose, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add(m_tbCdKey, 1, wxEXPAND|wxBOTTOM|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	fgSizer2->Add(m_imgCopyPart, 0, wxBOTTOM|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	fgSizer2->Add(m_imgCopyAll, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );


	wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 6, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add(m_labInfo, 1, wxALL, 5);
	fgSizer1->Add(fgSizer2, 1, wxEXPAND, 5 );
	fgSizer1->Add(0, 0, 0, wxEXPAND, 5);
	fgSizer1->Add(m_pButtonSizer, 1, wxEXPAND, 5);
	
	this->SetSizer( fgSizer1 );
	this->Layout();

	setParentSize(-1, 120);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CDKInfo::onButtonClicked, this);

	m_uiCurIndex = 0;
}

CDKInfo::~CDKInfo()
{
	safe_delete(m_vSplitKey);
}

void CDKInfo::setInfo(DesuraId id, const char* key)
{
	BasePage::setInfo(id);

	UserCore::Item::ItemInfoI *info = getItemInfo();

	if (!info)
	{
		GetParent()->Close();
		return;
	}

	m_butLaunch->Enable(info->isLaunchable());

	m_tbCdKey->ChangeValue(key);
	tokenizeKey(key);

	UserCore::Item::BranchInfoI* cb = info->getCurrentBranch();

	if (cb && cb->isSteamGame())
	{
		m_labInfo->SetLabel(gcWString(Managers::GetString(L"#CDK_INFO_STEAM"), info->getName()));
		m_labInfo->Wrap(360);

		m_butActivate = new gcButton(this, wxID_ANY, Managers::GetString(L"#CDK_ACTIVATE"));

		m_pButtonSizer->Clear(false);
		m_pButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
		m_pButtonSizer->Add(m_butActivate, 0, wxTOP|wxBOTTOM|wxLEFT, 5);
		m_pButtonSizer->Add(m_butLaunch, 0, wxTOP|wxBOTTOM|wxLEFT, 5);
		m_pButtonSizer->Add(m_butClose, 0, wxALL, 5);

		Layout();
		Refresh(false);

		setParentSize(-1, 140);
	}
}

void CDKInfo::tokenizeKey(const char* key)
{
	const char* pos = key;
	const char* start = key;

	while (pos[0] != '\0')
	{
		if (pos[0] == '-' || pos[0] == ' ')
		{
			tokenizeKey(start, pos, key);
			start = pos+1;
		}

		pos++;
	}

	tokenizeKey(start, pos, key);

	if (m_vSplitKey.size()==0)
		m_imgCopyPart->Disable();
}

void CDKInfo::tokenizeKey(const char* start, const char* end, const char* key)
{
	SplitInfo *info = new SplitInfo();

	info->start = start-key;
	info->end = end-key;
	info->text.assign(start, (size_t)(end-start));

	m_vSplitKey.push_back(info);
}

void CDKInfo::onButtonClicked(wxCommandEvent& event)
{
	if (event.GetId() == m_butClose->GetId())
	{
		GetParent()->Close();
	}
	else if (event.GetId() == m_imgCopyAll->GetId() || (m_vSplitKey.size() == 0 && event.GetId() == m_imgCopyPart->GetId()))
	{
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(m_tbCdKey->GetValue()));
			wxTheClipboard->Close();
		}	

		m_tbCdKey->SetSelection(-1,-1);
	}
	else if (event.GetId() == m_imgCopyPart->GetId())
	{
		SplitInfo *info = m_vSplitKey[m_uiCurIndex];

		m_uiCurIndex++;

		if (m_uiCurIndex >= m_vSplitKey.size())
			m_uiCurIndex = 0;
		
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(info->text));
			wxTheClipboard->Close();
		}

		m_tbCdKey->SetSelection(info->start,info->end);

		gcString label(Managers::GetString("#CDK_COPYPART"));

		if (m_uiCurIndex != 0)
			label = gcString(Managers::GetString("#CDK_COPYPART_X"), m_uiCurIndex+1);

		m_imgCopyPart->SetToolTip(label);
	}
	else if (m_butLaunch && m_butLaunch->GetId() == event.GetId())
	{
		g_pMainApp->handleInternalLink(getItemId(), ACTION_LAUNCH, FormatArgs("cdkey", std::string("exe=") + m_szExe));
		GetParent()->Close();
	}
	else if (m_butActivate && m_butActivate->GetId() == event.GetId())
	{
		UserCore::Item::ItemInfoI* info = getItemInfo();

		if (info && info->isLaunchable())
			info->addSFlag(UserCore::Item::ItemInfoI::STATUS_LAUNCHED);

#ifdef WIN32
		UTIL::WIN::runAs("steam://open/activateproduct");
#endif
	}
}
