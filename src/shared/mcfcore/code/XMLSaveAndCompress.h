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

#ifndef DESURA_XMLSAVEANDCOMPRESS_H
#define DESURA_XMLSAVEANDCOMPRESS_H
#ifdef _WIN32
#pragma once
#endif

class XMLSaveAndCompress
{
public:
	XMLSaveAndCompress(UTIL::FS::FileHandle *fh, bool compress = false);
	~XMLSaveAndCompress();

	void finish();
	void save(const char* str, size_t size);

	uint64 getTotalSize();

protected:
	void write(char* buff, size_t size, bool end = false);

private:
	bool m_bCompress;

	UTIL::MISC::BZ2Worker m_BZ2Worker;
	UTIL::FS::FileHandle *m_fhFile;

	char* m_szWriteBuf;
	char* m_szReadBuf;

	uint32 m_uiLastWritePos;

	uint64 m_uiRawSize;
	uint64 m_uiTotalSize;
};


#endif //DESURA_XMLSAVEANDCOMPRESS_H
