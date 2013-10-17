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


#ifndef DESURA_MCF_H
#define DESURA_MCF_H

#include "Common.h"

#include "mcfcore/MCFI.h"
#include "MCFFile.h"
#include "MCFHeader.h"

#include <vector>
#include <string>
#include <algorithm>
#include <string.h>

#include "util_thread/BaseThread.h"
#include "thread/UpdateThread.h"


#ifdef WIN32
	#include <direct.h>
#endif

//this is disable the warning for std::vector not being an export class.
#pragma warning( disable: 4251 )


//max number of threads to download at any one time
#define MAX_DL_THREADS 3

//unknown_item is defined in common.h
#define MCF_NOINDEX UNKNOWN_ITEM


class CourgetteInstance;



namespace MCFCore{  
	
namespace MISC
{
	class DownloadProvider;
	class UserCookies; 
}

class CEXPORT MCF : public MCFI
{
public:
	MCF();
	MCF(std::vector<MCFCore::Misc::DownloadProvider*> &vProviderList, Misc::GetFile_s* pFileAuth);
	virtual ~MCF();


	//mcf functions

	//! Adds a file to the file list
	//! 
	//! @param file MCFFile to add
	//!
	void addFile(MCFCore::MCFFile* file);

	//! Finds a files index by its hash
	//!
	//! @param hash File hash
	//! @return File index
	//!
	uint32 findFileIndexByHash(uint64 hash);

	//! Gets the file at an index
	//!
	//! @param index File index
	//! @return MCFFile
	//!
	MCFCore::MCFFile* getFile(uint32 index);
	
	//! Gets the file list
	//!
	//! @return MCFile list
	//!
	std::vector<MCFCore::MCFFile*>& getFileList()
	{
		return m_pFileList;
	}

	//! Sorts the file list by hash
	//!
	void sortFileList();

	//! Prints all the file information to the console
	//!
	void printAll();


	//inhereted functions
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual MCFCore::MCFHeaderI* getHeader();
	virtual uint64 getDLSize();
	virtual uint64 getINSize();
	virtual uint64 getFileSize();
	virtual uint32 getFileCount();
	virtual MCFCore::MCFFileI* getMCFFile(uint32 index);
	virtual const char* getFile();
	virtual void getDownloadProviders(const char* url, MCFCore::Misc::UserCookies *pCookies, bool *unauthed = NULL, bool local = false);
	virtual Misc::GetFile_s* getAuthInfo();
	virtual Event<MCFCore::Misc::ProgressInfo>& getProgEvent();
	virtual Event<gcException>&	getErrorEvent();
	virtual Event<MCFCore::Misc::DP_s>& getNewProvider();
	virtual bool isCompressed();
	virtual bool isComplete();
	virtual bool isComplete(MCFI* exsitingFiles);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void setHeader(MCFCore::MCFHeaderI* head);
	virtual void setHeader(DesuraId id, MCFBranch branch, MCFBuild build);	
	virtual void setFile(const char* file);
	virtual void setFile(const char* file, uint64 offset);
	virtual void setWorkerCount(uint16 count);
	virtual void disableCompression();
	virtual void addProvider(MCFCore::Misc::DownloadProvider* pov);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// File processing
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void parseFolder(const char *path, bool hashFile = false, bool reportProgress = false);
	virtual void parseMCF();
	virtual void saveMCF();
	virtual void saveFiles(const char* path );
	virtual bool verifyInstall(const char* path, bool flagMissing = false, bool useDiffs = false);
	virtual void removeFiles(const char* path,  bool removeNonSave = true);
	virtual void hashFiles();
	virtual void hashFiles(MCFI* inMcf);
	virtual bool crcCheck();
	virtual void makeCRC();
	virtual void removeIncompleteFiles();
	virtual void saveXml(const char* file);
	virtual void saveBlankMcf();
	virtual void saveMCFHeader();

	virtual void preAllocateFile();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Downloading
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void dlHeaderFromWeb();
	virtual void dlFilesFromWeb();
	virtual void dlHeaderFromHttp(const char* url);
	virtual void dlFilesFromHttp(const char* url, const char* installDir = NULL);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Threads
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void pause();
	virtual void unpause();
	virtual void stop();
	virtual bool isPaused();


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Mcf processing
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void getPatchStats(MCFI* inMcf, uint64* dlSize, uint32* fileCount);
	virtual void makePatch(MCFI* inMcf);
	virtual void makeFullFile(MCFI* inMcf, const char* path);
	virtual void makeBackPatchMCF(MCFI* inMcf, const char* path);
	virtual bool verifyMCF();
	virtual void copyMissingFiles(MCFI *sourceMcf);
	virtual void markFiles(MCFI* inMcf, bool tagSame, bool tagChanged, bool tagDeleted, bool tagNew);
	virtual void exportMcf(const char* path);
	virtual void markChanged(MCFI* inMcf);
	virtual void resetSavedFiles();

	virtual int32 verifyAll(const char* tempPath);
	virtual bool verifyUnitTest(MCFI* mcf);

	virtual void removeNonSavedFiles();
	virtual bool fixMD5AndCRC();
	virtual void optimiseAndSaveMcf(MCFI* prevMcf, const char* path);

	virtual uint64 getFileOffset();

	virtual void createCourgetteDiffs(MCFI* oldMcf, const char* outPath);

protected:
	//! A struct that holds the position of a MCFFile in two different Mcfs.
	//! Used for compariting two Mcf's
	//!
	typedef struct
	{
		uint32 thisMcf;		//!< Index in first mcf
		uint32 otherMcf;	//!< Index in second mcf
	} mcfDif_s;

	//! A struct that is used in the sorting of the mcf files
	//!
	struct file_sortkey
	{
		bool operator()(MCFCore::MCFFile *lhs, MCFCore::MCFFile *rhs )
		{
			return (lhs->getHash() < rhs->getHash());
		}
	};

	Event<MCFCore::Misc::ProgressInfo> onProgressEvent;	//!< Progress event
	Event<gcException> onErrorEvent;					//!< Error event
	Event<MCFCore::Misc::DP_s> onProviderEvent;						//!< Provider event

	//! Generates the xml for the file
	//!
	//! @param doc Xml Document node
	//!
	void genXml(XMLSaveAndCompress *sac);

	//! Parses an xml buffer and generates MCFFiles from it
	//!
	//! @param buff Xml buffer
	//! @param buffLen Buffer size
	//!
	void parseXml(char* buff, uint32 buffLen);

	//! Parases a folder generating MCFFiles. This is a recursive function
	//!
	//! @param path Path to the current folder
	//! @param oPath the original folder path
	//! @param hashFile Should make md5 hashs as it parses
	//!
	void parseFolder(const char *path, const char *oPath);

	//! Finds changes between a Mcf and this Mcf
	//!
	//! @param newFile Input mcf to check against
	//! @param vSame Vector to store the same files into
	//! @param vDiff Vector to store the diff files into
	//! @param vDel Vector to store the deleted files into (in this mcf not in newMcf)
	//! @param vNew Vector to store the new files into (in newMcf not in this mcf)
	//!
	void findChanges( MCF* newFile,  std::vector<mcfDif_s> *vSame, std::vector<mcfDif_s> *vDiff, std::vector<mcfDif_s> *vDel, std::vector<mcfDif_s> *vNew = NULL);

	//! Finds files in both that have the same hash
	//!
	void findSameHashFile(MCF* newFile, std::vector<mcfDif_s> &vSame, std::vector<size_t> &vOther);

	//! Copys one file and data from the MCF
	//!
	//! @param file Source mcf file
	//! @param lastOffset last offset in current mcf
	//! @param hFileSrc File handle to the source mcf
	//! @param hFileDest File handle to the dest mcf
	//!
	void copyFile(MCFCore::MCFFile* file, uint64 &lastOffset, UTIL::FS::FileHandle& hFileSrc, UTIL::FS::FileHandle& hFileDest);


	//! Finds a file by performing a binary search
	//!
	//! @param hash Hash to find
	//! @param f Index of first item
	//! @param l Index of last item
	//!
	uint32 binarySearch(uint64 hash, uint32 f, uint32 l);

	//! Reports an error to objects using the error event
	//!
	//! @param excpt Exception to report
	//!
	void error(gcException &excpt);

	//! A sub function for saveMCF which saves and compresses the files
	//!
	void saveMCF_CandSFiles();

	//! Setus up header to be saved
	//!
	void saveMCF_Header();

	//! Writes the header and xml to the MCF file
	//!
	void saveMCF_Header(char* xml, uint32 xmlSize, uint64 offset);

	//! This is used by the constuctors to init the mcf to a common state
	//!
	void init();

	//! Gets a read handle. Applys file offset for use with mcf embeded in another file
	void getReadHandle(UTIL::FS::FileHandle& handle);
	void getWriteHandle(UTIL::FS::FileHandle& handle);


	void createCourgetteDiff(CourgetteInstance* ci, UTIL::MISC::Buffer &oldBuff, UTIL::MISC::Buffer &newBuff, const char* oldHash, MCFFile* file, UTIL::FS::FileHandle& dest);
	void extractFile(const char* mcfPath, MCFFile* file, UTIL::MISC::Buffer &outBuff);

private:
	uint16 m_uiWCount;
	gcString m_szFile;

	Misc::GetFile_s* m_pFileAuth;

	volatile bool m_bStopped;
	volatile bool m_bPaused;

	uint32 m_iLastSorted;
	uint32 m_uiChunkCount;
	uint64 m_uiFileOffset;

	::Thread::BaseThread *m_pTHandle;

	MCFCore::MCFHeader* m_sHeader;
	std::vector<MCFCore::MCFFile*> m_pFileList;
	std::vector<MCFCore::Misc::DownloadProvider*> m_vProviderList;

	::Thread::Mutex m_mThreadMutex;
};


inline uint32 MCF::getFileCount()
{
	return (uint32)m_pFileList.size();
}

inline Misc::GetFile_s* MCF::getAuthInfo()
{
	return m_pFileAuth;
}

inline Event<MCFCore::Misc::ProgressInfo>& MCF::getProgEvent()
{
	return onProgressEvent;
}

inline Event<gcException>& MCF::getErrorEvent()
{
	return onErrorEvent;
}

inline Event<MCFCore::Misc::DP_s>& MCF::getNewProvider()
{
	return onProviderEvent;
}

inline void MCF::addProvider(MCFCore::Misc::DownloadProvider* pov)
{
	m_vProviderList.push_back(pov);
}

}

#endif

