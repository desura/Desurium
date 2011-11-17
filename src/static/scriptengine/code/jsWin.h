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


#ifndef DESURA_JSWIN_H
#define DESURA_JSWIN_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"

class OsJSBinding : public DesuraJSBase<OsJSBinding>
{
public:
	OsJSBinding();
	~OsJSBinding();

protected:
	void SetRegistryKey(gcString key, gcString data);
	void DelRegistryKey(gcString key);
	gcString GetRegistryKey(gcString key);

	bool SetFirewallAllow(gcString exePath, gcString name);
	void DelFirewallAllow(gcString exePath);

	void SetCompatiblityMode(gcString exePath, int32 os, int32 flags);

	bool IsLinux();
	bool IsWindows();
};


#endif //DESURA_JSWIN_H
