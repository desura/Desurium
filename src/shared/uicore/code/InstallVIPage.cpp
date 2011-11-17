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
#include "InstallVIPage.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

///////////////////////////////////////////////////////////////////////////////
/// InstallGIPage
///////////////////////////////////////////////////////////////////////////////

InstallVIPage::InstallVIPage(wxWindow* parent) : InstallBannerPage(parent)
{
	m_iStage = 0;

	m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYLABEL"));
	m_labInfo->SetLabel(L"");
}

InstallVIPage::~InstallVIPage()
{
}

void InstallVIPage::init()
{

}

void InstallVIPage::onButtonPressed(wxCommandEvent& event)
{
	if (event.GetId() == m_butCancel->GetId())
	{
		getItemHandle()->setPaused(true);
		GetParent()->Close();
	}

	InstallBannerPage::onButtonPressed(event);
}

void InstallVIPage::onVerifyComplete(UserCore::Misc::VerifyComplete& info)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	if (info.vStatus == UserCore::Misc::VerifyComplete::V_INSTALL)
	{
	}
	else if (info.vStatus == UserCore::Misc::VerifyComplete::V_DOWNLOAD)
	{		
	}
	else
	{
		m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));
		m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYCOMPLETE"));
		m_pbProgress->setProgress(100);
	}
}

void InstallVIPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	if (!getItemHandle()->shouldPauseOnError())
		gcErrorBox(GetParent(), "#IF_VERRTITLE", "#IF_VERROR", e);
}

void InstallVIPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	m_pbProgress->setProgress(info.percent);

	if (info.flag != m_iStage)
	{
		m_iStage = info.flag;

		switch (m_iStage)
		{
			case 0: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYLABEL"));	break;
			case 1: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYMCF"));		break;
			case 2: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYINSTALL"));	break;
			case 3: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYDOWNLOADF"));break;
			case 4: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYINSTALLF"));	break;
			case 5: m_labLabel->SetLabel(Managers::GetString(L"#IF_VERIFYBRANCH"));	break;
		}

		m_pbProgress->setCaption(gcString());
	}

	if (m_iStage == 3 && info.totalAmmount > 0) //downloading
	{
		std::string done = UTIL::MISC::niceSizeStr(info.doneAmmount, true);
		std::string total = UTIL::MISC::niceSizeStr(info.totalAmmount);

		m_pbProgress->setCaption(gcString("{0} of {1}", done, total));
	}

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(info.percent);

	Update();
}

}
}
}