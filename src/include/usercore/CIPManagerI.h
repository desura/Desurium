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


#ifndef DESURA_CIPMANAGERI_H
#define DESURA_CIPMANAGERI_H
#ifdef _WIN32
#pragma once
#endif

namespace UserCore
{

namespace Misc
{
	class CIPItem
	{
	public:
		DesuraId id;
		gcString name;
		gcString path;
	};
}


class CIPManagerI
{
public:

	virtual void getCIPList(std::vector<Misc::CIPItem> &list)=0;
	virtual void getItemList(std::vector<Misc::CIPItem> &list)=0;

	virtual void updateItem(DesuraId id, gcString path)=0;
	virtual void deleteItem(DesuraId id)=0;

	virtual void refreshList()=0;
	virtual bool getCIP(UserCore::Misc::CIPItem& info)=0;

	virtual EventV& getItemsUpdatedEvent()=0;
};

}

#endif //DESURA_CIPMANAGERI_H
