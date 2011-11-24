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

#ifndef DESURA_UPDATEFUNCTIONS_H
#define DESURA_UPDATEFUNCTIONS_H
#ifdef _WIN32
#pragma once
#endif

enum
{
	UPDATE_NONE,
	UPDATE_MCF,
	UPDATE_FILES,
	UPDATE_XML,
	UPDATE_SERVICE,
	UPDATE_CERT,
	UPDATE_FORCED,
	UPDATE_SERVICE_PATH,		//path is wrong but current service should launch
	UPDATE_SERVICE_LOCATION,	//path is worng and current service wont launch
	UPDATE_DATAPATH,			//not used
	UPDATE_SERVICE_HASH,
	UPDATE_SERVICE_DISABLED,
};

bool FileExists(const wchar_t* fileName);
int NeedUpdate();

void McfUpdate();
void FullUpdate();

bool CheckUpdate(const wchar_t* path);
bool CheckInstall();

bool MoveDataFolder();
bool ServiceUpdate(bool validService);
bool FixServiceDisabled();

#endif //DESURA_UPDATEFUNCTIONS_H
