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

#ifndef DESURA_INSTALLHANDLER_H
#define DESURA_INSTALLHANDLER_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseHandler.h"

class InstallHandler : public BaseHandler
{
public:
	InstallHandler(UserCore::Item::ItemHandle* handle, const char* path);
	~InstallHandler();

	virtual UserCore::Thread::UserServiceI* newService();

protected:
	virtual void onError(gcException e);
	virtual void onComplete(uint32 res);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);

private:
	gcString m_szPath;
	UserCore::Item::Helper::InstallerHandleHelperI* m_pIHH;
};


#endif //DESURA_INSTALLHANDLER_H
