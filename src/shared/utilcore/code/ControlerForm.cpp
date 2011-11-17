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
#include "ControlerForm.h"

#include <wx/snglinst.h>

#include "usercore\UserCoreI.h"
#include "wx_controls/gcMessageBox.h"

#include "UninstallDesuraForm.h"
#include "CrashDumpForm.h"
#include "McfViewerForm.h"
#include "ChangeDirForm.h"
#include "ToolInstallForm.h"

#ifdef DEBUG
//#include "BrowserTest.h"
#include "FlashDebug.h"
#endif

void SetExitCode(int32 code);

ControllerForm::ControllerForm() : wxFrame(NULL, wxID_ANY, "HiddenWindow")
{
	m_pChecker = NULL;
	m_pUser = NULL;
	Show(false);
}

ControllerForm::~ControllerForm()
{
	if (m_pUser)
		m_pUser->logOut(false, false);

	safe_delete(m_pUser);
	safe_delete(m_pChecker);
}

bool ControllerForm::init(int argc, wxCmdLineArgsArray &argv)
{
	std::map<std::string,bool> formList;
	bool isMcfFile = false;

	gcString mcf;

	wxFrame *form = NULL;

	for (int x=0; x<argc; x++)
	{
		formList[argv[x].c_str().operator const char *()] = true;
		
		if (argv[x].Find(".mcf") != -1)
			mcf = gcString(argv[x].c_str().operator const char *());
	}

	isMcfFile = mcf.size() > 0;

	if (formList["-uninstall"] == true)
	{
		if (!setUpSIC(L"Desura UnInstaller"))
			return false;

		if (!setUpUserCore())
			return false;

		form = new UninstallForm(this, m_pUser);
	}
	else if (formList["-crashdump"] == true)
	{
		form = new CrashDumpForm(this);
	}
#ifdef DEBUG
	//else if (formList["-browsertest"] == true)
	//{
	//	form = new BrowserTest(this);
	//}
	else if (formList["-flashdebug"] == true)
	{
		form = new FlashDebug(this);
	}
#endif
	else if (formList["-mcfviewer"] == true || isMcfFile)
	{
		form = new McfViewerForm(this, mcf);
	}
	else if (formList["-toolhelper"] == true)
	{
		gcString key;

		for (int x=0; x<argc; x++)
		{
			if (argv[x] == "-key")
			{
				if (x+1 < argc)
					key = argv[x+1].c_str().operator const char *();
			}
		}

		form = new ToolInstallForm(this, key.c_str());
		return true;
	}
	else if (formList["-setcachedir"] && formList["-dir"])
	{
		if (!setUpSIC(L"Desura Utility"))
			return false;

		gcString dir;

		for (int x=0; x<argc; x++)
		{
			if (argv[x] == "-dir")
			{
				if (x+1 < argc)
					dir = argv[x+1].c_str().operator const char *();
			}
		}

		if (!setUpUserCore())
			return false;

		form = new ChangeDirForm(this, dir.c_str(), m_pUser);
	}

	if (form)
		form->Show();

	return form?true:false;
}

bool ControllerForm::setUpSIC(const wchar_t* title)
{
	m_pChecker = new wxSingleInstanceChecker(wxT("DesuraRepair"));

	if (!m_pChecker->IsAnotherRunning())
		return true;

	wxMessageBox(wxT("Another instance is all ready running."), title);
	SetExitCode(-1);
	return false;
}

bool ControllerForm::setUpUserCore()
{
	safe_delete(m_pUser);
	m_pUser = (UserCore::UserI*)UserCore::FactoryBuilderUC(USERCORE);
	m_pUser->init(gcString(UTIL::OS::getAppDataPath()).c_str());

	try
	{
		m_pUser->logInCleanUp();
	}
	catch (gcException &e)
	{
		m_pUser->logOut();
		safe_delete(m_pUser);
		
		gcErrorBox(this, "Desura", "Failed to load item information!", e);
		SetExitCode(-1);
		return false;
	}

	return true;
}