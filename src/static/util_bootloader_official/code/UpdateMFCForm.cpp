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

#include "stdafx.h"
#define UTILWEB_INCLUDE

#include "Common.h"
#include "UMcfEx.h"

#include "UpdateMFCForm.h"
#include "AppUpdateInstall.h"

#include <process.h>
#include "UpdateFunctions.h"
#include <shlobj.h>

using namespace Desurium;

INT_PTR DisplayUpdateWindow(int updateType)
{
	UpdateForm dlg(updateType);
	return dlg.DoModal();
}

UINT __stdcall InstallFilesThread(void *form);
UINT __stdcall InstallMcfThread(void *form);



UpdateForm::UpdateForm(int updateType) 
	: Desurium::CDesuraDialog(IDD_UPDATEFORM_DIALOG)
{
	m_bInit = false;
	m_hIcon = LoadIcon(IDI_ICON);
	m_Progress = new DesuraProgress();
	m_updateType = updateType;
}

UpdateForm::~UpdateForm()
{
}


bool UpdateForm::OnInitDialog()
{
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect prect(0,0,414,218);
	BOOL res = m_Progress->Create(this, prect, IDC_PROGRESS, WS_VISIBLE|WS_CHILD);

	m_bInit = true;


	RECT clientRect;
	DWORD dwStyle =   ::GetWindowLong((HWND)GetSafeHwnd(), GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong((HWND)GetSafeHwnd(), GWL_EXSTYLE);
	::GetClientRect((HWND)GetSafeHwnd(), &clientRect);
	::AdjustWindowRectEx(&clientRect, dwStyle, ::GetMenu((HWND)GetSafeHwnd()) != NULL, dwExStyle);

	clientRect.top *= -1;
	clientRect.left *= -1;

	int w = clientRect.right - clientRect.left;// - clientRect.left;
	int h = clientRect.bottom - clientRect.top;// - clientRect.left;

	HRGN region = CreateRoundRectRgn(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom+clientRect.top-clientRect.left, 5, 5);
	SetWindowRgn(region, true);

	return TRUE;
}

INT_PTR UpdateForm::DoModal()
{
	if (m_updateType != -1)
	{
		if (m_updateType == UPDATE_MCF)
			UpdateMcf();
		else
			UpdateFiles();
	}

	return CDesuraDialog::DoModal();
}


void UpdateForm::UpdateFiles()
{
	BeginThread(InstallFilesThread, (void*)this);
}

void UpdateForm::UpdateMcf()
{
	BeginThread(InstallMcfThread, (void*)this);
}

void UpdateForm::onProgressN(unsigned int& prog)
{
	m_Progress->setProgress(prog);
	m_Progress->refresh();
}

void UpdateForm::onProgressD(Prog_s& info)
{
	uint64 done = (uint32)info.dlnow; //+= (uint64)size;
	uint32 per = 0;

	if (done > 0)
		per = (uint32)(done*100/info.dltotal);

	m_Progress->setProgress(per);
	m_Progress->setDone((int)info.dlnow);
	m_Progress->setTotal((int)info.dltotal);
	m_Progress->refresh();
}



bool FileExists(const wchar_t* fileName);


UINT DownloadAndInstallMCF(UpdateForm* temp);

//this should only be called if an update is needed.
UINT __stdcall InstallFilesThread(void *form)
{
	UpdateForm* temp = static_cast<UpdateForm*>(form);

	if (!temp)
	{
		::MessageBox(NULL, "Failed to find update form!", "Desura: ERROR", MB_OK);
		exit(-1);
	}

	while (!temp->isInit())
		gcSleep(500);

	if (FileExists(L"update_lock.txt"))
	{
		DWORD ret = ::MessageBox(temp->GetSafeHwnd(), "It looks like Desura tried to update and failed. Want to try again?", "Desura", MB_YESNO);
		DeleteFileW(L"update_lock.txt");

		if (ret == IDNO)
		{
			temp->EndDialog(2);
			return 2;
		}
	}

	FILE* fh = Safe::fopen("update_lock.txt", "wb");

	if (fh)
		fclose(fh);


	UINT res = DownloadAndInstallMCF(temp);


	DeleteFile("update_lock.txt");

	temp->EndDialog(res);
	return res;
}

//this should only be called if an update is needed.
UINT __stdcall InstallMcfThread(void *form)
{
	int res = 0;
	UpdateForm* temp = static_cast<UpdateForm*>(form);

	if (!temp)
	{
		::MessageBox(NULL, "Failed to find update form!", "Desura: ERROR", MB_OK);
		return 1;
	}

	while (!temp->isInit())
		gcSleep(500);

	temp->getProgressControl()->setMode(MODE_INSTALLING);

	AppUpdateInstall aui(temp);
	res = aui.run();

	if (res != 0)
	{
		char msg[255] ={0};
		_snprintf_s(msg, 255, "Failed to install update [%d]!", res);
		::MessageBox(temp->GetSafeHwnd(), msg, "Desura: ERROR", MB_OK);

		std::wstring path = UTIL::OS::getAppDataPath(UPDATEFILE_W);
		DeleteFileW(path.c_str());

		return 1;
	}

	temp->EndDialog(res);

	return 0;
}
