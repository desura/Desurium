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

#ifndef DESURA_GCJSBRANCHINFO_H
#define DESURA_GCJSBRANCHINFO_H
#ifdef _WIN32
#pragma once
#endif


#include "gcJSBase.h"

namespace UserCore
{
	namespace Item
	{
		class ItemInfoI;
		class BranchInfoI;
	}
}


class DesuraJSBranchInfo : public DesuraJSBase<DesuraJSBranchInfo>
{
public:
	DesuraJSBranchInfo();
	virtual bool preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv);

private:
	bool isValidBranch(UserCore::Item::BranchInfoI* branch);
	int32 getBranchFlags(UserCore::Item::BranchInfoI* branch);

	gcString getItemId(UserCore::Item::BranchInfoI* branch);
	gcString getBranchId(UserCore::Item::BranchInfoI* branch);
	gcString getBranchName(UserCore::Item::BranchInfoI* branch);
	gcString getBranchCost(UserCore::Item::BranchInfoI* branch);
	gcString getBranchPreorderDate(UserCore::Item::BranchInfoI* branch);

	bool isWindows(UserCore::Item::BranchInfoI* branch);
	bool isLinux(UserCore::Item::BranchInfoI* branch);
	
	bool is32Bit(UserCore::Item::BranchInfoI* branch);
	bool is64Bit(UserCore::Item::BranchInfoI* branch);


	uint32 m_uiIsValidHash;
};



#endif //DESURA_GCJSBRANCHINFO_H
