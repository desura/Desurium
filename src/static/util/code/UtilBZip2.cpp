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
#include "util/UtilMisc.h"
#include "BZip2.h"

#include <iterator>

namespace UTIL
{
namespace MISC
{

	
class BZ2WorkerData
{
public:
	BZ2WorkerData(uint32 type)
	{
		m_uiType = type;
		memset(&strm, 0, sizeof(bz_stream));

		if (m_uiType == BZ2_COMPRESS)
			m_iInitRes = BZ2_bzCompressInit(&strm, 9, 0, 0);
		else
			m_iInitRes = BZ2_bzDecompressInit(&strm, 0, 0);

		m_bEnd = false;
		m_bWriteEnd = false;
	}

	~BZ2WorkerData()
	{
		if (m_uiType == BZ2_COMPRESS)
			BZ2_bzCompressEnd(&strm);
		else
			BZ2_bzDecompressEnd(&strm);
	}

	bool isInit(int &res)
	{
		res = m_iInitRes;
		return (m_iInitRes == BZ_OK);
	}

	void doWork()
	{
		if (m_bEnd)
			return;

		if (m_uiType == BZ2_COMPRESS)
			doCompress();
		else
			doDecompress();
	}

	void read(char* buff, size_t &size)
	{
		if (size > m_DataOutStream.size())
			size = m_DataOutStream.size();

#if defined(WIN32) && !defined(__MINGW32__)
		std::copy(m_DataOutStream.begin(), m_DataOutStream.begin() + size, stdext::checked_array_iterator<char*>(buff, size));
#else
		std::copy(m_DataOutStream.begin(), m_DataOutStream.begin() + size, buff);
#endif
		m_DataOutStream.erase(m_DataOutStream.begin(), m_DataOutStream.begin()+size);
	}

	void write(const char* buff, size_t size, bool end)
	{
		if (m_bEnd || m_bWriteEnd)
			return;

		if (size > 0)
		{
			if (!buff)
				throw gcException(ERR_INVALIDDATA, "Buffer is null for bz2worker");

			m_DataInStream.reserve(m_DataInStream.size() + size);
			m_DataInStream.insert(m_DataInStream.end(), &buff[0], &buff[size]);
		}

		m_bWriteEnd = end;
	}

	uint32 getReadSize()
	{
		return m_DataOutStream.size();
	}

	int32 getLastStatus()
	{
		return m_iLastError;
	}

	void writeCB(const char* buff, size_t size, UTIL::CB::CallbackI* callback)
	{
		write(buff, size, false);
		doWork();

		uint32 read = getReadSize();

		if (read > 0 && callback)
			callback->onData((const unsigned char*)&m_DataOutStream[0], read);

		m_DataOutStream.clear();
	}

	void endCB(UTIL::CB::CallbackI* callback)
	{
		write(NULL, 0, true);

		do
		{
			doWork();

			uint32 read = getReadSize();

			if (read >0 && callback)
				callback->onData((const unsigned char*)&m_DataOutStream[0], read);

			m_DataOutStream.clear();
		}
		while (m_iLastError != BZ_STREAM_END);
	}

protected:
	void doCompress()
	{
		const size_t buffsize = 10*1024;
		strm.avail_in = m_DataInStream.size();

		//dont work if we have nothing to write and we are not at the end
		if (strm.avail_in == 0 && !m_bWriteEnd)
			return;

		if (strm.avail_in == 0)
			strm.next_in = NULL;
		else
			strm.next_in = &m_DataInStream[0];

		if (m_bWriteEnd && strm.total_in_lo32 == 0 && strm.total_in_hi32 == 0 && m_DataInStream.size() == 0)
		{
			m_bEnd = true;
			m_iLastError = BZ_STREAM_END;
			return;
		}

		do
		{
			strm.next_out = buff;
			strm.avail_out = buffsize;

			m_iLastError = BZ2_bzCompress(&strm, m_bWriteEnd?BZ_FINISH:BZ_RUN );
			if (m_iLastError != BZ_RUN_OK && m_iLastError != BZ_FINISH_OK && m_iLastError != BZ_STREAM_END)
			{
				m_bEnd = true;
				throw gcException(ERR_BZ2, m_iLastError, "Failed to compress buffer");
			}
			else
			{
				size_t x = buffsize - strm.avail_out;

				//reserve twice as much to reduce overhead
				if (m_DataOutStream.capacity() < (m_DataOutStream.size() + x))
					m_DataOutStream.reserve(m_DataOutStream.size() + x*2);

				m_DataOutStream.insert(m_DataOutStream.end(), &buff[0], &buff[x]);

				if (m_iLastError == BZ_STREAM_END)
					m_bEnd = true;
			}
		}
		while ((strm.avail_in > 0) || (m_bWriteEnd && m_iLastError != BZ_STREAM_END));

		if (m_iLastError == BZ_RUN_OK)
			m_iLastError = BZ_OK;

		m_DataInStream.clear();
	}

	void doDecompress()
	{
		const size_t buffsize = 10*1024;

		if (m_bWriteEnd && strm.total_in_lo32 == 0 && strm.total_in_hi32 == 0 && m_DataInStream.size() == 0)
		{
			m_bEnd = true;
			m_iLastError = BZ_STREAM_END;
			return;
		}

		do
		{
			strm.avail_in = m_DataInStream.size();

			if (strm.avail_in == 0)
				strm.next_in = NULL;
			else
				strm.next_in = &m_DataInStream[0];

			strm.next_out = buff;
			strm.avail_out = buffsize;

			m_iLastError = BZ2_bzDecompress(&strm);
			if (m_iLastError != BZ_OK && m_iLastError != BZ_STREAM_END)
			{
				m_bEnd = true;
				throw gcException(ERR_BZ2, m_iLastError, "Failed to decompress buffer");
			}
			else
			{
				if (m_bWriteEnd && m_DataInStream.size() == 0 && strm.total_out_hi32 == 0 && strm.total_out_lo32 == 0)
					m_iLastError = BZ_STREAM_END;
				
				m_DataInStream.erase(m_DataInStream.begin(), m_DataInStream.end()-strm.avail_in);
				size_t x=buffsize - strm.avail_out;

				if (x > 0)
				{
					//reserve twice as much to reduce overhead
					if (m_DataOutStream.capacity() < (m_DataOutStream.size() + x))
						m_DataOutStream.reserve(m_DataOutStream.size() + x*2);

					m_DataOutStream.insert(m_DataOutStream.end(), &buff[0], &buff[x]);
				}

				if (m_iLastError == BZ_STREAM_END)
					m_bEnd = true;
			}
		}
		while ((strm.avail_out == 0 && !m_bEnd) || (m_bWriteEnd && !m_bEnd));
	}

private:
	int32 m_iLastError;
	int32 m_iInitRes;

	bool m_bEnd;
	bool m_bWriteEnd;

	uint32 m_uiType;
	bz_stream strm;

	std::vector<char> m_DataInStream;
	std::vector<char> m_DataOutStream;

	char buff[10*1024];
};


BZ2Worker::BZ2Worker(uint32 type)
{
	m_pData = new BZ2WorkerData(type);
}

BZ2Worker::~BZ2Worker()
{
	delete m_pData;
}

bool BZ2Worker::isInit(int &res)
{
	return m_pData->isInit(res);
}

void BZ2Worker::read(char* buff, size_t &size)
{
	m_pData->read(buff, size);
}

void BZ2Worker::write(const char* buff, size_t size, bool end)
{
	m_pData->write(buff, size, end);
}

void BZ2Worker::doWork()
{
	m_pData->doWork();
}

uint32 BZ2Worker::getReadSize()
{
	return m_pData->getReadSize();
}

int32 BZ2Worker::getLastStatus()
{
	return m_pData->getLastStatus();
}

void BZ2Worker::writeCB(const char* buff, size_t size, UTIL::CB::CallbackI* callback)
{
	m_pData->writeCB(buff, size, callback);
}

void BZ2Worker::endCB(UTIL::CB::CallbackI* callback)
{
	m_pData->endCB(callback);
}

}
}
