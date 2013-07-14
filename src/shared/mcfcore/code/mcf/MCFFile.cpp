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
#include "MCFFile.h"

#include "BZip2.h"

#include <iostream>
#include <fstream>

#include "XMLMacros.h"
#include "Courgette.h"

#include "util/MD5Progressive.h"

#define DEFAULT_BLOCKSIZE (512 * 1024)

#include "XMLSaveAndCompress.h"

#ifdef NIX
#include <ctype.h>
#endif

namespace MCFCore
{

const char* g_vExcludeFileList[] = 
{
	"Thumbs.db",
	NULL
};

//! This is a list of folders not to include when making a MCF. Last item must be NULL
const char* g_vExcludeDirList[] =
{
	".",
	"..",
	".svn",
	".git",
	".hg",
	NULL
};

//! This is a list of file extentions that shouldnt be compressed inside the MCF. Last item must be NULL
const char* g_vNoCompressList[] =
{
	"zip",
	"rar",
	"bz2",
	"tar",
	"gzip",
	"iso",
	"pk3",
	NULL
};

void ReplaceAllStr(std::string searchStr, std::string replaceStr, std::string &str)
{
	size_t pos = 0;

	do
	{
		pos = str.find(searchStr, pos);
		
		if (pos == std::string::npos)
			break;

		str.replace(pos, searchStr.size(), replaceStr);
		pos += searchStr.size();
	}
	while (true);
}

MCFFileI::~MCFFileI()
{
}

MCFFile::MCFFile()
{
	m_iHash = 0;
	m_iSize = 0;
	m_iCSize = 0;
	m_iTimeStamp = 0;
	m_llOffset = 0;

	m_llDiffOffset = 0;
	m_llDiffSize = 0;

	m_iBlockSize = DEFAULT_BLOCKSIZE;
	m_uiFlags = DEFAULT_FLAGS;
}

MCFFile::MCFFile(MCFFile* tMCFFile)
{
	setName(tMCFFile->getName());
	setPath(tMCFFile->getPath());
	setDir(tMCFFile->getDir());
	setCsum(tMCFFile->getCsum());
	setCCsum(tMCFFile->getCCsum());

	m_iSize = tMCFFile->getSize();
	m_iCSize = tMCFFile->getCSize();
	m_iHash = tMCFFile->getHash();
	m_iTimeStamp = tMCFFile->getTimeStamp();
	m_llOffset = 0;

	m_iBlockSize = DEFAULT_BLOCKSIZE;
	m_uiFlags = tMCFFile->getFlags();

	m_vCRCList.clear();

	for (size_t x=0; x<tMCFFile->getCRCCount(); x++)
		m_vCRCList.push_back(tMCFFile->getCRC(x));
}

MCFFile::~MCFFile()
{
}

bool MCFFile::shouldCompress()
{
	if (m_szName == "")
		return false;

	UTIL::FS::File file(m_szName);
	return !UTIL::MISC::matchList(file.getFileExt().c_str(), g_vNoCompressList);
}

const char* MCFFile::getName()
{
	return m_szName.c_str();
}

const char* MCFFile::getPath()
{
	return m_szPath.c_str();
}

const char* MCFFile::getDir()
{
	return m_szDir.c_str();
}

const char* MCFFile::getCsum()
{
	return m_szCsum.c_str();
}

const char* MCFFile::getCCsum()
{
	return m_szCCsum.c_str();
}

void MCFFile::setName(const char* var)
{
	m_szName = var;
	saveHash();

	return;
}

void MCFFile::setDir(const char* var)
{
	m_szDir = var;
}

void MCFFile::setPath(const char* var)
{
	m_szPath = var;

#ifdef NIX
	ReplaceAllStr("/", "\\", m_szPath);
#endif

	saveHash();
}

void MCFFile::setCsum(const char* var)
{
	m_szCsum = var;
}

void MCFFile::setCCsum(const char* var)
{
	m_szCCsum = var;
}

//must delete this other wise it will leak
std::string MCFFile::getFullPath()
{
	if (m_szName == "")
		Warning("MCF: Name for MCF item was Null!\n");

	if (m_szPath == "")
		Warning(gcString("MCF: Path for {0} was Null!\n", m_szName));

	//dont just use names by them selfs other wise it will install to the desura root folder.
	if (m_szDir == "" && m_szPath == "")
		return "";

	UTIL::FS::Path path(m_szDir, m_szName, false);
	path += m_szPath;

	return path.getFullPath();
}

//use the checksum over timestamp as its more reliable.
//If zero file size we must jump out as they might not have a checksum
uint8 MCFFile::isEquals( MCFFile * file)
{
	if (!file)
		return CMP_NOTEQUAL;

		std::string tpath = UTIL::FS::Path(m_szPath, m_szName, false).getFullPath();
		std::string fpath = UTIL::FS::Path(gcString(file->getPath()), gcString(file->getName()), false).getFullPath();

#ifdef WIN32
		std::transform(tpath.begin(), tpath.end(), tpath.begin(), ::tolower);
		std::transform(fpath.begin(), fpath.end(), fpath.begin(), ::tolower);
#endif

	if (tpath != fpath)
	{
		return CMP_NOTEQUAL;
	}
	else if (this->getSize() == 0 && (this->getFlags()&FLAG_ZEROSIZE) && file->getSize() == 0 && (file->getFlags()&FLAG_ZEROSIZE))
	{
		return CMP_SAME;
	}
	else if (this->getCsum() && file->getCsum() && strcmp(this->getCsum(), file->getCsum()) == 0)
	{
		return CMP_SAME;
	}
	else if (file->getTimeStamp() == this->getTimeStamp())
	{
		return CMP_SAME;
	}
	else if (file->getTimeStamp() > this->getTimeStamp())
	{
		return CMP_NEWER;
	}
	else if (file->getTimeStamp() < this->getTimeStamp())
	{
		return CMP_OLDER;
	}
	else
	{
		return CMP_NOTEQUAL;
	}
}

void MCFFile::saveHash()
{
	if (m_szPath != "" && m_szName != "")
	{
		std::string path = UTIL::FS::Path(m_szPath, m_szName, false).getFullPath();

#ifdef WIN32
		std::transform(path.begin(), path.end(), path.begin(), ::tolower);
#endif

		m_iHash = UTIL::MISC::FNVHash64(path);
	}
}

void MCFFile::printAll()
{
	Debug(gcString("MCF: Name {0}\n", m_szName));
	Debug(gcString("\tPath: {0}\n", m_szPath));
	Debug(gcString("\tTimeStamp {0}\n", m_iTimeStamp));
	Debug(gcString("\n"));
}


uint64 MCFFile::getCurSize()
{
	if (isCompressed())
		return m_iCSize;
	else
		return m_iSize;
}

void MCFFile::loadXmlData(tinyxml2::XMLElement *xmlNode)
{
	XML::GetChild("name", this, &MCFFile::setName, xmlNode);
	XML::GetChild("path", this, &MCFFile::setPath, xmlNode);
	XML::GetChild("nom_csum", this, &MCFFile::setCsum, xmlNode);
	XML::GetChild("com_csum", this, &MCFFile::setCCsum, xmlNode);

	XML::GetChild("size", m_iSize, xmlNode);
	XML::GetChild("csize", m_iCSize, xmlNode);
	XML::GetChild("flags", m_uiFlags, xmlNode);

	XML::GetChild("offset", m_llOffset, xmlNode);
	XML::GetChild("tstamp", m_iTimeStamp, xmlNode);

	tinyxml2::XMLElement* diff = xmlNode->FirstChildElement("diff");

	if (hasDiff() && diff)
	{
		XML::GetChild("offset", m_llDiffOffset, diff);
		XML::GetChild("size", m_llDiffSize, diff);
		XML::GetChild("file_csum", m_szDiffOrgFileHash, diff);
		XML::GetChild("csum", m_szDiffHash, diff);
	}
	else
	{
		delFlag(FLAG_HASDIFF);
	}

	tinyxml2::XMLElement *crcNode = xmlNode->FirstChildElement("crc");

	if (crcNode && crcNode->ToElement())
	{
		tinyxml2::XMLElement* crcEl = crcNode->ToElement();

		const char* bs = crcEl->Attribute("blocksize");

		if (bs)
			m_iBlockSize = atoi(bs);

		gcString baseBuff(crcEl->GetText());

		size_t outSize = 0;
		const unsigned char* buff = UTIL::STRING::base64_decode(baseBuff, outSize);

		if (outSize % 4 != 0)
		{
			Warning("Crc % 4 != 0");
			outSize -= outSize%4;
		}

		for (size_t x=0; x<outSize; x+=4)
		{
			uint32 crc = (((uint32)buff[x])<<24) + (((uint32)buff[x+1])<<16) + (((uint32)buff[x+2])<<8) + (((uint32)buff[x+3])<<0);
			m_vCRCList.push_back(crc);
		}

		safe_delete(buff);
	}
}

void SaveToSac(XMLSaveAndCompress *sac, uint32 t)
{
	gcString res("{0}", t);
	sac->save(res.c_str(), res.size());
}

void SaveToSac(XMLSaveAndCompress *sac, uint64 t)
{
	gcString res("{0}", t);
	sac->save(res.c_str(), res.size());
}

void SaveToSac(XMLSaveAndCompress *sac, const std::string &str)
{
	std::string res = str;

	ReplaceAllStr("&", "&amp;", res);
	ReplaceAllStr("<", "&lt;", res);
	ReplaceAllStr(">", "&gt;", res);
	ReplaceAllStr("\"", "&quot;", res);
	ReplaceAllStr("\'", "&apos;", res);

	sac->save(res.c_str(), res.size());
}

void MCFFile::genXml(XMLSaveAndCompress *sac)
{
	sac->save("<name>", 6);
	SaveToSac(sac, m_szName);
	sac->save("</name>", 7);
	
	sac->save("<path>", 6);
	SaveToSac(sac, m_szPath);
	sac->save("</path>", 7);
	
	sac->save("<flags>", 7);
	SaveToSac(sac, (uint32)m_uiFlags);
	sac->save("</flags>", 8);

	if (m_iTimeStamp)
	{
		sac->save("<tstamp>", 8);
		SaveToSac(sac, m_iTimeStamp);
		sac->save("</tstamp>", 9);
	}

	if (isZeroSize())
		return;

	sac->save("<offset>", 8);
	SaveToSac(sac, m_llOffset);
	sac->save("</offset>", 9);

	sac->save("<size>", 6);
	SaveToSac(sac, m_iSize);
	sac->save("</size>", 7);

	if (isCompressed())
	{
		sac->save("<csize>", 7);
		SaveToSac(sac, m_iCSize);
		sac->save("</csize>", 8);
	}

	sac->save("<nom_csum>", 10);
	SaveToSac(sac, m_szCsum);
	sac->save("</nom_csum>", 11);

	if (isCompressed())
	{
		sac->save("<com_csum>", 10);
		SaveToSac(sac, m_szCCsum);
		sac->save("</com_csum>", 11);
	}

	if (hasDiff())
	{
		sac->save("<diff>", 6);
			sac->save("<offset>", 8);
			SaveToSac(sac, m_llDiffOffset);
			sac->save("</offset>", 9);

			sac->save("<size>", 6);
			SaveToSac(sac, m_llDiffSize);
			sac->save("</size>", 7);

			sac->save("<file_csum>", 11);
			SaveToSac(sac, m_szDiffOrgFileHash);
			sac->save("</file_csum>", 12);

			sac->save("<csum>", 6);
			SaveToSac(sac, m_szDiffHash);
			sac->save("</csum>", 7);
		sac->save("</diff>", 7);
	}

	if (!m_vCRCList.empty())
	{
		size_t size = m_vCRCList.size()*4;
		UTIL::MISC::Buffer data(size, true);

		char* cur = data;

		for (size_t x=0; x<m_vCRCList.size(); x++)
		{
			cur[0] = (((uint32)m_vCRCList[x])>>24)&0xFF;
			cur[1] = (((uint32)m_vCRCList[x])>>16)&0xFF;
			cur[2] = (((uint32)m_vCRCList[x])>>8)&0xFF;
			cur[3] = (((uint32)m_vCRCList[x])>>0)&0xFF;

			cur += 4;
		}

		std::string crc = UTIL::STRING::base64_encode(data, size);

		sac->save("<crc blocksize=\"", 16);
		SaveToSac(sac, m_iBlockSize);
		sac->save("\">", 2);
		SaveToSac(sac, crc);
		sac->save("</crc>", 6);
	}
}

void MCFFile::copyBorkedSettings(MCFFile* tMCFFile)
{
	setCCsum(tMCFFile->getCCsum());
	m_vCRCList.clear();

	for (size_t x=0; x<tMCFFile->getCRCCount(); x++)
		m_vCRCList.push_back(tMCFFile->getCRC(x));
}

void MCFFile::copySettings(MCFFile* tMCFFile)
{
	gcString tn(this->getName());
	gcString on(tMCFFile->getName());

	std::transform(tn.begin(), tn.end(), tn.begin(), ::tolower);
	std::transform(on.begin(), on.end(), on.begin(), ::tolower);

	if (tn != on)
		setName(tMCFFile->getName());

	gcString tp(this->getPath());
	gcString op(tMCFFile->getPath());

	std::transform(tp.begin(), tp.end(), tp.begin(), ::tolower);
	std::transform(op.begin(), op.end(), op.begin(), ::tolower);

	if (tp != op)
		setPath(tMCFFile->getPath());

	setDir(tMCFFile->getDir());
	setCsum(tMCFFile->getCsum());
	setCCsum(tMCFFile->getCCsum());

	m_uiFlags = tMCFFile->getFlags() | (m_uiFlags&FLAG_XECUTABLE);
	m_iSize = tMCFFile->getSize();
	m_iCSize = tMCFFile->getCSize();
	m_iHash = tMCFFile->getHash();
	m_iTimeStamp = tMCFFile->getTimeStamp();
	m_llOffset = 0;

	m_vCRCList.clear();

	for (size_t x=0; x<tMCFFile->getCRCCount(); x++)
		m_vCRCList.push_back(tMCFFile->getCRC(x));
}

void MCFFile::verifyFile(bool useDiffs)
{
	UTIL::FS::Path path(getFullPath(), "", true);

	if (HasAllFlags(getFlags(), FLAG_ZEROSIZE))
	{
		if (UTIL::FS::isValidFile(path) && UTIL::FS::getFileSize(path) == 0)
			addFlag(FLAG_COMPLETE);
		else
			delFlag(FLAG_COMPLETE);
	}
	else
	{
		std::string temp = UTIL::MISC::hashFile(path.getFullPath());
		verify(temp.c_str(), false, useDiffs);
	}
}

void MCFFile::verifyMcf(UTIL::FS::FileHandle& file, const volatile bool &stop)
{
	file.seek(getOffSet());

	MD5Progressive md5;

	file.read(getCurSize(), [&md5, &stop](const unsigned char* data, size_t size) -> bool
	{
		if (!stop && data && size > 0)
			md5.update((const char*)data, size);

		return stop;
	});

	if (stop)
		throw gcException(ERR_USERCANCELED);

	std::string temp = md5.finish();
	verify(temp.c_str(), true);
}

void MCFFile::verify(const char* hash, bool compressed, bool useDiffs)
{
	delFlag(FLAG_COMPLETE);

	if (!hash)
		return;

	if (getFlags() & FLAG_ZEROSIZE)
	{
		addFlag(FLAG_COMPLETE);
	}
	else if (compressed && (getFlags() & FLAG_COMPRESSED))
	{
		if (m_szCCsum == hash)
			addFlag(FLAG_COMPLETE);
	}
	else
	{
		if (m_szCsum == hash)
			addFlag(FLAG_COMPLETE);
		else if (useDiffs && m_llDiffSize != 0 && m_szDiffOrgFileHash == hash)
			addFlag(FLAG_CANUSEDIFF);
	}
}

void MCFFile::delFile()
{
	UTIL::FS::delFile(UTIL::FS::PathWithFile(getFullPath()));
}

bool MCFFile::hashCheckFile()
{
	return hashCheckFile(NULL);
}

bool MCFFile::hashCheckFile(std::string* retHash)
{
	if (m_szCsum == "")
		return false;

	UTIL::FS::Path path(m_szDir, m_szName, false);
	path += m_szPath;

	try
	{
		UTIL::FS::FileHandle fh(path.getFullPath().c_str(), UTIL::FS::FILE_READ);

		std::string hash = UTIL::MISC::hashFile(fh.getHandle(), getSize());

		if (retHash)
			*retHash = hash;

		return (hash == m_szCsum);
	}
	catch (...)
	{
		return false;
	}
}

void MCFFile::hashFile()
{
	UTIL::FS::Path path(m_szDir, m_szName, false);
	path += m_szPath;

	std::string hash = UTIL::MISC::hashFile(path.getFullPath());
	setCsum(hash.c_str());
}

void MCFFile::generateMD5(UTIL::FS::FileHandle& file)
{
	file.seek(getOffSet());
	std::string temp = UTIL::MISC::hashFile(file.getHandle(), getCurSize());

	if (isCompressed())
		setCCsum(temp.c_str());
	else
		setCsum(temp.c_str());
}

void MCFFile::generateCRC(UTIL::FS::FileHandle& file)
{
	m_vCRCList.clear();

	if (isZeroSize() || !isSaved())
		return;

	if (m_iBlockSize == 0)
		m_iBlockSize = DEFAULT_BLOCKSIZE;

	file.seek(getOffSet());

	UTIL::MISC::Buffer buff(m_iBlockSize);
	uint64 done = 0;

	while (done < this->getCurSize())
	{
		uint32 todo = m_iBlockSize;
		if (todo > (getCurSize() - done))
			todo = (uint32)(getCurSize() - done);

		file.read(buff, todo);
		uint32 crc = UTIL::MISC::CRC32((unsigned char*)buff.data(), todo);
		m_vCRCList.push_back(crc);

		done += todo;
	}
}

void MCFFile::resetCRC()
{
	m_vCRCList.clear();

	if (isZeroSize() || !isSaved())
		return;

	if (m_iBlockSize == 0)
		m_iBlockSize = DEFAULT_BLOCKSIZE;
}

void MCFFile::setCRC(std::vector<uint32> &vCRCList)
{
	m_vCRCList.clear();

	for (size_t x=0; x<vCRCList.size(); x++)
	{
		m_vCRCList.push_back(vCRCList[x]);
	}
}

bool MCFFile::crcCheck(UTIL::FS::FileHandle& file)
{
	uint32 done = 0;
	uint16 x=0;

	while (done < getCurSize())
	{
		if (!crcCheck(x, file))
			return false;

		done += getBlockSize();
		x++;
	}

	return true;
}

bool MCFFile::crcCheck(uint16 blockId, UTIL::FS::FileHandle& file)
{
	if (!m_vCRCList.empty() && blockId >= m_vCRCList.size())
	{
		//corupt MCF header. Cant do much about it
		return false;
	}

	uint32 offset = blockId*getBlockSize();
	file.seek(getOffSet()+offset);


	char* buff = new char[getBlockSize()];

	uint32 todo = getBlockSize();
	if (todo > (getCurSize() - offset))
		todo = (uint32)(getCurSize() - offset);

	try
	{
		file.read(buff, todo);
	}
	catch (gcException &)
	{
		safe_delete(buff);
		return false;
	}

	bool res = false;

	if ( m_vCRCList.empty() )
	{
		res = legacyBlockCheck(buff, todo);
	}
	else
	{
		uint32 crc = UTIL::MISC::CRC32((unsigned char*)buff, todo);
		res = (crc == m_vCRCList[blockId]);
	}

	safe_delete(buff);

	return res;
}

//make sure it has some valid data
bool MCFFile::legacyBlockCheck(const char* buff, uint32 size)
{
	if (!buff)
		return false;

	size_t count = 0;
	size_t quadSize = size/4;
	int* quadBuff = (int*)buff;

	for (size_t x=0; x<quadSize; x++)
	{
		if (quadBuff[x] == 0)
			count++;
		else
			count=0;

		if (count>15 || count >= quadSize)
			return false;
	}

	return true;
}

void MCFFile::setSize(uint64 size)
{
	m_iSize = size;
}

void MCFFile::setCSize(uint64 size)
{
	m_iCSize = size;
}

void MCFFile::setTimeStamp(uint64 time)
{
	m_iTimeStamp = time;
}

void MCFFile::setOffSet(uint64 offset)
{ 
	m_llOffset = offset; 
}

void MCFFile::addFlag(uint16 flag)
{
	m_uiFlags |= flag;
}

void MCFFile::delFlag(uint16 flag)
{
	m_uiFlags &= ~flag;
}
	
bool MCFFile::hasStartedDL()
{
	return HasAnyFlags(m_uiFlags, FLAG_STARTEDDL|FLAG_COMPLETE);
}

bool MCFFile::hasDiff()
{
	return HasAnyFlags(m_uiFlags, FLAG_HASDIFF);
}

uint64 MCFFile::getDiffSize()
{
	return m_llDiffSize;
}

uint64 MCFFile::getDiffOffSet()
{
	return m_llDiffOffset;
}

const char* MCFFile::getDiffHash()
{
	return m_szDiffHash.c_str();
}

const char* MCFFile::getDiffOrgFileHash()
{
	return m_szDiffOrgFileHash.c_str();
}

void MCFFile::setDiffOffset(uint64 offset)
{
	m_llDiffOffset = offset;
}

void MCFFile::setDiffInfo(const char* orgHash, const char* hash, uint64 size)
{
	m_llDiffSize = size;
	m_szDiffOrgFileHash = orgHash;
	m_szDiffHash = hash;
}

void MCFFile::clearDiff()
{
	delFlag(MCFFileI::FLAG_CANUSEDIFF|MCFFileI::FLAG_HASDIFF);
	m_llDiffSize = 0;
	m_llDiffOffset = 0;
	m_szDiffOrgFileHash = "";
	m_szDiffHash = "";
}

}
