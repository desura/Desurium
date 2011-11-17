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

#ifndef DESURA_MCFDPREPORTERI_H
#define DESURA_MCFDPREPORTERI_H
#ifdef _WIN32
#pragma once
#endif

#define MCF_DPREPORTER "MCF_DOWNLOADREPORTER_001"

namespace MCFCore
{


class DPReporterI
{
public:
	virtual uint32 getProviderCount()=0;
	virtual uint32 getProviderId(uint32 index)=0;

	virtual uint32 getLastRate(uint32 id)=0;
	virtual void getName(uint32 id, char* buff, uint32 size)=0;

	virtual uint64 getTotalSinceStart()=0;
	virtual void resetStart()=0;
};



}

#endif //DESURA_MCFDPREPORTERI_H
