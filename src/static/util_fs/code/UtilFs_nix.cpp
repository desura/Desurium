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

#include <wordexp.h>
#include <string>

#include "Common.h"
#include "util/UtilFs.h"


namespace UTIL
{
namespace FS
{

std::string expandPath(const char* file)
{
	if (!file)
		return "";
	
	std::string f;
	size_t size = strlen(file);
	
	f.reserve(size);
	
	for (size_t x=0; x<size; x++)
	{
		if (file[x] == ' ')
			f.push_back('\\');
			
		f.push_back(file[x]);
	}
	
	wordexp_t exp_result;
	memset(&exp_result, 0, sizeof(wordexp_t));
	
	int res = wordexp(f.c_str(), &exp_result, 0);
	
	if (res != 0)
		return "";
	
	std::string r;
	
	if (exp_result.we_wordv[0])
		r = exp_result.we_wordv[0];
	
	wordfree(&exp_result);
	
	return r;
}

FileHandle::FileHandle(const FileHandle& handle)
{
	m_hFileHandle = fdopen(dup(fileno(handle.getHandle())), handle.getMode());
	m_bIsOpen = handle.isOpen();
}

FileHandle& FileHandle::operator=(const FileHandle& handle)
{
	if (this != &handle)
	{
		m_hFileHandle = fdopen(dup(fileno(handle.getHandle())), handle.getMode());
		m_bIsOpen = handle.isOpen();
	}
	
	return *this;
}

void FileHandle::open(const char* fileName, FILE_MODE mode, uint64 offset)
{
	if (m_bIsOpen)
		close();
		
	if (!fileName)
		throw gcException(ERR_BADPATH, "Cant open file with null path");
		
	std::string fullFile = expandPath(fileName);

	if (fullFile == "")
		fullFile = fileName;

#ifdef DEBUG
	m_szFileName = fullFile;
#endif

	m_uiOffset = offset;

	FILE* fh = NULL;

	switch (mode)
	{
	case FILE_READ:
		fh = fopen64(fullFile.c_str(), "rb");
		m_szMode = "rb";
		break;

	case FILE_WRITE:
		fh = fopen64(fullFile.c_str(), "wb");
		m_szMode = "wb";
		break;

	case FILE_APPEND:
		fh = fopen64(fullFile.c_str(), "rb+");
		m_szMode = "rb+";

		if (fh)
			fseek(fh, 0, SEEK_END);
		else
			fh = fopen64(fullFile.c_str(), "wb");
			
		break;

	default:
		throw gcException(ERR_INVALID, "The mode was invalid");
		break;
	}

	if (!fh)
	{
		printf("Error opening %s as %d: %d\n", fullFile.c_str(), mode, errno);
		throw gcException(ERR_INVALIDFILE, gcString("Couldnt open the file [{0}] in mode {1}", fullFile.c_str(), mode));
	}
	
	m_hFileHandle = fh;
	m_bIsOpen = true;
}

void FileHandle::close()
{
	if (!m_bIsOpen)
		return;

	if (m_hFileHandle)
		fclose(m_hFileHandle);

	m_hFileHandle = NULL;
	m_bIsOpen = false;
}

void FileHandle::read(char* buff, uint32 size)
{
	if (!m_bIsOpen || !m_hFileHandle)
		throw gcException(ERR_NULLHANDLE);

	if (!buff)
		throw gcException(ERR_INVALIDDATA);

	uint32 dwRead =  fread(buff, size, 1, m_hFileHandle);

	if (dwRead == 0)
#ifdef DEBUG
		throw gcException(ERR_FAILEDREAD, gcString("Failed to read from file: [{0}]", m_szFileName));
#else
		throw gcException(ERR_FAILEDREAD);
#endif

	if (dwRead != 1)
		throw gcException(ERR_PARTREAD);
}

void FileHandle::write(const char* buff, uint32 size)
{
	if (size == 0)
		return;

	if (!m_bIsOpen || !m_hFileHandle)
		throw gcException(ERR_NULLHANDLE);

	if (!buff)
		throw gcException(ERR_INVALIDDATA);

	uint32 dwWrite = fwrite(buff, size, 1, m_hFileHandle);

	if (dwWrite == 0)
		throw gcException(ERR_FAILEDWRITE);

	if (dwWrite != 1)
		throw gcException(ERR_PARTWRITE);
}

void FileHandle::seek(uint64 pos)
{
	if (!m_bIsOpen || !m_hFileHandle)
		throw gcException(ERR_NULLHANDLE);

	if (fseeko64(m_hFileHandle, m_uiOffset+pos, SEEK_SET) != 0)
		throw gcException(ERR_FAILEDSEEK);
}

}
}
