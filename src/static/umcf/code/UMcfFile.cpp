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


#pragma warning( disable : 4627 )

#include "Common.h"
#include "umcf/UMcfFile.h"
#include <iostream>
#include <fstream>

#include "XMLMacros.h"
#include "UMcfFile_utils.h"

#ifdef NIX
#include <errno.h>
#include <limits.h>
#endif


UMcfFile::UMcfFile()
{
	m_uiFlags = 0;
	m_uiHash = 0;
	m_ullSize = 0;
	m_ullCSize = 0;
	m_ullTimeStamp = 0;
}

UMcfFile::~UMcfFile()
{
}

const wchar_t* UMcfFile::getName()
{
	return m_szName.c_str();
}

const wchar_t* UMcfFile::getPath()
{
	return m_szPath.c_str();
}

const char* UMcfFile::getCsum()
{
	return m_szCsum.c_str();
}

const char* UMcfFile::getCCsum()
{
	return m_szCCsum.c_str();
}

void UMcfFile::setName(const wchar_t* var)
{
	if (!var)
		m_szName = L"";
	else 
		m_szName = var;
}

void UMcfFile::setPath(const wchar_t* var)
{
	if (!var)
		m_szPath = L"";
	else 
		m_szPath = var;
}

void UMcfFile::setCsum(const char* var)
{
	if (!var)
		m_szCsum = "";
	else 
		m_szCsum = var;
}

void UMcfFile::setCCsum(const char* var)
{
	if (!var)
		m_szCCsum = "";
	else 
		m_szCCsum = var;
}

uint8 UMcfFile::loadXmlData(tinyxml2::XMLElement *xmlNode)
{
	std::string name;
	std::string path;

	XML::GetChild("name", name, xmlNode);
	XML::GetChild("path", path, xmlNode);
	XML::GetChild("nom_csum", m_szCsum, xmlNode);
	XML::GetChild("com_csum", m_szCCsum, xmlNode);

	m_szName = name;
	m_szPath = path;

#ifdef NIX
	std::replace(m_szPath.begin(), m_szPath.end(), '\\', '/');
#endif

	XML::GetChild("size", m_ullSize, xmlNode);
	XML::GetChild("csize", m_ullCSize, xmlNode);
	XML::GetChild("flags", m_uiFlags, xmlNode);

	XML::GetChild("offset", m_ullOffset, xmlNode);
	XML::GetChild("tstamp", m_ullTimeStamp, xmlNode);

	return MCFF_OK;
}

void UMcfFile::genXml(tinyxml2::XMLElement *element, tinyxml2::XMLDocument& doc)
{
#ifdef NIX
	std::wstring copy(m_szPath);
	std::replace(copy.begin(), copy.end(), '/', '\\');
#endif

	XML::WriteChild("name", gcString(m_szName), element, doc);
#ifdef NIX
	XML::WriteChild("path", gcString(copy), element, doc);
#else
	XML::WriteChild("path", gcString(m_szPath), element, doc);
#endif
	XML::WriteChild("nom_csum", m_szCsum, element, doc);
	XML::WriteChild("com_csum", m_szCCsum, element, doc);

	XML::WriteChild("size", m_ullSize, element, doc);
	XML::WriteChild("csize", m_ullCSize, element, doc);
	XML::WriteChild("flags", m_uiFlags, element, doc);

	XML::WriteChild("offset", m_ullOffset, element, doc);
	XML::WriteChild("tstamp", m_ullTimeStamp, element, doc);
}


bool UMcfFile::checkFile(const wchar_t* dir)
{
#ifdef NIX
	gcString path("{0}/{1}/{2}", dir, m_szPath, m_szName);

	struct stat stFileInfo;
	int intStat = stat(path.c_str(), &stFileInfo);

#ifdef DEBUG
	char buffer[PATH_MAX];
	snprintf(buffer, PATH_MAX, "%s (%s)", __func__, path.c_str());
	ERROR_OUTPUT(buffer);
#endif

#else
	gcWString path("{0}\\{1}\\{2}", dir, m_szPath, m_szName);
	struct _stat64i32 stFileInfo;

	// Attempt to get the file attributes
	int intStat = _wstat(path.c_str(), &stFileInfo);
#endif

	return (intStat == 0);
}

bool UMcfFile::isSame(UMcfFile* file)
{
	if (!file || !this->getPath() || !file->getPath() || !this->getName() || !file->getName())
		return false;

	return (Safe::wcsicmp(getName(), file->getName()) == 0 && Safe::wcsicmp(getPath(), file->getPath()) == 0);
}

bool UMcfFile::verifyFile(FILEHANDLE hFile, uint64 baseOffset)
{
#ifdef WIN32
	LARGE_INTEGER mov;
	mov.QuadPart = baseOffset + getOffSet();

	if (SetFilePointerEx(hFile, mov, NULL, FILE_BEGIN) == 0)
		return false;
#else
	fseek(hFile, baseOffset + getOffSet(), SEEK_SET);
#endif

	std::string temp;
	
	if (getFlags() & UMCFF_FLAG_ZEROSIZE)
		return true;

	if (getFlags() & UMCFF_FLAG_COMPRESSED)
		temp = UTIL::MISC::hashFile((FHANDLE)hFile, m_ullCSize);
	else
		temp = UTIL::MISC::hashFile((FHANDLE)hFile, m_ullSize);

	return verify(temp.c_str());
}

bool UMcfFile::verify(const char* hash)
{
	if (!hash)
		return false;

	if (getFlags() & UMCFF_FLAG_COMPRESSED)
		return m_szCCsum == std::string(hash);

	return m_szCsum == std::string(hash);
}



bool checkFile(const char* dir)
{
	return true;
}

void RecMakeFolder(UTIL::FS::Path path)
{
	std::string subPath;

	for (size_t x=0; x<path.getFolderCount(); x++)
	{
		subPath += path.getFolder(x);
		subPath += path.GetDirSeperator();

#ifdef WIN32
		//skip drive letters
		if (x == 0 && subPath.size() == 3 && subPath[1] == ':')
			continue;
#endif

		if (!CreateDir(gcWString(subPath).c_str()))
			throw gcException(ERR_BADPATH, GetLastError(), gcString("Failed to make path: {0}", subPath));
	}
}


uint8 UMcfFile::decompressAndSave(FILEHANDLE hSrc, FILEHANDLE hSink, uint64 offset, DelegateI<ProgressCB> *del)
{
	UTIL::MISC::Buffer buff(BUFFSIZE, true);
	const size_t buffsize = BUFFSIZE;

	if (!FileSeek(hSrc, offset + m_ullOffset))
		return MCFF_ERR_INVALIDFILE;

	uint64 leftToDo = m_ullCSize;
	bool end = false;

	UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_DECOMPRESS);

	do
	{
		size_t curSize = BUFFSIZE;

		if (curSize > leftToDo)
		{
			end = true;
			curSize = (size_t)leftToDo;
		}

		if (curSize == 0)
			return MCFF_ERR_PARTREAD;

		if (!FileRead(hSrc, curSize, buff))
			return MCFF_ERR_FAILEDREAD;

		leftToDo -= curSize;

		worker.write(buff, curSize, end);
		worker.doWork();

		size_t b = 0;

		do
		{
			b = buffsize;
			worker.read(buff.data(), b);

			if (!FileWrite(hSink, b, buff))
				return MCFF_ERR_FAILEDWRITE;
		}
		while (b > 0);

		ProgressCB p(m_ullCSize - leftToDo);

		if (del)
			del->operator()(p);

		if (p.cancel)
			return MCFF_ERR_CANCELED;

	}
	while (!end);

	return MCFF_OK;
}

uint8 UMcfFile::saveData(FILEHANDLE hSrc, FILEHANDLE hSink, uint64 offset, DelegateI<ProgressCB> *del)
{
	UTIL::MISC::Buffer buff(BUFFSIZE, true);
	const size_t buffsize = BUFFSIZE;

	if (!FileSeek(hSrc, offset + m_ullOffset))
		return MCFF_ERR_INVALIDFILE;

	uint64 done = 0;

	do
	{
		size_t curSize = buffsize;

		if ((m_ullSize-done) < buffsize)
			curSize = (uint32)(m_ullSize-done);

		if (!FileRead(hSrc, curSize, buff))
			return MCFF_ERR_FAILEDREAD;

		if (!FileWrite(hSink, curSize, buff))
			return MCFF_ERR_FAILEDWRITE;

		done += curSize;
		ProgressCB p(done);

		if (del)
			del->operator()(p);

		if (p.cancel)
			return MCFF_ERR_CANCELED;
	}
	while (done < m_ullSize);

	return MCFF_OK;
}

void UMcfFile::remove(const wchar_t* dir)
{

#ifdef WIN32
	gcWString path("{0}\\{1}\\{2}", dir, getPath(), getName());
	FileDelete(path.c_str());
#else
	gcString path("{0}/{1}/{2}", dir, getPath(), getName());
	::remove(path.c_str());
#endif
}

uint8 UMcfFile::readMCFAndSave(FILEHANDLE hFile, const wchar_t* dir, uint64 offset, DelegateI<ProgressCB> *del)
{
	if (!IsValidFileHandle(hFile))
		return MCFF_ERR_INVALIDHANDLE;

	if (!dir)
		return MCFF_ERR_INVALIDPATH;

	UTIL::FS::Path path(gcString(dir), gcString(m_szName), false);
	path += gcString(m_szPath);

	RecMakeFolder(path);

	FILEHANDLE hSaveFile;

	std::string fullPath = path.getFullPath();

	if (!FileOpen(hSaveFile, gcWString(fullPath).c_str(), FILE_WRITE))
		return MCFF_ERR_INVALIDHANDLE;

	if (getFlags() & UMCFF_FLAG_ZEROSIZE)
	{
		FileClose(hSaveFile);
		return MCFF_OK;
	}

	uint8 res = 0;

	if (getFlags() & UMCFF_FLAG_COMPRESSED)
		res = decompressAndSave(hFile, hSaveFile, offset, del);
	else
		res = saveData(hFile, hSaveFile, offset, del);

	if (del)
		del->destroy();

	FileClose(hSaveFile);

#ifdef NIX
	struct stat s;

	if (HasAnyFlags(getFlags(), UMCFF_FLAG_XECUTABLE) && stat(fullPath.c_str(), &s) == 0)
		chmod(fullPath.c_str(), (s.st_mode|S_IXUSR|S_IRWXG|S_IRWXO));
#endif	

	if (m_szCsum.size() == 0)
	{
		printf("File has no md5 hash: %ls\n", m_szName.c_str());
		return MCFF_ERR_BADHASH;
	}

	std::string fileHash = UTIL::MISC::hashFile(path.getFullPath().c_str());
	if (fileHash != m_szCsum)
	{
		printf("Hash failed on file: %ls\n", m_szName.c_str());
		return MCFF_ERR_BADHASH;
	}
	
	return res;
}
