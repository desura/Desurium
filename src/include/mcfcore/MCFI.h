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

#ifndef DESURA_MCF_I_H
#define DESURA_MCF_I_H

#include "MCFHeaderI.h"
#include "MCFFileI.h"
#include "ProgressInfo.h"
#include "DownloadProvider.h"
#include "Event.h"

#define MCF_FACTORY "MCF_INTERFACE_001"

namespace MCFCore
{
namespace Misc
{ 
	class UserCookies; 
	class DownloadProvider;

	//! Structer to hold information for the download provider event
	typedef struct
	{
		DownloadProvider::PROVIDER action;	//!< Action (add or remove)
		DownloadProvider* provider;			//!< Provider information
	} DP_s;

	typedef struct
	{
		char authhash[33];
		char authkey[10];
	} GetFile_s;
}

//! MCFI is the interface file for MCF's. A MCF stores content for the desura application and allows part downloads 
//! and patching to be more effecent than normall http downloads.
//!
class MCFI
{
public:
	virtual ~MCFI()=0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Gets the header of this class.
	//! 
	//! @return Header object
	//!
	virtual MCFCore::MCFHeaderI* getHeader()=0;

	//! Gets the size of all files that need to be downloaded
	//!
	//! @return Download size
	//!
	virtual uint64 getDLSize()=0;

	//! Gets the size of all files that are in the mcf
	//!
	//! @return Download size
	//!
	virtual uint64 getINSize()=0;

	//! Gets the sum of all file sizes saved in the MCF 
	//!
	//! @return Total size
	//!
	virtual uint64 getFileSize()=0;

	//! Gets the total file count for this MCF
	//!
	//! @return File count
	//!
	virtual uint32 getFileCount()=0;

	//! Gets the file info for this file
	//!
	//! @param index File index
	//! @return Mcf file
	//!
	virtual MCFCore::MCFFileI* getMCFFile(uint32 index)=0;

	//! This gets the current file path for the MCF
	//!
	//! @return Mcf path
	//!
	virtual const char* getFile()=0;

	//! Gets all the download providers
	//!
	//! @param pCookies User cookies
	//! @param unauthed Is this an unauthed MCF
	//! @param local use local download mirror (admin only!)
	//!
	virtual void getDownloadProviders(const char* url, MCFCore::Misc::UserCookies *pCookies, bool *unauthed = NULL, bool local = false)=0;

	//! Gets the authirzation information for testing purpuses
	//!
	//! @return Auth information
	//!
	virtual Misc::GetFile_s* getAuthInfo()=0;

	//! Gets the progress event
	//!
	//! @return Progress event
	//!
	virtual Event<MCFCore::Misc::ProgressInfo>& getProgEvent()=0;

	//! Gets the error event
	//!
	//! @return Error event
	//!
	virtual Event<gcException>&	getErrorEvent()=0;

	//! Gets the new provider event (i.e. when downloads starts downloading from a server)
	//!
	//! @return Provider event
	//!
	virtual Event<MCFCore::Misc::DP_s>& getNewProvider()=0;

	//! See if we should compress the files for this MCF
	//!
	//! @return True if compressed, false if not
	//!
	virtual bool isCompressed()=0;

	//! Does this MCF contain every file
	//!
	//! @return True if complete, false if not
	//!
	virtual bool isComplete()=0;

	//! Does this MCF contain every file that is not in exsitingFiles
	//!
	//! @return True if complete, false if not
	//!
	virtual bool isComplete(MCFI* exsitingFiles)=0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//! Sets the header
	//!
	//! @param head Header object
	//!
	virtual void setHeader(MCFCore::MCFHeaderI* head)=0;

	//! Sets the header using raw information
	//!
	//! @param id DesuraId
	//! @param brach Mcf branch
	//! @param build Mcf content version
	//!
	virtual void setHeader(DesuraId id, MCFBranch branch, MCFBuild build)=0;	

	//! Sets the path and file name of the MCF for save and load
	//!
	//! @param file File name
	//!
	virtual void setFile(const char* file)=0;

	//! Overides the default worker count of 1 per core
	//! 
	//!  @param count Number of worker threads
	//!
	virtual void setWorkerCount(uint16 count)=0;

	//! Turns compression off. Use full for gamecore update MCF files.
	//!
	virtual void disableCompression()=0;

	//! Allows adding a provider for testing purpuses
	//! 
	//! @param pov Download provider to add to list
	//!
	virtual void addProvider(MCFCore::Misc::DownloadProvider* pov) = 0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// File processing
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//! Parsers a folder to get the info to build a MCF
	//!
	//! @param path Folder to parse
	//! @param hashFile Produce md5 hashs of the files
	//!
	virtual void parseFolder(const char *path, bool hashFile = false, bool reportProgress = false)=0;

	//! Parsers a MCF to get file info
	//! 
	virtual void parseMCF()=0;

	//! Saves the MCF to disk. Note: Must of run parseFolder before
	//!
	//! @param oldFolderPath Folder to use to create diffs from. 
	//!
	virtual void saveMCF()=0;

	//! Saves the files to disk.
	//! 
	//! @param path Path to save files to
	//!
	virtual void saveFiles(const char* path)=0;

	//! This makes sure all the files at path match the MCF files
	//!
	//! @param path Path to installed files
	//! @param flagMissing add a flag to missing files
	//! @return True if complete, false if not
	//!
	virtual bool verifyInstall(const char* path, bool flagMissing = false, bool useDiffs = false)=0;

	//! Removes the installed files from the local computer matching the files in the MCF
	//!
	//! @param path Path to installed files
	//! @param removeNonSave Remove files that are part of the install but not saved in the MCF
	//!
	virtual void removeFiles(const char* path,  bool removeNonSave = true)=0;

	//! Forces a rehash of files from the local disk
	//!
	virtual void hashFiles()=0;

	//! Forces a rehash of files from the local disk of only files that are also in inMcf
	//! This allows for quicker comparisons for patching
	//!
	//! @param inMcf Mcf to hash files against
	//!
	virtual void hashFiles(MCFI* inMcf)=0;

	//! Checks the crcs of files in the MCF
	//!
	//! @return True if all files pass crc checks, false if not
	//!
	virtual bool crcCheck()=0;

	//! Forces all MCF files to remake crc's. Usefull for mcfs that dont have crc's
	//!
	virtual void makeCRC()=0;

	//! Removes the save flag for all incomplete files
	//!
	virtual void removeIncompleteFiles()=0;

	//! Saves the xml portion of the MCF to a file
	//! 
	//! @param file File to save xml to
	//!
	virtual void saveXml(const char* file)=0;


	//! Saves the header and xml to a blank mcf with no files
	//!
	virtual void saveBlankMcf()=0;

	//! Saves changes to the current mcf header
	//!
	virtual void saveMCFHeader()=0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Downloading
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//! Downloads header and file list from web using MCF service
	//!
	virtual void dlHeaderFromWeb()=0;

	//! Downloads all files from web using MCF service
	//! 
	virtual void dlFilesFromWeb()=0;

	//! Downloads header and file list from web using http
	//!
	//! @param url Url to the MCF file to download the header from
	//!
	virtual void dlHeaderFromHttp(const char* url)=0;

	//! Downloads all files from web using http
	//!
	//! @param url Url to the MCF file to download the files from
	//! @param installDir dir used to expand diffs from
	//!
	virtual void dlFilesFromHttp(const char* url, const char* installDir = NULL)=0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Threads
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//! Pauses all threads
	//!
	virtual void pause()=0;

	//! Unpauses all threads
	//!
	virtual void unpause()=0;

	//! Stops current thread process. Undefined result. Make sure to delete MCF file
	//!
	virtual void stop()=0;

	//! Is threads currently paused
	//!
	virtual bool isPaused()=0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Mcf processing
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Gets the stats of creating a patch between two mcf's.
	//!
	//! @param inMcf MCF to check against
	//! @param dlSize Total download size of patch
	//! @param fileCount number of files included in patch
	//!
	virtual void getPatchStats(MCFI* inMcf, uint64* dlSize, uint32* fileCount)=0;

	//! This looks at another MCF file, and tags all duplicate files
	//!
	//! @param inMcf MCF to check against
	//!
	virtual void makePatch(MCFI* inMcf)=0;

	//! This copys a patch MCF into this and saves the full version at path
	//!
	//! @param inMcf Patch MCF 
	//! @param path path to save result MCF
	//!
	virtual void makeFullFile(MCFI* inMcf, const char* path)=0;

	//! This makes a backwards patch from two full MCF files.
	//! 
	//! @param inMcf Mcf file to compair against this one
	//! @param path Path to save result MCF
	//!
	virtual void makeBackPatchMCF(MCFI* inMcf, const char* path)=0;

	//! This makes sure all the files in the MCF are good
	//!
	//! @return True if good, false if not
	//! 
	virtual bool verifyMCF()=0;

	//! Copys missing files from the source to the current MCF
	//!
	//! @param sourceMcf Mcf to analise for new files
	//!
	virtual void copyMissingFiles(MCFI *sourceMcf)=0;


	//! This looks at another MCF file, and tags files (i.e. marked them as saved)
	//!
	//! @param inMcf MCF to check against
	//! @param tagSame tag all the same files
	//! @param tagChanged tag all the changed files
	//! @param tagDeleted tag all deleted files
	//! @param tagNew tag all new files
	//!
	virtual void markFiles(MCFI* inMcf, bool tagSame, bool tagChanged, bool tagDeleted, bool tagNew)=0;


	//! Takes the contents of this mcf and exports it to a new mcf with the newest header and crc check
	//!
	//! @param path New mcf path
	//!
	virtual void exportMcf(const char* path)=0;

	//! Marks all the files that are different to allow for easy removal. Not safe to save mcf after this!
	//!
	//! @param inMcf MCF to check against
	//!
	virtual void markChanged(MCFI* inMcf)=0;

	//! Resets all file flags to saved. For create mcf
	//!
	virtual void resetSavedFiles()=0;

	//! Verifys the mcf and verifys all files inside it. Prints errors to console.
	//! 
	//! @return Returns 0 if completed correctly. -x where x is number of files that failed. 1 if mcf header is invalid. 
	//!
	virtual int32 verifyAll(const char* tempPath)=0;

	//! Unit test only - not for external use
	//!
	virtual bool verifyUnitTest(MCFI* mcf)=0;

	//! Removed all the non saved files from the mcf
	//!
	virtual void removeNonSavedFiles()=0;

	//! Fixes the md5 and crc's of files
	//!
	virtual bool fixMD5AndCRC()=0;

	//! Moves all the changed files to the start of the mcf
	//!
	virtual void optimiseAndSaveMcf(MCFI* prevMcf, const char* path)=0;

	//! Set the mcf file with an offset
	//!
	virtual void setFile(const char* file, uint64 offset)=0;

	//! Creates a mcf to use with courgette diff
	//!
	//! @param oldMcf old version to compare against
	//! @param outPath path to save mcf with diff in it to
	//!
	virtual void createCourgetteDiffs(MCFI* oldMcf, const char* outPath)=0;

	//! Pre allocates the mcf. Usefull for large mcf's
	//!
	virtual void preAllocateFile()=0;
};

}

#endif
