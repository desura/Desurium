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

#ifndef DESURA_TOOLINSTALLFORM_H
#define DESURA_TOOLINSTALLFORM_H
#ifdef _WIN32
#pragma once
#endif


#include "wx_controls/gcControls.h"

namespace IPC
{
	class PipeServer;
}

void PipeIsActive();

class ToolInstallForm : public gcFrame
{
public:
	ToolInstallForm(wxWindow* parent, const char* key);
	~ToolInstallForm();

	void timerNotify();
	void pipeIsActive();

protected:
	void onDissconnect(uint32& id);
	void onConnect();

	void startServer();
	void stopServer();


	void onClose(wxCloseEvent& event);

	void startTimer();
	void stopTimer();


private:
	class ToolInstallThread;
	ToolInstallThread* m_pThread;

	bool m_bInstallInProgress;
	bool m_bIPCConnected;

	IPC::PipeServer *m_pIPCServer;

	uint32 m_uiTimerId;
	gcString m_szKey;
};


#endif //DESURA_TOOLINSTALLFORM_H
