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

#ifndef DESURA_UNINSTALLTHREAD_H
#define DESURA_UNINSTALLTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "UIBaseServiceTask.h"

class IPCUninstallMcf;

namespace UserCore
{
namespace ItemTask
{


class UIServiceTask : public UIBaseServiceTask
{
public:
	UIServiceTask(UserCore::Item::ItemHandle* handle, bool removeAll, bool removeAcc);
	~UIServiceTask();

protected:
	virtual bool initService();
	virtual void onComplete();

	virtual void onServiceError(gcException& e);

private:
	IPCUninstallMcf* m_pIPCUI;

	bool m_bRemoveAll;
	bool m_bRemoveAcc;
	volatile bool m_bRunning;
};

}
}

#endif
