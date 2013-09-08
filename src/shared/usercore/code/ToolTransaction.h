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

#ifndef DESURA_TOOLTRANSACTION_H
#define DESURA_TOOLTRANSACTION_H
#ifdef _WIN32
#pragma once
#endif

class IPCToolMain;

#include "ToolManager.h"

namespace UserCore
{
namespace Misc
{

class ToolTransInfo
{
public:
	ToolTransInfo(bool download, ToolTransaction* transaction, ToolManager* pToolManager);
	~ToolTransInfo();

	void removeItem(DesuraId id);

	void onDLComplete(DesuraId id);
	void onDLError(DesuraId id, gcException e);
	void onDLProgress(DesuraId id, UserCore::Misc::ToolProgress &prog);

	void onINComplete();
	void onINError(gcException &e);

	bool isDownload();
	bool startNextInstall(IPCToolMain* pToolMain, DesuraId &toolId);


	void getIds(std::vector<DesuraId> &idList);
	void startingIPC();
	void updateTransaction(Misc::ToolTransaction* pTransaction);


protected:
	size_t getIndex(DesuraId id);


private:
	bool m_bIsDownload;
	uint32 m_uiCompleteCount;

	Misc::ToolTransaction* m_pTransaction;
	std::vector<UserCore::Misc::ToolProgress> m_vProgress;

	ToolManager* m_pToolManager;
};

}
}

#endif //DESURA_TOOLTRANSACTION_H
