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
#include "User.h"
#include "usercore/UserCoreI.h"
#include "../../branding/usercore_version.h"

gcString g_szUserCoreVersion("{0}.{1}.{2}.{3}", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);


namespace UserCore
{

	const char* GetUserCoreVersion()
	{
		return g_szUserCoreVersion.c_str();
	}

extern "C"
{

	CEXPORT void* FactoryBuilderUC(const char* name)
	{
		if (strcmp(USERCORE, name) == 0)
		{
			UserCore::User* temp = new UserCore::User();
			return temp;
		}
		else if (strcmp(USERCORE_VER, name) == 0)
		{
			return (void *)&GetUserCoreVersion;
		}
		else if (strcmp(USERCORE_GETLOGIN, name) == 0)
		{
			return (void *)&UserCore::User::getLoginInfo;
		}

		return NULL;
	}

}
}
