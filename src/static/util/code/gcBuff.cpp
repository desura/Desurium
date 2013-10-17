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

#include "Common.h"
#include "util/gcBuff.h"

gcBuff::gcBuff(uint32 size)
{
	m_cBuff = new char[size];
	m_uiSize = size;
}	

gcBuff::gcBuff(const char* src, uint32 size)
{
	m_cBuff = new char[size];
	m_uiSize = size;

	cpy(src, size);
}

gcBuff::gcBuff(gcBuff &buff)
{
	uint32 size = buff.size();

	if (size == 0)
	{
		m_cBuff = NULL;
		m_uiSize = 0;
	}
	else
	{
		m_cBuff = new char[size];
		m_uiSize = size;
		cpy(buff.c_ptr(), size);
	}
}

gcBuff::gcBuff(gcBuff *buff)
{
	if (!buff || buff->size() == 0)
	{
		m_cBuff = NULL;
		m_uiSize = 0;
	}
	else
	{
		uint32 size = buff->size();
		m_cBuff = new char[size];
		m_uiSize = size;
		cpy(buff->c_ptr(), size);
	}	
}

gcBuff::~gcBuff()
{
	safe_delete(m_cBuff);
}

char gcBuff::operator[] (uint32 i)
{
	if (i > m_uiSize)
		return '\0';

	return m_cBuff[i];
}

gcBuff::operator char*()
{
	return m_cBuff;
}

void gcBuff::cpy(const char* src, uint32 size)
{
	if (!src)
		return;

	if (size > m_uiSize)
		size = m_uiSize;

	memcpy(m_cBuff, src, size);
}

