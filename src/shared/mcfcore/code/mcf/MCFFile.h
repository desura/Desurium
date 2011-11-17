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



#ifndef DESURA_MCF_FILE_H
#define DESURA_MCF_FILE_H

#include "Common.h"
#include "mcfcore/MCFFileI.h"

#include <string.h>
#include "tinyxml.h"

class XMLSaveAndCompress;
class CourgetteInstance;

namespace MCFCore
{

extern const char* g_vExcludeFileList[];
extern const char* g_vExcludeDirList[];
extern const char* g_vNoCompressList[];

//! MCFFile represents one file that is stored inside a MCF
//! 
class CEXPORT MCFFile : public MCFFileI
{
public:
	//! Normal Constuctor
	//!
	MCFFile();

	//! Copy Constuctor
	//!
	MCFFile(MCFFile* tMCFFile);

	virtual ~MCFFile();

	//! Loads the file data from xml
	//!
	//! @param xmlNode node in the xml tree
	//!
	void loadXmlData(TiXmlElement *xmlNode);

	//! Generates xml from file data and appends it to element
	//!
	//! @param element node to attach new xml to
	//!
	void genXml(XMLSaveAndCompress *sac);



	//! Gets hash of full path (path+name) for use in compairing
	//!
	//! @return Hash
	//!
	uint64 getHash(){return m_iHash;}

	//! Gets the last modified time stamp of the file. Format is YYYYMMDDHHMMSS i.e. 20090128180010
	//!
	//! @return TimeStamp
	//!
	uint64 getTimeStamp(){return m_iTimeStamp;}

	//! Gets the offset of this file inside the MCF
	//!
	//! @return Offset
	//!
	uint64 getOffSet(){return m_llOffset;}

	//! Sets the name of the file
	//!
	//! @param var Name of file
	//!
	void setName(const char* var);

	//! Sets the path of the file from the root folder i.e. /scripts/python
	//!
	//! @param var Path of file
	//!
	void setPath(const char* var);

	//! Sets the directory of the root folder i.e. c:/root
	//!
	//! @param var Root path
	//!
	void setDir(const char *var);

	//! Sets the uncompressed md5 checksum
	//!
	//! @param var Uncompressed md5
	//!
	void setCsum(const char* var);

	//! Sets the compressed md5 checksum
	//!
	//! @param var Compressed md5
	//!
	void setCCsum(const char* var);

	//! Sets the uncompressed size of the file
	//!
	//! @param size Uncompressed size
	//!
	void setSize(uint64 size);

	//! Sets the compressed size of the file
	//!
	//! @param size Compressed size
	//!
	void setCSize(uint64 size);

	//! Sets the last modified timestamp of the file. Format is YYYYMMDDHHMMSS i.e. 20090128180010
	//!
	//! @param time Timestamp
	//!
	void setTimeStamp(uint64 time);

	//! Sets the offset of the file in the MCF
	//!
	//! @param offset Offset
	//!
	void setOffSet(uint64 offset);

	//! Prints all file information to console
	//!
	void printAll();


	//! Adds a flag to the file flags
	//!
	//! @param flag Flag to add
	//!
	void addFlag(uint16 flag);

	//! Removes a flag from the file flags
	//!
	//! @param flag Flag to remove
	//!
	void delFlag(uint16 flag);
	

	//! Checks to see if a file has started to download
	//!
	//! @return True if it has, false if it hasnt
	//!
	bool hasStartedDL();

	//! Checks to see if it should compress this file
	//!
	//! @return True if it should, false if it shouldnt
	//!
	bool shouldCompress();

	//! Checks to see if the save md5 matches the files md5
	//!
	//! @return True if it matches, false if it doesnt
	//!
	bool hashCheckFile();
	bool hashCheckFile(std::string* retHash);

	//! Generates a md5 hash from the file and saves it to this file
	//!
	void hashFile();

	//! Copys settings from another file to this file
	//!
	//! @param tMCFFile File to copy from
	//!
	void copySettings(MCFFile* tMCFFile);
	void copyBorkedSettings(MCFFile* tMCFFile);

	//! Checks to see if this file is completed inside a MCF. Sets a flag if true
	//!
	//! @param file Handle to the MCF file
	//! @param stop Cancel the verify
	//! @see verify()
	//!
	void verifyMcf(UTIL::FS::FileHandle& file, const volatile bool &stop);

	//! Checks to see if this file is complete on the computer. Sets a flag if true
	//!
	//! @see verify()
	void verifyFile(bool useDiffs = false);

	//! Deletes this file from the computer
	//!
	void delFile();

	//! Compairs two files and see if they are the same.
	//!
	//! @param file File to compair to
	//! @return Compair status (see FILE_CMP enum)
	//! @see MCFFileI
	//!
	uint8 isEquals( MCFFile * file);


	//! Gets the crc block size
	//!
	//! @return Block size
	//!
	uint32 getBlockSize(){return m_iBlockSize;}

	//! gets the crc at an index
	//!
	//! @param index Index in the vector of the crc you want to get
	//! @return Crc
	//!
	uint32 getCRC(uint32 index){return m_vCRCList[index];}

	//! Gets the total crc count
	//!
	//! @return Crc count
	//!
	uint32 getCRCCount(){return m_vCRCList.size();}

	//! Generates crc values from a MCF file
	//!
	//! @param file Handle for MCF file
	//!
	void generateCRC(UTIL::FS::FileHandle& file);

	//! Sets the crc list for this file from a vector
	//!
	//! @param vCRCList Crc vector
	//!
	void setCRC(std::vector<uint32> &vCRCList);

	//! Checks to see if a block crc matches the stored crc
	//!
	//! @param blockId Id of the block to generate the crc from
	//! @param file Handle for the MCF file
	//! @return True if it matches, false if it doesnt
	//!
	bool crcCheck(uint16 blockId, UTIL::FS::FileHandle& file);

	//! Checks to see if all stored crcs match the crcs of the file in the MCF
	//!
	//! @param file Handle of the MCF
	//! @return True if it matches, false if it doesnt
	//!
	bool crcCheck(UTIL::FS::FileHandle& file);

	//! Generates md5 from a MCF file
	//!
	//! @param file Handle for MCF file
	//!
	void generateMD5(UTIL::FS::FileHandle& file);


	void resetCRC();

	virtual bool hasDiff();
	virtual uint64 getDiffSize();
	virtual uint64 getDiffOffSet();
	virtual const char* getDiffHash();
	virtual const char* getDiffOrgFileHash();


	//interface overides
	const char* getName();
	const char* getPath();
	const char* getDir();
	const char* getCsum();
	const char* getCCsum();

	std::string getFullPath();

	uint64 getSize(){return m_iSize;}
	uint64 getCSize(){return m_iCSize;}
	uint64 getCurSize();

	bool isSaved(){return (m_uiFlags & FLAG_SAVE)?true:false;}
	bool isComplete(){return (m_uiFlags & FLAG_COMPLETE)?true:false;}
	bool isCompressed(){return (m_uiFlags & FLAG_COMPRESSED)?true:false;}
	bool isZeroSize(){return (m_uiFlags & FLAG_ZEROSIZE)?true:false;}

	uint16 getFlags(){return m_uiFlags;}

	void setDiffInfo(const char* orgHash, const char* hash, uint64 size);
	void setDiffOffset(uint64 offset);

	virtual void clearDiff();

protected:
	//! Sets the crc list for this file from a vector
	//!
	//! @param vCRCList Crc vector
	//!
	void verify(const char* hash, bool compressed = false, bool useDiffs = false);

	//! Generates hash of full path (path+name) for use in compairing
	//!
	void saveHash();

	//! This is used instead of crc checks if the MCF file doesnt have crc information
	//!
	//! @param buff Input buff to check
	//! @param size Size of buffer
	//! @return true if complete, false if incomplete
	//!
	bool legacyBlockCheck(const char* buff, uint32 size);

private:
	gcString m_szName;
	gcString m_szPath;
	gcString m_szDir;
	gcString m_szCsum;		//norm md5 checksum
	gcString m_szCCsum;	//compressed md5 checksum

	uint16 m_uiFlags;
	uint64 m_iHash;
	uint64 m_iSize;
	uint64 m_iCSize;
	uint64 m_iTimeStamp;
	uint64 m_llOffset;

	uint64 m_llDiffOffset;
	uint64 m_llDiffSize;
	gcString m_szDiffOrgFileHash;	//md5 of the file used to make this diff
	gcString m_szDiffHash;			//md5 of the diff section

	uint32 m_iBlockSize;
	std::vector<uint32> m_vCRCList;
};

}

#endif
