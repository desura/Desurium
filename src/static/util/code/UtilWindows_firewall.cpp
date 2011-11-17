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
#include "util/UtilWindows.h"
#include "third_party/WinXPSP2FireWall.h"

namespace UTIL
{
namespace WIN
{

bool setFirewallAllow(const char* exePath, const char* name)
{
	if (!UTIL::FS::isValidFile(exePath))
		return false;

	CoInitialize(NULL);

	WinXPSP2FireWall fw;
	fw.Initialize();

	FW_ERROR_CODE err = fw.AddApplication(gcWString(exePath).c_str(), gcWString(name).c_str());

	CoUninitialize();

	return err == FW_NOERROR;
}

void delFirewallAllow(const char* exePath)
{
	if (!UTIL::FS::isValidFile(exePath))
		return;

	CoInitialize(NULL);

	WinXPSP2FireWall fw;
	fw.Initialize();

	FW_ERROR_CODE err = fw.RemoveApplication(gcWString(exePath).c_str());

	CoUninitialize();
}

}
}