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

#ifndef DESURA_GCJSSETTINGS_H
#define DESURA_GCJSSETTINGS_H
#ifdef _WIN32
#pragma once
#endif


#include "gcJSBase.h"



class DesuraJSSettings : public DesuraJSBase<DesuraJSSettings>
{
public:
	DesuraJSSettings();

private:
	std::vector<MapElementI*> getLanguages();
	std::vector<gcString> getSteamNames();
	std::vector<MapElementI*> getThemes();

	int32 getCoreCount();
	gcString getValue(gcString name);

	void setCachePath(gcString path);
	void setValue(gcString name, gcString val);

	std::vector<MapElementI*> getCurrentCIPItems();
	std::vector<MapElementI*> getAllCIPItems();

	void updateCIPList();
	void saveCIPList(std::vector<std::map<gcString, gcString>> list);

	bool isValidCIPPath(gcString path);
	gcWString browseCIPPath(gcWString name, gcWString path);

	bool isValidLinkBinary(gcString path);
	gcString browseLinkBinary(gcString name, gcString path);
};


#endif //DESURA_GCJSSETTINGS_H
