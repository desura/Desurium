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


#ifndef DESURA_VERIFYCOMPLETE_H
#define DESURA_VERIFYCOMPLETE_H
#ifdef _WIN32
#pragma once
#endif

namespace UserCore
{
namespace Misc
{

class VerifyComplete
{
public:
	enum VSTATUS
	{
		V_COMPLETE = 0,
		V_DOWNLOAD,
		V_INSTALL,
		V_SWITCHBRANCH,
		V_RESET,
	};

	VerifyComplete()
	{
		vStatus = V_COMPLETE;
	}

	VerifyComplete(VSTATUS status, const char* file = NULL)
	{
		if (file)
			szFile = gcString(file);

		vStatus = status;
	}

	gcString szFile;
	VSTATUS vStatus;
};

}
}

#endif //DESURA_VERIFYCOMPLETE_H
