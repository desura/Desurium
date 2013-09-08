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
#include "InstallDLPage.h"
#include "MainApp.h"

#define DEFAULT_BANNER "#icon_download"

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

InstallDLPage::InstallDLPage(wxWindow* parent) : InstallBannerPage(parent, true)
{
	m_butPause = new gcButton(this, wxID_ANY, Managers::GetString(L"#PAUSE"));
	m_butHide = new gcButton(this, wxID_ANY, Managers::GetString(L"#HIDE"));
	m_butCancel->SetLabel(Managers::GetString(L"#CANCEL"));

	m_pButSizer->Clear(false);
	m_pButSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	m_pButSizer->Add( m_butHide, 0,  wxTOP|wxBOTTOM|wxLEFT, 5 );
	m_pButSizer->Add( m_butPause, 0,  wxTOP|wxBOTTOM|wxLEFT, 5 );
	m_pButSizer->Add( m_butCancel, 0, wxALL, 5 );

	m_butPause->Enable(false);
	m_butCancel->Enable(false);

	this->Layout();

	m_bPaused = false;
	m_bError = false;
	m_bInit = true;
}

InstallDLPage::~InstallDLPage()
{
}

void InstallDLPage::init()
{
}

void InstallDLPage::onButtonPressed(wxCommandEvent& event)
{
	if (event.GetId() == m_butPause->GetId())
		getItemHandle()->setPaused(!m_bPaused);

	if (event.GetId() == m_butHide->GetId())
		GetParent()->Close();

	if (event.GetId() == m_butCancel->GetId())
		getItemHandle()->cancelCurrentStage();
}

void InstallDLPage::onPause(bool &state)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());

	m_bPaused = state;

	if (!state)
	{
		m_butPause->SetLabel(Managers::GetString(L"#PAUSE"));

		if (par)
			par->setProgressState(gcFrame::P_NORMAL);
	}
	else
	{
		m_labInfo->SetLabel(Managers::GetString(L"#UDF_UNKNOWNPAUSE"));
		m_butPause->SetLabel(Managers::GetString(L"#RESUME"));

		if (par)
			par->setProgressState(gcFrame::P_PAUSED);
	}
}

void InstallDLPage::onComplete(gcString& path)
{
	if (m_bError)
		return;

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	m_labInfo->SetLabel(wxT("Completed"));
}


class DownloadErrorHelper : public HelperButtonsI
{
public:
	DownloadErrorHelper(DesuraId id)
	{
		m_Id = id;
	}

	virtual uint32 getCount()
	{
		return 2;
	}

	virtual const wchar_t* getLabel(uint32 index)
	{
		if (index == 0)
			return Managers::GetString(L"#TRYAGAIN");

		return Managers::GetString(L"#MENU_CONSOLE");
	}

	virtual const wchar_t* getToolTip(uint32 index)
	{
		return NULL;
	}

	virtual void performAction(uint32 index)
	{
		if (index == 0)
			g_pMainApp->handleInternalLink(m_Id, ACTION_INSTALL);

		return g_pMainApp->handleInternalLink(m_Id, ACTION_SHOWCONSOLE);
	}

	DesuraId m_Id;
};

void InstallDLPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	m_bError = true;

	DownloadErrorHelper helper(getItemId());

	if (!getItemHandle()->shouldPauseOnError())
		gcErrorBox(GetParent(), "#IF_DLERRTITLE", "#IF_DLERROR", e, &helper);
}

void InstallDLPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	if (m_bInit && (info.flag & MCFCore::Misc::ProgressInfo::FLAG_INITFINISHED))
	{
		m_bInit = false;

		m_butPause->Enable(true);
		m_butCancel->Enable(true);
		m_pbProgress->setProgress(0);
	}
	else if (info.flag & MCFCore::Misc::ProgressInfo::FLAG_FINALIZING)
	{
		m_labInfo->SetLabel( Managers::GetString(L"#FINALIZING") );
		m_butPause->Enable(false);
		m_butCancel->Enable(false);
		m_pbProgress->setProgress(100);
	}
	else
	{
		if (!m_bInit)
		{
			std::string lab = UTIL::MISC::genTimeString(info.hour, info.min, info.rate);
			m_labInfo->SetLabel(lab);


			if (info.totalAmmount > 0)
			{
				std::string done = UTIL::MISC::niceSizeStr(info.doneAmmount, true);
				std::string total = UTIL::MISC::niceSizeStr(info.totalAmmount);

				m_pbProgress->setCaption(gcString("{0} of {1}", done, total));
			}
		}

		m_pbProgress->setProgress(info.percent);

		gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
		if (par)
			par->setProgress(info.percent);
	}

	Refresh(false);
}


}
}
}
