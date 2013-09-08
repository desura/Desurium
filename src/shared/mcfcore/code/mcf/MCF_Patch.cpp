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
#include "MCF.h"
#include "Courgette.h"
#include "util/MD5Progressive.h"

#define BLOCKSIZE (512*1024)

namespace UTIL
{
namespace MISC
{
unsigned long CRC32(const unsigned char byte, unsigned long dwCrc32);
}
}

class CRCInfo
{
public:
	CRCInfo(uint32 blockSize)
	{
		m_uiCrc = 0xFFFFFFFF;
		m_uiCount = 0;

		m_uiBlockSize = blockSize;
	}

	void finish()
	{
		m_uiCrc = ~m_uiCrc;
		vCRCList.push_back(m_uiCrc);

		m_uiCrc = 0xFFFFFFFF;
		m_uiCount = 0;
	}

	void generate(const char* buff, uint32 size)
	{
		for (size_t x=0; x<size; x++)
		{
			if (m_uiCount == m_uiBlockSize)
				finish();

			m_uiCrc = UTIL::MISC::CRC32(buff[x], m_uiCrc);
			m_uiCount++;
		}
	}

	std::vector<uint32> vCRCList;

private:
	uint32 m_uiCrc;
	uint32 m_uiCount;
	uint32 m_uiBlockSize;
};

namespace MCFCore
{

void MCF::resetSavedFiles()
{
	for (size_t x=0; x<m_pFileList.size(); x++ )
	{
		m_pFileList[x]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );
	}
}

void MCF::copyFile(MCFCore::MCFFile* file, uint64 &lastOffset, UTIL::FS::FileHandle& hFileSrc, UTIL::FS::FileHandle& hFileDest)
{
	if (m_bStopped)
		return;

	if (!hFileSrc.isValidFile())
		throw gcException(ERR_NULLSRCFILE);

	if (!hFileDest.isValidFile())
		throw gcException(ERR_NULLDESTFILE);

	uint32 element = findFileIndexByHash(file->getHash());

	MCFCore::MCFFile* temp = NULL;

	if (element == UNKNOWN_ITEM)
	{
		temp = new MCFCore::MCFFile();
		m_pFileList.push_back(temp);
	}
	else
	{
		temp = m_pFileList[element];
	}

	temp->copySettings(file);
	temp->setOffSet(0);

	if (!file->isZeroSize())
	{
		hFileDest.seek(lastOffset);
		hFileSrc.seek(file->getOffSet());

		if (file->isCompressed() && file->getCSize() > file->getSize())
		{
			UTIL::MISC::BZ2Worker bz(UTIL::MISC::BZ2_DECOMPRESS);


			temp->resetCRC();
			CRCInfo ci(temp->getBlockSize());

			hFileSrc.read(file->getCurSize(), [&](const unsigned char* buff, uint32 size)  -> bool
			{
				UTIL::FS::FileHandle* dest = &hFileDest;

				CRCInfo* pCi = &ci;

				bz.write((const char*)buff, size, [&](const unsigned char* tbuff, uint32 tsize) -> bool
				{
					dest->write((const char*)tbuff, tsize);
					pCi->generate((const char*)tbuff, tsize);

					return false;
				});

				return false;
			});

			bz.end([&hFileDest, &ci](const unsigned char* tbuff, uint32 tsize) -> bool
			{
				hFileDest.write((const char*)tbuff, tsize);
				ci.generate((const char*)tbuff, tsize);

				return false;
			});

			ci.finish();

			temp->delFlag(MCFCore::MCFFileI::FLAG_COMPRESSED);
			temp->setCRC(ci.vCRCList);
		}
		else
		{
			hFileSrc.read(file->getCurSize(), [&hFileDest](const unsigned char* buff, uint32 size) -> bool
			{
				hFileDest.write((const char*)buff, size);
				return false;
			});
		}

		temp->setOffSet(lastOffset);
		lastOffset += temp->getCurSize();
	}
}

void MCF::copyMissingFiles(MCFI *sourceMcf)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(sourceMcf);

	if (!temp)
		return;

	std::vector<mcfDif_s> vSame;
	std::vector<mcfDif_s>  vProcessList;
	findChanges( temp, &vSame, NULL, NULL);

	if (vSame.size() == 0)
		return;

	uint64 lastOffset = getHeader()->getSize();

	//find the last files offset
	for (size_t x=0; x< m_pFileList.size(); x++)
	{
		if (!m_pFileList[x]->isSaved())
			continue;

		if ((m_pFileList[x]->isComplete() || m_pFileList[x]->hasStartedDL()) && m_pFileList[x]->getOffSet() >= lastOffset)
			lastOffset = m_pFileList[x]->getOffSet() + m_pFileList[x]->getCurSize();
	}

	UTIL::FS::FileHandle hFileSrc;
	UTIL::FS::FileHandle hFileDest;

	hFileSrc.open(temp->getFile(), UTIL::FS::FILE_READ);

	bool destValid = UTIL::FS::isValidFile(getFile());

	if (destValid)
		hFileDest.open(getFile(), UTIL::FS::FILE_READ);

	size_t totalCount = vSame.size()*2;
	size_t curCount = 0;
	bool placeholder = false;

	for (size_t x=0; x<vSame.size(); x++)
	{
		MCFCore::MCFFile* tempFile = temp->getFile(vSame[x].otherMcf);
		MCFCore::MCFFile* thisFile = m_pFileList[vSame[x].thisMcf];

		if (thisFile->isSaved() && destValid)
			thisFile->verifyMcf(hFileDest, placeholder);

		if (tempFile->isSaved() && !(thisFile->isSaved() && thisFile->isComplete()) )
		{
			vProcessList.push_back(vSame[x]);
		}
		else
		{
			//we are not copying the file below so increase again
			curCount++;
		}

		curCount++;

		MCFCore::Misc::ProgressInfo temp;
		temp.doneAmmount = curCount;
		temp.totalAmmount = totalCount;
		temp.percent = (uint8)(curCount*100/totalCount);
		onProgressEvent(temp);
	}
	hFileDest.close();

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(getFile()));
	hFileDest.open(getFile(), UTIL::FS::FILE_APPEND);

	for (size_t x=0; x<vProcessList.size(); x++)
	{
		MCFCore::MCFFile* tempFile = temp->getFile(vProcessList[x].otherMcf);
		MCFCore::MCFFile* thisFile = m_pFileList[vProcessList[x].thisMcf];

		tempFile->verifyMcf(hFileSrc, placeholder);
		bool isComplete = tempFile->isComplete();

		if (thisFile->hasStartedDL())
		{
			uint64 offset = thisFile->getOffSet();
			copyFile(tempFile, offset, hFileSrc, hFileDest);
		}
		else if (isComplete)
		{
			copyFile(tempFile, lastOffset, hFileSrc, hFileDest);

			thisFile->addFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
			thisFile->delFlag(MCFCore::MCFFileI::FLAG_STARTEDDL);
		}
		else
		{
			thisFile->delFlag(MCFCore::MCFFileI::FLAG_STARTEDDL|MCFCore::MCFFileI::FLAG_COMPLETE);
		}

		curCount++;
		MCFCore::Misc::ProgressInfo temp;
		temp.doneAmmount = curCount;
		temp.totalAmmount = totalCount;
		temp.percent = (uint8)(curCount*100/totalCount);
		onProgressEvent(temp);
	}

	//reset the file offsets so it doesnt fuck the mcf up
	for (size_t x=0; x< m_pFileList.size(); x++)
	{
		if (!m_pFileList[x]->isSaved())
			continue;

		if (!m_pFileList[x]->isComplete() && !m_pFileList[x]->hasStartedDL())
		{
			m_pFileList[x]->setOffSet(lastOffset);
			lastOffset += m_pFileList[x]->getCurSize();
		}
	}

	hFileSrc.close();
	hFileDest.close();

	saveMCF_Header();
}

void MCF::makeFullFile(MCFI* patchFile, const char* path)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(patchFile);

	bool matches = getHeader()->matches(temp->getHeader());

	if (!matches)
		throw gcException(ERR_INVALIDFILE, "The patch MCF Header doesnt match the parent MCF Header");

	uint32 par = temp->getHeader()->getParent();
	uint32 ver = this->getHeader()->getBuild();

	if (par != ver)
		throw gcException(ERR_INVALIDFILE, gcString("The patch MCF parent version doesnt match the parent MCF version ({0} != {1})", par, ver));

	std::vector<mcfDif_s> vSame;
	std::vector<size_t> vOther;
	findSameHashFile(temp, vSame, vOther);

	// remove all the deleted files from this MCF
	for (size_t x=0; x<vOther.size(); x++)
	{
		m_pFileList[vOther[x]]->delFlag( MCFCore::MCFFileI::FLAG_SAVE );
	}

	// remove all the same files that are in the patch from the patch as to group the new files together
	for (size_t x=0; x<vSame.size(); x++)
	{
		MCFCore::MCFFile* tempFile = temp->getFile(vSame[x].otherMcf);
		MCFCore::MCFFile* thisFile = m_pFileList[vSame[x].thisMcf];

		if (thisFile->isSaved())
		{
			tempFile->delFlag(MCFCore::MCFFileI::FLAG_SAVE);
		}
		else
		{
			thisFile->delFlag( MCFCore::MCFFileI::FLAG_SAVE );

			//copy executable flag
			thisFile->addFlag( tempFile->getFlags() & MCFCore::MCFFileI::FLAG_XECUTABLE );

			//Make sure we copy the case as linux might change it for patches
			thisFile->setPath(tempFile->getPath());
			thisFile->setName(tempFile->getName());
		}
	}

	MCF fullMcf;
	fullMcf.setFile(path);
	fullMcf.setHeader(temp->getHeader());
	fullMcf.getHeader()->updateFileVersion();

	//allways start after the header.
	uint64 lastOffset = fullMcf.getHeader()->getSize();

	UTIL::FS::FileHandle hFileSrc(temp->getFile(), UTIL::FS::FILE_READ);
	UTIL::FS::FileHandle hFileDest(path, UTIL::FS::FILE_WRITE);

	hFileDest.seek(lastOffset);

	//copy all the files from the patch first as they are more likley to be downloaded
	for (uint32 x=0; x<(uint32)temp->getFileCount(); x++)
	{
		MCFCore::MCFFile* tempFile = temp->getFile(x);

		if (!tempFile || !tempFile->isSaved())
			continue;

		printf("Copying %s from patch MCF.\n", tempFile->getName());
		fullMcf.copyFile(tempFile, lastOffset, hFileSrc, hFileDest);
	}

	hFileSrc.close();
	hFileSrc.open(getFile(), UTIL::FS::FILE_READ);

	//copy all the remaining files from the original MCF
	for (uint32 x=0; x<(uint32)m_pFileList.size(); x++)
	{
		if (!m_pFileList[x]->isSaved())
			continue;

		printf("Copying %s from old MCF.\n", m_pFileList[x]->getName());
		fullMcf.copyFile(m_pFileList[x], lastOffset, hFileSrc, hFileDest);
	}

	hFileSrc.close();
	hFileDest.close();

	hFileDest.open(path, UTIL::FS::FILE_READ);

	for (size_t x=0; x<fullMcf.getFileCount(); x++)
	{
		MCFFile* file = fullMcf.getFile(x);

		if (file->getCRCCount() == 0)
			file->generateCRC(hFileDest);
	}

	hFileDest.close();

	fullMcf.saveMCF_Header();
}

void MCF::makeBackPatchMCF(MCFI* backFile, const char* path)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(backFile);

	std::vector<mcfDif_s> vSame;
	findChanges( temp, &vSame, NULL, NULL);

	for (size_t x=0; x<vSame.size(); x++)
	{
		m_pFileList[vSame[x].thisMcf]->delFlag( MCFCore::MCFFileI::FLAG_SAVE );
	}

	MCF FullFile;
	FullFile.setFile(path);

	FullFile.setHeader(temp->getHeader());

	//allways start after the header.
	uint64 lastOffset = FullFile.getHeader()->getSize();

	UTIL::FS::FileHandle hFileSrc;
	UTIL::FS::FileHandle hFileDest;

	hFileSrc.open(this->getFile(), UTIL::FS::FILE_READ);
	hFileDest.open(path, UTIL::FS::FILE_WRITE);

	for (uint32 x=0; x<this->getFileCount(); x++)
	{
		MCFCore::MCFFile* tempFile = this->getFile(x);

		if (tempFile && !tempFile->isSaved())
		{
			FullFile.addFile( new MCFCore::MCFFile(tempFile));
		}
		else
		{
			FullFile.copyFile(tempFile, lastOffset, hFileSrc, hFileDest);
		}
	}

	hFileSrc.close();
	hFileDest.close();

	FullFile.saveMCF_Header();

}

void MCF::markFiles(MCFI* inMcf, bool tagSame, bool tagChanged, bool tagDeleted, bool tagNew)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(inMcf);

	std::vector<mcfDif_s> vSame;
	std::vector<mcfDif_s> vDiff;
	std::vector<mcfDif_s> vDel;
	std::vector<mcfDif_s> vNew;

	findChanges(temp, tagSame?&vSame:NULL, tagChanged?&vDiff:NULL, tagDeleted?&vDel:NULL, tagNew?&vNew:NULL);

	for (size_t x=0; x<m_pFileList.size(); x++ )
		m_pFileList[x]->delFlag( MCFCore::MCFFileI::FLAG_SAVE );

	for (size_t x=0; x<vSame.size(); x++)
		m_pFileList[vSame[x].thisMcf]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );

	for (size_t x=0; x<vDiff.size(); x++)
		m_pFileList[vDiff[x].thisMcf]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );

	for (size_t x=0; x<vDel.size(); x++)
		m_pFileList[vDel[x].thisMcf]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );

	for (size_t x=0; x<vNew.size(); x++)
		m_pFileList[vNew[x].thisMcf]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );
}

void MCF::markChanged(MCFI* inMcf)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(inMcf);

	std::vector<mcfDif_s> vSame;
	findChanges( temp, &vSame, NULL, NULL);

	for (size_t x=0; x<m_pFileList.size(); x++ )
	{
		m_pFileList[x]->addFlag( MCFCore::MCFFileI::FLAG_SAVE );
	}

	for (size_t x=0; x<vSame.size(); x++)
	{
		m_pFileList[vSame[x].thisMcf]->delFlag( MCFCore::MCFFileI::FLAG_SAVE );
	}
}

void MCF::getPatchStats(MCFI* inMcf, uint64* dlSize, uint32* fileCount)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(inMcf);

	if (!temp)
		return;
	
	uint64 sameSize = 0;

	std::vector<mcfDif_s> vSame;
	findChanges(temp, &vSame, NULL, NULL);

	for (size_t x=0; x<vSame.size(); x++)
		sameSize += m_pFileList[vSame[x].thisMcf]->getCurSize();

	if (dlSize)
		*dlSize = getDLSize()-sameSize;

	if (fileCount)
		*fileCount = m_pFileList.size() - vSame.size();
}

void MCF::makePatch(MCFI* file)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(file);

	std::vector<mcfDif_s> vSame;
	findChanges( temp, &vSame, NULL, NULL);

	for (size_t x=0; x<vSame.size(); x++)
	{
		m_pFileList[vSame[x].thisMcf]->copySettings(temp->getFile(vSame[x].otherMcf));
		m_pFileList[vSame[x].thisMcf]->delFlag(MCFCore::MCFFileI::FLAG_SAVE);
		m_pFileList[vSame[x].thisMcf]->setOffSet(0);
	}
}

void MCF::findSameHashFile(MCF* newFile, std::vector<mcfDif_s> &vSame, std::vector<size_t> &vOther)
{
	std::vector<MCFCore::MCFFile*> vThisFileList = m_pFileList;
	std::vector<MCFCore::MCFFile*> vNewFileList = newFile->getFileList();

	for (size_t x=0; x<vThisFileList.size(); x++)
	{
		bool found = false;
		
		for (size_t y=0; y<vNewFileList.size(); y++)
		{
			MCFCore::MCFFile* a = vThisFileList[x];
			MCFCore::MCFFile* b = vNewFileList[y];

			if (a->getSize() != b->getSize())
				continue;

			if (gcString(a->getCsum()) == gcString(b->getCsum()))
			{
				mcfDif_s t;
				t.otherMcf = y;
				t.thisMcf = x;

				vSame.push_back(t);
				
				found = true;
			}
		}
		
		if (!found)
			vOther.push_back(x);
	}
}

void MCF::findChanges(MCF* newFile,  std::vector<mcfDif_s> *vSame, std::vector<mcfDif_s> *vDiff, std::vector<mcfDif_s> *vDel, std::vector<mcfDif_s> *vNew)
{
	if (m_bStopped)
		return;

	newFile->sortFileList();
	this->sortFileList();

	std::vector<uint32> vUsedList;

	for (size_t x=0; x<m_pFileList.size(); x++ )
	{
		mcfDif_s temp;
		uint32 element = newFile->findFileIndexByHash( m_pFileList[x]->getHash() );

		if (element == UNKNOWN_ITEM)
		{
			temp.thisMcf = (uint32)x;
			temp.otherMcf = UNKNOWN_ITEM;

			if (vDel)
				(*vDel).push_back(temp);

			continue;
		}

		vUsedList.push_back(element);

		int res = m_pFileList[x]->isEquals( newFile->getFile(element) );

		switch (res)
		{
		case MCFCore::MCFFileI::CMP_SAME:
			temp.thisMcf = (uint32)x;
			temp.otherMcf = element;

			if (vSame)
				(*vSame).push_back(temp);

			break;

		case MCFCore::MCFFileI::CMP_OLDER:
		case MCFCore::MCFFileI::CMP_NEWER:
			temp.thisMcf = (uint32)x;
			temp.otherMcf = element;

			if (vDiff)
				(*vDiff).push_back(temp);

			break;
		}
	}

	if (!vNew)
		return;

	for (size_t x=0; x<newFile->getFileCount(); x++)
	{
		if (std::find(vUsedList.begin(), vUsedList.end(), x) == vUsedList.end())
		{
			mcfDif_s temp;
			temp.thisMcf = UNKNOWN_ITEM;
			temp.otherMcf = (uint32)x;
			vNew->push_back(temp);
		}
	}
}


void MCF::createCourgetteDiffs(MCFI* oldMcf, const char* outPath)
{
	if (m_bStopped)
		return;

	MCF *temp = static_cast<MCF*>(oldMcf);

	if (!temp)
		throw gcException(ERR_NULLHANDLE, "The old mcf was null!");

	std::vector<mcfDif_s> vDiff;
	std::vector<mcfDif_s> vSame;
	std::vector<mcfDif_s> vNew;

	findChanges(temp, &vSame, &vDiff, &vNew, NULL);

	if (vDiff.size() == 0)
	{
		UTIL::FS::copyFile(getFile(), outPath);
		return;
	}

	UTIL::FS::FileHandle hFileSrc;
	UTIL::FS::FileHandle hFileDest;

	MCF FullFile;
	FullFile.setFile(outPath);
	FullFile.setHeader(temp->getHeader());
	FullFile.getHeader()->updateFileVersion();

	uint64 lastOffset = FullFile.getHeader()->getSize();

	auto copyFiles = [this, &FullFile, &lastOffset, &hFileSrc, &hFileDest](size_t x)
	{
		MCFCore::MCFFile* tempFile = getFile(x);

		if (tempFile && !tempFile->isSaved())
		{
			FullFile.addFile( new MCFCore::MCFFile(tempFile));
		}
		else
		{
			FullFile.copyFile(tempFile, lastOffset, hFileSrc, hFileDest);
		}
	};

	hFileSrc.open(this->getFile(), UTIL::FS::FILE_READ);
	hFileDest.open(outPath, UTIL::FS::FILE_WRITE);

	for (size_t x=0; x<vSame.size(); x++)
		copyFiles(vSame[x].thisMcf);

	for (size_t x=0; x<vNew.size(); x++)
		copyFiles(vNew[x].thisMcf);

	for (size_t x=0; x<vDiff.size(); x++)
		copyFiles(vDiff[x].thisMcf);

	hFileSrc.close();

	CourgetteInstance ci;

	FullFile.sortFileList();

	for (size_t x=0; x<vDiff.size(); x++)
	{
		MCFFile* newMCFFile = this->getFile(vDiff[x].thisMcf);
		MCFFile* oldMCFFile = temp->getFile(vDiff[x].otherMcf);

		uint32 index = FullFile.findFileIndexByHash(newMCFFile->getHash());

		MCFFile* copyMCFFile = FullFile.getFile(index);
			
		UTIL::MISC::Buffer oldBuff(0);
		UTIL::MISC::Buffer newBuff(0);

		extractFile(this->getFile(), newMCFFile, newBuff);
		extractFile(temp->getFile(), oldMCFFile, oldBuff);

		printf("Creating courgette diff for: %s\n", newMCFFile->getName());
		createCourgetteDiff(&ci, oldBuff, newBuff, oldMCFFile->getCsum(), copyMCFFile, hFileDest);
			
		copyMCFFile->setDiffOffset(lastOffset);
		lastOffset += copyMCFFile->getDiffSize();
	}

	temp->getHeader()->addFlags(MCFCore::MCFHeaderI::FLAG_COURGETTE);

	hFileDest.close();
	FullFile.saveMCF_Header();
}

void MCF::createCourgetteDiff(CourgetteInstance* ci, UTIL::MISC::Buffer &oldBuff, UTIL::MISC::Buffer &newBuff, const char* oldHash, MCFFile* file, UTIL::FS::FileHandle& dest)
{
	MD5Progressive md5;
	uint64 totSize = 0;

	UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_COMPRESS);

	bool res = ci->createDiff(oldBuff.data(), oldBuff.size(), newBuff.data(), newBuff.size(), [&worker, &totSize, &md5](const char* buff, size_t size) -> bool
	{
		try
		{
			worker.write(buff, size, false);
			worker.doWork();

			md5.update(buff, size);
			totSize += size;
		}
		catch (gcException)
		{
			return false;
		}

		return true;
	});

	if (!res)
		throw gcException(ERR_BADSTATUS, res, "Failed to create diff. Bad response!");

	worker.write(NULL, 0, true);

	while (worker.getLastStatus() != BZ_STREAM_END)
		worker.doWork();

	size_t tot = worker.getReadSize();
	UTIL::MISC::Buffer buff(tot);

	worker.read(buff, tot);
	dest.write(buff, tot);
	
	std::string diffHash = md5.finish();

	file->addFlag(MCFCore::MCFFileI::FLAG_HASDIFF);
	file->setDiffInfo(oldHash, diffHash.c_str(), tot);
}

void MCF::extractFile(const char* mcfPath, MCFFile* file, UTIL::MISC::Buffer &outBuff)
{
	if (file->getSize() >> 32)
		throw gcException(ERR_INVALID, "File is larger than 4gb");

	UTIL::FS::FileHandle fh(mcfPath, UTIL::FS::FILE_READ);
	fh.seek(file->getOffSet());

	outBuff.resize((size_t)file->getSize());

	if (file->isCompressed())
	{
		UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_DECOMPRESS);

		char* out = outBuff.data();
		char temp[10*1024];
		size_t tsize = 10*1024;

		size_t todo = (size_t)file->getCSize();
		size_t outSize = 0;

		while (todo > 0)
		{
			if (tsize > todo)
				tsize = todo;

			fh.read(temp, tsize);
			worker.write(temp, tsize, false);

			worker.doWork();

			outSize = worker.getReadSize();
			worker.read(out, outSize);

			todo -= tsize;
			out += outSize;
		};

		worker.write(NULL, 0, true);

		do
		{
			worker.doWork();

			outSize = worker.getReadSize();
			worker.read(out, outSize);
			out += outSize;
		}
		while (outSize != 0);
	}
	else
	{
		fh.read(outBuff.data(), outBuff.size());
	}

	MD5Progressive md5;
	md5.update(outBuff.data(), outBuff.size());

	gcString md5Hash = md5.finish();

	if (md5Hash != gcString(file->getCsum()))
		throw gcException(ERR_HASHMISSMATCH, "The exstracted file hash didnt match what was expected");
}


}
