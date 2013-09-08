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


#ifndef DESURA_MCF_MAIN_H
#define DESURA_MCF_MAIN_H

#include "MCFI.h"

namespace MCFCore
{
	CEXPORT void* FactoryBuilder(const char* name);
	CEXPORT void FactoryDelete(void *p, const char* name);
}

extern "C" CEXPORT const char* GetMCFCoreVersion();

//! Basic handler class for mcf files that auto cleans up
class McfHandle
{
public:
	McfHandle()
	{
		m_pMcf = NULL;
	}

	McfHandle(MCFCore::MCFI *MCF)
	{
		m_pMcf = NULL;
		setHandle(MCF);
	}

	McfHandle(const McfHandle &handle)
	{
		m_pMcf = NULL;
		setHandle(handle.m_pMcf);
		
		handle.m_pMcf = NULL;
	}

	~McfHandle()
	{
		if (m_pMcf)
			MCFCore::FactoryDelete(m_pMcf, MCF_FACTORY);

		m_pMcf = NULL;
	}

	MCFCore::MCFI* operator ->()
	{
		if (!m_pMcf)
			m_pMcf = (MCFCore::MCFI*)MCFCore::FactoryBuilder(MCF_FACTORY);

		return m_pMcf;
	}

	MCFCore::MCFI* handle() const
	{
		return m_pMcf;
	}

	MCFCore::MCFI* releaseHandle()
	{
		MCFCore::MCFI* temp = m_pMcf;
		m_pMcf = NULL;
		return temp;
	}

	void setHandle(MCFCore::MCFI* handle)
	{
		if (m_pMcf)
			MCFCore::FactoryDelete(m_pMcf, MCF_FACTORY);

		m_pMcf = handle;
	}

private:
	mutable MCFCore::MCFI* m_pMcf;
};

#endif
