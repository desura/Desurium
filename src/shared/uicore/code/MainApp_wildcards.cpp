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
#include "MainApp.h"

#include "SteamUser.h"
#include "managers/WildcardManager.h"
#include "managers/CVar.h"

CVar gc_steamuser("gc_steamuser", "", CFLAG_USER|CVAR_WINDOWS_ONLY);


void MainApp::processWildCards(WCSpecialInfo &info, wxWindow* parent)
{
	if (!info.handled)
	{
		if (Safe::stricmp("STEAMUSER", info.name.c_str()) == 0)
			getSteamUser(&info, parent);
	}

	info.processed = true;
}

void MainApp::getSteamUser(WCSpecialInfo *info, wxWindow *parent)
{
	if (gc_steamuser.getString() && strcmp(gc_steamuser.getString(), "") != 0)
	{
		info->result = gc_steamuser.getString();
		info->handled = true;
	}
	else
	{
		SteamUserDialog dlg(parent?parent:this);
		dlg.ShowModal();

		const char *steamuser = dlg.getSteamUser();

		if (steamuser)
		{
			gc_steamuser.setValue(steamuser);
			info->result = steamuser;
			info->handled = true;
		}
	}
}
