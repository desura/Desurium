/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_DLLVERSION_H
#define DESURA_DLLVERSION_H
#ifdef _WIN32
#pragma once
#endif

namespace WebCore
{
namespace Misc
{

//! Class holds the version numbers of all the modules
class DLLVersion
{
public:
	//! Default constuctor
	//!
	DLLVersion()
	{
	}

	//! Constuctor
	//!
	//! @param d Desura Exe Version
	//! @param ui UICore Version
	//! @param w WebCore Version
	//! @param m MCFCore Version
	//! @param u UserCore Version
	//!
	DLLVersion(const char* d, const char* ui, const char* w, const char* m, const char* u)
	:	szMcfVer(m),
		szDEVer(d),
		szUIVer(ui),
		szWebVer(w),
		szUserVer(u)
	{}

	//! Copy Constuctor
	//!
	//! @param dv object to copy from
	//!
	DLLVersion(DLLVersion* dv)
	{
		if (dv)
		{
			szMcfVer = dv->szMcfVer;
			szDEVer = dv->szDEVer;
			szUIVer = dv->szUIVer;
			szWebVer = dv->szWebVer;
			szUserVer = dv->szUserVer;
		}
	}

	gcString szMcfVer;
	gcString szDEVer;
	gcString szUIVer;
	gcString szWebVer;
	gcString szUserVer;
};

}
}

#endif //DESURA_DLLVERSION_H
