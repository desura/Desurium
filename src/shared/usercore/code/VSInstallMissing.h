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

#ifndef DESURA_VSINSTALLMISSING_H
#define DESURA_VSINSTALLMISSING_H
#ifdef _WIN32
#pragma once
#endif

#include "VSBaseTask.h"
class IPCInstallMcf;

namespace UserCore
{
namespace ItemTask
{

class VSInstallMissing : public VSBaseTask
{
public:
	VSInstallMissing();
	~VSInstallMissing();

	virtual bool doTask();

	Event<MCFCore::Misc::ProgressInfo> onProgressEvent;

protected:
	virtual void onStop();
	void onComplete();

private:
	IPCInstallMcf *m_pIPCIM;

	::Thread::WaitCondition m_WaitCond;
	volatile bool m_bFinished;
};

}
}

#endif //DESURA_VSINSTALLMISSING_H
