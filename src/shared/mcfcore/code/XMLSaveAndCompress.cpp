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
#include "XMLSaveAndCompress.h"

#define BUFFSIZE 10024

XMLSaveAndCompress::XMLSaveAndCompress(UTIL::FS::FileHandle *fh, bool compress) : m_BZ2Worker(UTIL::MISC::BZ2_COMPRESS), m_fhFile(fh)
{
	m_bCompress = compress;
	m_szReadBuf = NULL;

	if (compress)
		m_szReadBuf = new char[BUFFSIZE+1];

	m_uiLastWritePos = 0;
	m_uiTotalSize = 0;
	m_uiRawSize = 0;
	
	m_szWriteBuf = new char[BUFFSIZE+1];
}

XMLSaveAndCompress::~XMLSaveAndCompress()
{
	safe_delete(m_szReadBuf);
	safe_delete(m_szWriteBuf);
}

void XMLSaveAndCompress::finish()
{
	write(m_szWriteBuf, m_uiLastWritePos, true);
}

void XMLSaveAndCompress::save(const char* str, size_t size)
{
	if (!str || size == 0)
		return;

	if (m_uiLastWritePos + size < BUFFSIZE)
	{
		memcpy(m_szWriteBuf+m_uiLastWritePos, str, size);
		m_uiLastWritePos += size;
	}
	else
	{
		write(m_szWriteBuf, m_uiLastWritePos, false);
		m_uiLastWritePos = 0;

		if (size > BUFFSIZE)
		{
			write(const_cast<char*>(str), size, false);
		}
		else
		{
			memcpy(m_szWriteBuf, str, size);
			m_uiLastWritePos = size;
		}
	}
}

uint64 XMLSaveAndCompress::getTotalSize()
{
	return m_uiTotalSize;
}

void XMLSaveAndCompress::write(char* buff, size_t size, bool end)
{
	m_uiRawSize += size;

	if (m_bCompress)
	{
		if ((buff && size > 0) || end)
			m_BZ2Worker.write(buff, size, end);

		m_BZ2Worker.doWork();

		size_t readSize = m_BZ2Worker.getReadSize();

		if (readSize > BUFFSIZE || end)
		{
			size_t readAmmount = BUFFSIZE;

			do
			{
				readAmmount = BUFFSIZE;
				m_BZ2Worker.read(m_szReadBuf, readAmmount);
				m_fhFile->write(m_szReadBuf, readAmmount);

				m_uiTotalSize += readAmmount;
			}
			while (end && readAmmount > 0);
		}
	}
	else
	{
		m_uiTotalSize += size;

		if (size > 0 && buff)
			m_fhFile->write(buff, size);
	}
}
