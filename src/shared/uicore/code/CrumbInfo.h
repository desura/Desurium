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

#ifndef DESURA_CRUMBINFO_H
#define DESURA_CRUMBINFO_H
#ifdef _WIN32
#pragma once
#endif

class CrumbInfo
{
public:
	CrumbInfo(const wchar_t *n, const wchar_t* u, int c=0)
	{
		name = gcWString(n);
		url = gcWString(u);
		crumbId = c;
	}

	CrumbInfo(CrumbInfo* info)
	{
		if (!info)
			return;

		name = info->name;
		url = info->url;
		crumbId = info->crumbId;
	}

	CrumbInfo(CrumbInfo& info)
	{
		name = info.name;
		url = info.url;
		crumbId = info.crumbId;
	}

	gcWString name;
	gcWString url;
	int crumbId;
};


#endif //DESURA_CRUMBINFO_H
