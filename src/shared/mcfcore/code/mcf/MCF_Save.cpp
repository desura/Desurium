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
#include "MCF.h"

#include "BZip2.h"

#include "thread/SFTController.h"
#include "thread/SMTController.h"
#include "thread/WGTController.h"
#include "mcfcore/DownloadProvider.h"

#include "XMLSaveAndCompress.h"

#include <time.h>
#include "boost/date_time/posix_time/conversion.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "thread/MCFServerCon.h"

#define MAX_FRAGMENT_SIZE (20*1024*1024)

namespace bpt = boost::posix_time;


class OutBuffer : public MCFCore::Misc::OutBufferI
{
public:
	OutBuffer(uint32 size)
	{
		m_uiBuffSize = size;
		m_szBuffer = new char[size];
		m_uiTotalSize = 0;
	}

	~OutBuffer()
	{
		safe_delete(m_szBuffer);
	}

	OutBuffer& operator=(OutBuffer& o)
	{
		m_szBuffer = o.m_szBuffer;
		o.m_szBuffer = NULL;

		m_uiBuffSize = o.m_uiBuffSize;
		o.m_uiBuffSize = 0;

		m_uiTotalSize = 0;

		return *this;
	}

	OutBuffer& operator=(uint32 size)
	{
		safe_delete(m_szBuffer);
		m_uiBuffSize = size;
		m_szBuffer = new char[size];
		m_uiTotalSize = 0;

		return *this;
	}

	virtual bool writeData(char* data, uint32 size)
	{
		char* bstart = m_szBuffer + m_uiTotalSize;
		size_t bsize = size;

		if (bsize > m_uiBuffSize-m_uiTotalSize)
			bsize = m_uiBuffSize-m_uiTotalSize;

		if (bsize == 0)
			return true;

		memcpy(bstart, data, bsize);
		m_uiTotalSize += bsize;

		return true;
	}
	
	virtual void reset()
	{
		m_uiTotalSize = 0;
	}

	char* m_szBuffer;
	uint32 m_uiBuffSize;
	uint32 m_uiTotalSize;
};


namespace MCFCore
{


void MCF::dlHeaderFromWeb()
{
	if (m_bStopped)
		return;

	if (m_vProviderList.size() == 0)
		throw gcException(ERR_ZEROFILE);

	gcException lastE;
	bool successful = false;

	OutBuffer out(MCF_HEADERSIZE_V2);
	MCFCore::Misc::MCFServerCon msc;

	for (size_t x=0; x<m_vProviderList.size(); x++)
	{
		try
		{
			msc.disconnect();
			msc.connect(m_vProviderList[x]->getUrl(), m_pFileAuth);

			msc.downloadRange(0, 5, &out); //4 id bytes and 1 version byte

			if (out.m_uiTotalSize != 5)
				throw gcException(ERR_BADHEADER, "Did not get any data from mcf server.");

			const char* data = out.m_szBuffer;

			if ( !(data[0] == 'L' && data[1] == 'M' && data[2] == 'C' && data[3] == 'F') )
				throw gcException(ERR_BADHEADER, "Failed magic check.");

			size_t headerSize = MCF_HEADERSIZE_V1;

			if (data[4] == 0x01)
				headerSize = MCF_HEADERSIZE_V1;
			else if (data[4] == 0x02)
				headerSize = MCF_HEADERSIZE_V2;
			else
				throw gcException(ERR_BADHEADER, "Bad version number");

			out.reset();
			msc.downloadRange(0, headerSize, &out);

			if (out.m_uiTotalSize != headerSize)
				throw gcException(ERR_BADHEADER, "Did not get correct ammount of data from server.");

			MCFCore::MCFHeader webHeader((uint8*)out.m_szBuffer);

			if (!webHeader.isValid())
				throw gcException(ERR_BADHEADER, "Mcf header was not valid.");

			uint32 ths = webHeader.getXmlSize();
			out = ths;

			msc.downloadRange(webHeader.getXmlStart(), webHeader.getXmlSize(), &out);

			if (out.m_uiTotalSize == 0 || out.m_uiTotalSize != webHeader.getXmlSize())
				throw gcException(ERR_WEBDL_FAILED, "Failed to download MCF xml from web (size is ether zero or didnt match header size)");

			data = out.m_szBuffer;

			if (data[0] == 'L' && data[1] == 'M' && data[2] == 'C' && data[3] == 'F')
				throw gcException(ERR_WEBDL_FAILED, "Server failed 4gb seek.");

			setHeader(&webHeader);
			successful = true;
			break;

		}
		catch (gcException &e)
		{
			lastE = e;
			Warning(gcString("Failed to download MCF Header from {1}: {0}\n", e, m_vProviderList[x]->getUrl()));
		}
	}

	if (!successful)
		throw lastE;

	uint32 bz2BuffLen = getHeader()->getXmlSize()*25;
	char* bz2Buff = NULL;

	if ( isCompressed() )
	{
		bz2Buff = new char[bz2BuffLen];
		UTIL::STRING::zeroBuffer(bz2Buff, bz2BuffLen);

		try
		{
			UTIL::BZIP::BZ2DBuff((char*)bz2Buff, &bz2BuffLen, out.m_szBuffer, out.m_uiTotalSize);
			parseXml(bz2Buff, bz2BuffLen);
			safe_delete(bz2Buff);
		}
		catch (gcException &)
		{
			safe_delete(bz2Buff);
			throw;
		}
	}
	else
	{
		parseXml(out.m_szBuffer, out.m_uiTotalSize);
	}

	//we remove the complete flag due to the files not existing in the MCF
	for (size_t x=0; x< m_pFileList.size(); x++)
	{
		m_pFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
	}

	if (m_szFile != "")
		saveMCF_Header();
}

void MCF::dlFilesFromWeb( )
{
	assert(!m_pTHandle);

	if (m_bStopped)
		return;

	if (m_vProviderList.size() == 0)
		throw gcException(ERR_NOFILE);

	bool mcfExists = UTIL::FS::isValidFile(UTIL::FS::PathWithFile(getFile()));

	//save the header first incase we fail
	saveMCF_Header();

	uint16 workerCount = (uint16)m_vProviderList.size();

	if (workerCount > 3)
		workerCount = 3;

	MCFCore::Thread::WGTController *temp = new MCFCore::Thread::WGTController(m_vProviderList, workerCount, this, mcfExists);
	temp->onProgressEvent += delegate(&onProgressEvent);
	temp->onErrorEvent += delegate(&onErrorEvent);
	temp->onProviderEvent += delegate(&onProviderEvent);

	m_mThreadMutex.lock();
	m_pTHandle = temp;
	m_mThreadMutex.unlock();

	if (m_bStopped)
		return;

	try
	{
		m_pTHandle->start();
		m_pTHandle->join();
		safe_delete(m_pTHandle);
	}
	catch (gcException &)
	{
		safe_delete(m_pTHandle);
		throw;
	}

	saveMCF_Header();
}

void MCF::parseFolder(const char *path, bool hashFile, bool reportProgress)
{
	parseFolder(NULL, path);

	if (hashFile)
	{
		MCFCore::Misc::ProgressInfo p;
		p.totalAmmount = m_pFileList.size();

		for (size_t x=0; x<m_pFileList.size(); x++)
		{
			m_pFileList[x]->hashFile();

			if (reportProgress)
			{
				p.doneAmmount = x+1;
				p.percent = (uint8)(p.doneAmmount*100/p.totalAmmount);
				onProgressEvent(p);
			}
		}
	}
}


void MCF::parseFolder(const char *filePath, const char *oPath)
{
	if (m_bStopped)
		return;

	if (!oPath)
		throw gcException(ERR_BADPATH);


	UTIL::FS::Path path(oPath, "", false);

	if (filePath)
		path += filePath;

	std::vector<UTIL::FS::Path> fileList;
	std::vector<UTIL::FS::Path> dirList;


	if (!UTIL::FS::isValidFolder(path))
		throw gcException(ERR_BADPATH, gcString("The file path was invalid [{0}]", path.getFullPath()));

	UTIL::FS::getAllFiles(path, fileList, NULL);
	UTIL::FS::getAllFolders(path, dirList);

	if (fileList.size() == 0 && dirList.size() == 0)
	{
		MCFCore::MCFFile *temp = new MCFCore::MCFFile();

		if (filePath)
			temp->setPath(filePath);

		temp->setName("%%EMPTYFOLDER%%");
		temp->setDir(oPath);
		temp->setSize(0);

		m_pFileList.push_back(temp);
		return;
	}

	for (size_t x=0; x<fileList.size(); x++)
	{
		std::string file = fileList[x].getFile().getFile();

		if (UTIL::MISC::matchList(file.c_str(), MCFCore::g_vExcludeFileList))
			continue;

		if (file.size() > 0 && file[file.size()-1] == ' ')
			throw gcException(ERR_BADPATH, gcString("File [{0}] has a space at the end of its name. This is not valid for MCF archives.", fileList[x].getFullPath()));

		MCFCore::MCFFile *temp = new MCFCore::MCFFile();

		if (filePath)
			temp->setPath(filePath);
		else
			temp->setPath(DIRS_STR);

		temp->setName(file.c_str());
		temp->setDir(oPath);

		temp->setSize((uint32)UTIL::FS::getFileSize(fileList[x]));

		time_t lastWrite = UTIL::FS::lastWriteTime(fileList[x]);

		std::string timeStr = bpt::to_iso_string(bpt::from_time_t(lastWrite));
		temp->setTimeStamp(UTIL::MISC::atoll(timeStr.c_str()));

		

#ifdef NIX
		std::string fullpath = fileList[x].getFullPath();

		struct stat s;
		if ((stat(fullpath.c_str(), &s) == 0) && s.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
			temp->addFlag(MCFFileI::FLAG_XECUTABLE);
#endif

		m_pFileList.push_back( temp );
	}

	for (size_t x=0; x<dirList.size(); x++)
	{
		if (UTIL::MISC::matchList(dirList[x].getLastFolder().c_str(), MCFCore::g_vExcludeDirList))
			continue;

		gcString newPath(dirList[x].getLastFolder().c_str());

		if (filePath)
			newPath = gcString("{0}{1}{2}", filePath, DIRS_STR, dirList[x].getLastFolder());

		parseFolder(newPath.c_str(), oPath);
	}
};





void MCF::saveFiles(const char* path)
{
	assert(!m_pTHandle);

	if (m_bStopped || !path)
		return;

	UTIL::FS::recMakeFolder(UTIL::FS::Path(path, "", false));

	MCFCore::Thread::SFTController *temp = new MCFCore::Thread::SFTController(m_uiWCount, this, path);
	temp->onProgressEvent +=delegate(&onProgressEvent);
	temp->onErrorEvent += delegate(&onErrorEvent);

	m_mThreadMutex.lock();
	m_pTHandle = temp;
	m_mThreadMutex.unlock();

	if (m_bStopped)
		return;

	try
	{
		m_pTHandle->start();
		m_pTHandle->join();
		safe_delete(m_pTHandle);
	}
	catch (gcException &)
	{
		safe_delete(m_pTHandle);
		throw;
	}
}


void MCF::saveMCF_CandSFiles()
{
	if (m_bStopped || m_pTHandle)
		return;

	if (m_pFileList.size() == 0)
		throw gcException(ERR_SAVE_NOFILES);

	if (!m_sHeader)
		throw gcException(ERR_SAVE_NOHEADER);

	MCFCore::Thread::SMTController* temp = new MCFCore::Thread::SMTController(m_uiWCount, this);
	temp->onProgressEvent +=delegate(&onProgressEvent);
	temp->onErrorEvent += delegate(&onErrorEvent);

	m_mThreadMutex.lock();
	m_pTHandle = temp;
	m_mThreadMutex.unlock();

	if (m_bStopped)
		return;

	try
	{
		m_pTHandle->start();
		m_pTHandle->join();
		safe_delete(m_pTHandle);
	}
	catch (gcException &)
	{
		safe_delete(m_pTHandle);
		throw;
	}

#ifdef DEBUG
	uint64 offset = getDLSize() + m_sHeader->getSize();
	uint64 filesize = UTIL::FS::getFileSize(m_szFile);

	if (filesize > offset)
		assert(false);
#endif
}


void MCF::exportMcf(const char* path)
{
	printf("Exporting mcf!!\n");

	if (m_bStopped)
		return;

	if (!path)
		throw gcException(ERR_INVALID, "Path is null");

	MCF dest;

	dest.setFile(path);
	dest.setHeader(this->getHeader());
	dest.getHeader()->updateFileVersion();

	//allways start after the header.
	uint64 lastOffset = dest.getHeader()->getSize();

	UTIL::FS::FileHandle hFileSrc(getFile(), UTIL::FS::FILE_READ);
	UTIL::FS::FileHandle hFileDest(path, UTIL::FS::FILE_WRITE);

	hFileDest.seek(lastOffset);

	//copy all the files from the original MCF
	for (uint32 x=0; x<(uint32)m_pFileList.size(); x++)
	{
		if (m_pFileList[x]->isSaved())
		{
			printf("Copying %s from old MCF.\n", m_pFileList[x]->getName());
			dest.copyFile(m_pFileList[x], lastOffset, hFileSrc, hFileDest);
		}
	}

	printf("Doing crc!\n");

	hFileSrc.close();
	hFileDest.close();

	dest.saveMCF_Header();
	dest.makeCRC();
}

void MCF::optimiseAndSaveMcf(MCFI* prevMcf, const char* path)
{
	printf("Optimising mcf!!\n");

	if (m_bStopped)
		return;

	if (!path)
		throw gcException(ERR_INVALID, "Path is null");


	MCF* mcf = dynamic_cast<MCF*>(prevMcf);

	if (!mcf)
		return;

	std::vector<mcfDif_s> vDiff;
	std::vector<mcfDif_s> vNew;
	std::vector<mcfDif_s> vSame;

	this->findChanges(mcf, &vSame, &vDiff, NULL, &vNew);

	MCF dest;
	dest.setFile(path);
	dest.setHeader(this->getHeader());
	dest.getHeader()->updateFileVersion();

	//allways start after the header.
	uint64 lastOffset = dest.getHeader()->getSize();

	UTIL::FS::FileHandle hFileSrc(getFile(), UTIL::FS::FILE_READ);
	UTIL::FS::FileHandle hFileDest(path, UTIL::FS::FILE_WRITE);

	hFileDest.seek(lastOffset);

	for (size_t x=0; x<vNew.size(); x++)
	{
		MCFFile* file = m_pFileList[vNew[x].thisMcf];

		if (file->isSaved())
		{
			printf("Copying New File %s\n", file->getName());
			dest.copyFile(file, lastOffset, hFileSrc, hFileDest);
		}
	}

	for (size_t x=0; x<vDiff.size(); x++)
	{
		MCFFile* file = m_pFileList[vDiff[x].thisMcf];

		if (file->isSaved())
		{
			printf("Copying Diff File %s\n", file->getName());
			dest.copyFile(file, lastOffset, hFileSrc, hFileDest);
		}
	}

	for (size_t x=0; x<vSame.size(); x++)
	{
		MCFFile* file = m_pFileList[vSame[x].thisMcf];

		if (file->isSaved())
		{
			printf("Copying Same File %s\n", file->getName());
			dest.copyFile(file, lastOffset, hFileSrc, hFileDest);
		}
	}

	printf("Doing crc!\n");

	hFileSrc.close();
	hFileDest.close();

	dest.saveMCF_Header();
	dest.makeCRC();
}


void MCF::saveMCF()
{
	if (m_bStopped)
		return;

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(m_szFile));

	saveMCF_CandSFiles();
	saveMCF_Header();
}


struct OffsetSortKey
{
	bool operator()(MCFFile *lhs, MCFFile *rhs )
	{
		return (lhs->getOffSet() < rhs->getOffSet());
	}
};

void MCF::saveXml(const char* file)
{
	UTIL::FS::FileHandle handle(file, UTIL::FS::FILE_WRITE);

	try
	{
		std::sort(m_pFileList.begin(), m_pFileList.end(), OffsetSortKey());

		XMLSaveAndCompress sac(&handle, false);
		genXml(&sac);

		sac.finish();
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to save mcf xml to {0}: {1}", file, e));
	}
}

void MCF::saveMCFHeader()
{
	if (m_bStopped)
		return;

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(m_szFile));
	saveMCF_Header();
}


void MCF::preAllocateFile()
{
	uint64 offset = getDLSize() + m_sHeader->getSize();

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(m_szFile));
	UTIL::FS::FileHandle hFile(m_szFile.c_str(), UTIL::FS::FILE_APPEND);
	uint64 pos = UTIL::FS::getFileSize(m_szFile.c_str());

	MCFCore::Misc::ProgressInfo prog;

	prog.totalAmmount = offset;

	//we do this as if its a massive file. Seeking will zero the end and take ages
	while (pos < offset)
	{
		if (m_bStopped)
			break;

		pos += 10*1024*1024;

		if (pos > offset)
			pos = offset;

		hFile.seek(pos);

		prog.percent = (uint8)(pos * 100 / offset);
		prog.doneAmmount = pos;

		onProgressEvent(prog);
	}

	if (!m_bStopped)
		return;

	// Got canceled
	hFile.close();
	UTIL::FS::delFile(m_szFile.c_str());
}

void MCF::saveMCF_Header()
{
	//donot use getDLSize here as the file might have a gap in it. :(

	uint64 offset = 0;

	for (size_t x=0; x<m_pFileList.size(); x++)
	{
		MCFFile* file = m_pFileList[x];

		if (!file->isSaved())
			continue;

		uint64 pos = file->getOffSet() + file->getCurSize();

		if (offset < pos)
			offset = pos;

		uint64 diffpos = file->getDiffOffSet() + file->getDiffSize();

		if (file->hasDiff() && offset < diffpos)
			offset = diffpos;
	}

	if (offset == 0)
		offset = m_sHeader->getSize();

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(m_szFile));
	UTIL::FS::FileHandle hFile(m_szFile.c_str(), UTIL::FS::FILE_APPEND);

	hFile.seek(offset);

	XMLSaveAndCompress sac(&hFile, isCompressed());
	genXml(&sac);
	sac.finish();

	m_sHeader->setXmlStart(offset);	
	m_sHeader->setXmlSize((uint32)sac.getTotalSize());
	m_sHeader->saveToFile(hFile);
}

void MCF::parseMCF()
{
	if (m_bStopped)
		return;

	UTIL::FS::FileHandle hFile;
	getReadHandle(hFile);

	safe_delete(m_pFileList);

	MCFCore::MCFHeader tempHeader;
	tempHeader.readFromFile(hFile);
	setHeader(&tempHeader);

	hFile.seek(tempHeader.getXmlStart());

	uint32 xmlBuffLen = tempHeader.getXmlSize()+1;
	UTIL::MISC::Buffer xmlBuff(xmlBuffLen, true);

	hFile.read(xmlBuff, tempHeader.getXmlSize());


	if (getHeader()->getFlags() & MCFCore::MCFHeaderI::FLAG_NOTCOMPRESSED)
	{
		parseXml(xmlBuff, xmlBuffLen);
	}
	else
	{
		UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_DECOMPRESS);

		worker.write(xmlBuff, xmlBuffLen, true);
		worker.doWork();

		if (worker.getLastStatus() != BZ_STREAM_END)
			throw gcException(ERR_BZ2, worker.getLastStatus(), "Failed to decompress mcf header xml");

		size_t bz2BuffLen = worker.getReadSize();

		UTIL::MISC::Buffer bz2Buff(bz2BuffLen);

		worker.read(bz2Buff, bz2BuffLen);
		parseXml(bz2Buff, bz2BuffLen);
	}
}




void MCF::saveBlankMcf()
{
	std::vector<size_t> saved;

	for (size_t x=0; x<m_pFileList.size(); x++)
	{
		if (m_pFileList[x]->isSaved())
		{
			saved.push_back(x);
			m_pFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_SAVE);
		}
	}

	saveMCF_Header();

	for (size_t x=0; x<saved.size(); x++)
	{
		m_pFileList[saved[x]]->addFlag(MCFCore::MCFFileI::FLAG_SAVE);
	}
}


bool MCF::fixMD5AndCRC()
{
	gcString empty("d41d8cd98f00b204e9800998ecf8427e");
	bool fixed = false;

	UTIL::FS::FileHandle handle;
	getReadHandle(handle);

	for (size_t x=0; x<m_pFileList.size(); x++)
	{
		if (m_pFileList[x]->isSaved() == false)
			continue;

		if (m_pFileList[x]->isCompressed() && empty == m_pFileList[x]->getCCsum())
		{
			fixed = true;
			m_pFileList[x]->generateMD5(handle);
		}

		fixed = true;
		m_pFileList[x]->generateCRC(handle);
	}	

	handle.close();

	if (fixed)
	{
		saveMCF_Header();
		return true;
	}

	return false;
}


}
