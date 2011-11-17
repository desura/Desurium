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

#include "common.h"
#include "InstallHandler.h"
#include "User.h"

InstallHandler::InstallHandler(UserCore::Item::ItemHandle* handle, const char* path) : BaseHandler(handle)
{
	m_szPath = path;
	
}

InstallHandler::~InstallHandler()
{

}







//res will be 1 on error (i.e. hash fail) and 0 if no error
void InstallHandler::onComplete(uint32 res)
{

}

void InstallHandler::onError(gcException e)
{

}

void InstallHandler::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{

}