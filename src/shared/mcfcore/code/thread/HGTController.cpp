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
#include "HGTController.h"

#include "mcf/MCFFile.h"
#include "mcf/MCF.h"

#include "Courgette.h"
#include "util/MD5Progressive.h"

namespace MCFCore
{
namespace Thread
{

HGTController::HGTController(const char* url, MCFCore::MCF* caller, const char* insDir) : MCFCore::Thread::BaseMCFThread(1, caller, "WebGet Thread")
{
	m_uiDownloaded = 0;
	m_uiTotal = 0;

	m_szUrl = url;
	m_szInstallDir = insDir;

	setPriority(BELOW_NORMAL);

	m_pCurBlock = NULL;
}

HGTController::~HGTController()
{
	safe_delete(m_vSuperBlockList);
}

void HGTController::run()
{
	bool usingDiffs = false;

	fillDownloadList(usingDiffs);

	try
	{
		doDownload();

		//if we failed to expand the diffs, restart the download
		if (usingDiffs && !expandDiffs())
		{
			fillDownloadList(usingDiffs);

			if (usingDiffs)
				throw gcException(ERR_WEBDL_FAILED, "Diff download failed and shouldnt be trying again. :(");

			doDownload();
		}
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return;
	}
}

bool HGTController::expandDiffs()
{
	CourgetteInstance ci;

	bool failedSome = false;
	printf("\n");

	size_t fsSize = m_rvFileList.size();
	for (size_t x=0; x<fsSize; x++)
	{
		if (!HasAllFlags(m_rvFileList[x]->getFlags(), MCFFileI::FLAG_CANUSEDIFF))
			continue;

		printf("Expanding Courgette Diff for: %s\n", m_rvFileList[x]->getName());

		if (!expandDiff(&ci, m_rvFileList[x]))
		{
			m_rvFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
			failedSome = true;
		}
		else
		{
			m_rvFileList[x]->delFlag(MCFFileI::FLAG_COMPRESSED);
			m_rvFileList[x]->addFlag(MCFFileI::FLAG_COMPLETE);
		}

		m_rvFileList[x]->clearDiff();
	}

	return !failedSome;
}

void HGTController::decompressDiff(uint64 size, UTIL::FS::FileHandle &fhSrc, UTIL::FS::FileHandle &fhDest)
{
	uint64 done = 0;

	//uint32 buffSize = 10*1024;              unused variables
	//char buff[10*1024] = {0};

	UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_DECOMPRESS);

	fhSrc.read(done, [&worker, &fhDest](const unsigned char* buff, uint32 size) -> bool
	{
		UTIL::FS::FileHandle* pFile = &fhDest;

		worker.write((const char*)buff, size, [pFile](const unsigned char* tbuff, uint32 tsize) -> bool
		{
			pFile->write((const char*)tbuff, tsize);
			return false;
		});

		return false;
	});

	worker.end([&fhDest](const unsigned char* tbuff, uint32 tsize) -> bool
	{
		fhDest.write((const char*)tbuff, tsize);
		return false;
	});
}

bool HGTController::expandDiff(CourgetteInstance* ci, MCFCore::MCFFile* file)
{
	m_hFile.close();

	UTIL::FS::Path path(m_szInstallDir, file->getName(), false);
	path += file->getPath();

	gcString oldFile = path.getFullPath();
	gcString diffFile("{0}_diff", m_szFile);

	try
	{
		UTIL::FS::FileHandle fhSrc(m_szFile, UTIL::FS::FILE_READ);
		UTIL::FS::FileHandle fhDest(diffFile.c_str(), UTIL::FS::FILE_WRITE);
		fhSrc.seek(file->getOffSet());

		decompressDiff(file->getDiffSize(), fhSrc, fhDest);

		UTIL::FS::delFile(diffFile);
	}
	catch (gcException)
	{
		UTIL::FS::delFile(diffFile);
		return false;
	}

	MD5Progressive md5;

	try
	{
		m_hFile.open(m_szFile, UTIL::FS::FILE_APPEND);
		m_hFile.seek(file->getOffSet());
	}
	catch (...)
	{
		return false;
	}

	uint64 fsize = file->getSize();
	uint64 tot = 0;

	bool res = ci->applyDiff(oldFile.c_str(), diffFile.c_str(), [this, &md5, &tot, fsize](const char* buff, size_t size) -> bool
	{
		md5.update(buff, size);
		return writeDiff(tot, fsize, buff, size);
	});

	return (res && md5.finish() == file->getCsum());
}

bool HGTController::writeDiff(uint64 &tot, uint64 fsize, const char* buff, size_t size)
{
	//make sure we dont rape other files
	if ((tot+size) > fsize)
		return false;

	try
	{
		m_hFile.write(buff, size);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

void HGTController::doDownload()
{
	//header should be saved all ready so appened to it
	m_hFile.open(m_szFile, UTIL::FS::FILE_APPEND);
	m_hFile.seek(0);

	HttpHandle wc(m_szUrl.c_str());

	//wc->getProgressEvent() += delegate(this, &HGTController::onProgress);
	wc->getWriteEvent() += delegate(this, &HGTController::onWriteMemory);

	for (size_t x=0; x<m_vSuperBlockList.size(); x++)
	{
		m_pCurBlock = m_vSuperBlockList[x];

		wc->cleanUp();
		wc->setDownloadRange(m_pCurBlock->offset, m_pCurBlock->size);

		try
		{
			m_hFile.seek(m_pCurBlock->vBlockList[0]->fileOffset);
			wc->getWeb();
		}
		catch (gcException &e)
		{
			onErrorEvent(e);
			nonBlockStop();
		}
	}

	size_t fsSize = m_rvFileList.size();
	for (size_t x=0; x<fsSize; x++)
	{
		if (!HasAllFlags(m_rvFileList[x]->getFlags(), MCFFileI::FLAG_CANUSEDIFF))
			m_rvFileList[x]->addFlag(MCFFileI::FLAG_COMPLETE);
	}
}

static bool WGTBlockSort(Misc::WGTBlock* a, Misc::WGTBlock* b)
{
	return a->webOffset < b->webOffset;
}

static bool SortByOffset(MCFCore::MCFFile* a, MCFCore::MCFFile* b)
{
	return a->getOffSet() < b->getOffSet();
}

void HGTController::fillDownloadList(bool &usingDiffs)
{
	usingDiffs = false;
	safe_delete(m_vSuperBlockList);
	MCFCore::MCF *webMcf = new MCFCore::MCF();

	try
	{
		webMcf->dlHeaderFromHttp(m_szUrl.c_str());
	}
	catch (gcException &e)
	{
		onErrorEvent(e);
		safe_delete(webMcf);
		return;
	}

	webMcf->sortFileList();

	uint64 mcfOffset = m_pHeader->getSize();
	size_t fsSize = m_rvFileList.size();

	//find the last files offset
	for (size_t x=0; x< fsSize; x++)
	{
		if (!m_rvFileList[x]->isSaved())
			continue;

		if ((m_rvFileList[x]->isComplete() || m_rvFileList[x]->hasStartedDL()) && m_rvFileList[x]->getOffSet() > mcfOffset)
		{
			mcfOffset = m_rvFileList[x]->getOffSet() + m_rvFileList[x]->getCurSize();
		}
	}

	std::deque<Misc::WGTBlock*> vBlockList;
	std::sort(m_rvFileList.begin(), m_rvFileList.end(), SortByOffset);

	for (size_t x=0; x<fsSize; x++)
	{
		//dont download all ready downloaded items
		if (m_rvFileList[x]->isComplete())
			continue;

		//skip files that arnt "saved" in the MCF
		if (!m_rvFileList[x]->isSaved())
			continue;	

		uint32 index = webMcf->findFileIndexByHash(m_rvFileList[x]->getHash());
		MCFCore::MCFFile *webFile = webMcf->getFile(index);

		uint64 size = m_rvFileList[x]->getCurSize();
		bool started = m_rvFileList[x]->hasStartedDL();

		if (index == UNKNOWN_ITEM || !webFile || !webFile->isSaved())
		{
			Warning(gcString("File {0} is not in web MCF. Skipping download.\n", m_rvFileList[x]->getName()));

			if (!started)
				m_rvFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_SAVE);

			continue;
		}	


		if (!started)
			m_rvFileList[x]->setOffSet(mcfOffset);


		Misc::WGTBlock* temp = new Misc::WGTBlock;

		temp->fileOffset = m_rvFileList[x]->getOffSet();
		temp->file = m_rvFileList[x];

		if (webFile->hasDiff() && HasAllFlags(m_rvFileList[x]->getFlags(), MCFFileI::FLAG_CANUSEDIFF))
		{
			temp->webOffset = webFile->getDiffOffSet();
			temp->size = webFile->getDiffSize();

			if (!started)
				mcfOffset += webFile->getSize();

			usingDiffs = true;
		}
		else
		{
			m_rvFileList[x]->delFlag(MCFFileI::FLAG_CANUSEDIFF);
			temp->webOffset = webFile->getOffSet();
			temp->size = size;

			if (!started)
				mcfOffset += size;
		}

		vBlockList.push_back(temp);
		m_uiTotal += temp->size;
	}


	std::sort(vBlockList.begin(), vBlockList.end(), &WGTBlockSort);

	while (!vBlockList.empty())
	{
		Misc::WGTSuperBlock* sb = new Misc::WGTSuperBlock();
		sb->offset = vBlockList[0]->webOffset;

		do
		{
			Misc::WGTBlock* block = vBlockList.front();
			vBlockList.pop_front();

			sb->size += block->size;
			sb->vBlockList.push_back(block);
		}
		while (!vBlockList.empty() && vBlockList[0]->webOffset == (sb->offset + sb->size));

		m_vSuperBlockList.push_back(sb);
	}
}

void HGTController::onWriteMemory(WriteMem_s& ws)
{
	ws.handled = true;
	ws.wrote = ws.size;
	ws.stop = isStopped();

	try
	{
		ws.stop = saveData((const char*)ws.data, ws.size);
	}
	catch (gcException)
	{
		ws.stop = true;
	}

	m_uiDownloaded += ws.size;
	onProgress();
}

bool HGTController::saveData(const char* data, uint32 size)
{
	if (isStopped())
		return true;

	if (size == 0)
		return false;

	if (!m_pCurBlock || m_pCurBlock->vBlockList.empty())
		return true;

	MCFCore::Thread::Misc::WGTBlock* block = m_pCurBlock->vBlockList[0];

	uint64 done = m_pCurBlock->done;
	uint64 ds = block->size - done;

	if (done == 0)
	{
		m_hFile.seek(block->fileOffset);
		block->file->addFlag(MCFCore::MCFFileI::FLAG_STARTEDDL);
	}

	if (size >= ds)
	{
		m_hFile.write((char*)data, (uint32)ds);

		m_pCurBlock->vBlockList.pop_front();
		m_pCurBlock->done = 0;

		block->file->addFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
		block->file->delFlag(MCFCore::MCFFileI::FLAG_STARTEDDL);

		saveData(data+ds, (uint32)(size-ds));
	}
	else
	{
		m_hFile.write((char*)data, size);
		m_pCurBlock->done += size;
	}

	return false;
}

void HGTController::onProgress()
{
	MCFCore::Misc::ProgressInfo prog;

	prog.doneAmmount = m_uiDownloaded;
	prog.totalAmmount = m_uiTotal;
	prog.percent = (uint8)(m_uiDownloaded*100/m_uiTotal);

	onProgressEvent(prog);
}

}
}
