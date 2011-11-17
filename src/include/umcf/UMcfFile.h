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

#ifndef DESURA_UMCF_FILE_H
#define DESURA_UMCF_FILE_H

#include <string.h>
#include "tinyxml.h"

#define UMCF_NAME_LEN 256
#define UMCF_PATH_LEN 2048
#define UMCF_FPATH_LEN MCF_NAME_LEN+MCF_PATH_LEN+MCF_PATH_LEN
#define UMCF_CSUM_LEN 33

#ifndef FILEHANDLE
#ifdef WIN32
	typedef HANDLE FILEHANDLE;
#else
	typedef FILE* FILEHANDLE;
#endif
#endif


enum
{
	UMCFF_NO_FLAG		= 0,
	UMCFF_FLAG_SAVE		= 1<<1,		//file is saved into the MCF
	UMCFF_FLAG_COMPLETE	= 1<<2,		//file is complete
	UMCFF_FLAG_COMPRESSED= 1<<3,		//file is compressed using bzip2
	UMCFF_FLAG_ZEROSIZE	= 1<<4,		//the file has no data
	UMCFF_FLAG_XECUTABLE = 1<<7,	//executable bit set for linux
};

enum
{
	MCFF_OK = 0,
	MCF_NOTCOMPRESSED,	//not really an error
	MCFF_ERR_BZ2CFAIL,
	MCFF_ERR_BZ2CFAIL_ZEROSIZE,
	MCFF_ERR_BZ2DFAIL,
	MCFF_ERR_INVALIDFILE,
	MCFF_ERR_FAILEDREAD,
	MCFF_ERR_FAILEDWRITE,
	MCFF_ERR_PARTREAD,
	MCFF_ERR_INVALIDHANDLE,
	MCFF_ERR_ZEROFILE,
	MCFF_ERR_ZEROSIZE,
	MCFF_ERR_PARTWRITE,
	MCFF_ERR_FAILEDDOWNLOAD,
	MCFF_INVALIDDATA,
	MCFF_ERR_INVALIDPATH,
	MCFF_ERR_BADHASH,
	MCFF_ERR_CANCELED,
};


class ProgressCB
{
public:
	ProgressCB(uint64 d)
	{
		cancel = false;
		done = d;
	}

	bool cancel;
	uint64 done;
};

class UMcfFile
{
public:
	UMcfFile();
	~UMcfFile();

	uint64 getSize() const 
	{
		return m_ullSize;
	}

	uint64 getCSize() const 
	{
		return m_ullCSize;
	}

	uint32 getHash() const  
	{
		return m_uiHash;
	}

	uint64 getTimeStamp() const 
	{
		return m_ullTimeStamp;
	}

	uint64 getOffSet() const  
	{
		return m_ullOffset;
	}

	void cleanData();

	const wchar_t* getName();
	const wchar_t* getPath();
	const wchar_t* getDir();
	const char* getCsum();
	const char* getCCsum();

	void setName(const wchar_t* var);
	void setPath(const wchar_t* var);
	void setCsum(const char* var);
	void setCCsum(const char* var);

	void setSize(uint64 size){m_ullSize = size;}
	void setTimeStamp(uint64 time){m_ullTimeStamp = time;}
	void setOffset(uint64 offset){ m_ullOffset = offset; }

	uint8 loadXmlData(TiXmlElement *xmlNode);

	//checks to see if file exists in dir
	bool checkFile(const wchar_t* dir);

	void addFlag(uint16 flag){m_uiFlags |= flag;}
	void delFlag(uint16 flag){m_uiFlags &= ~flag;}
	uint16 getFlags(){return m_uiFlags;}

	bool isSaved(){return (m_uiFlags & UMCFF_FLAG_SAVE)?true:false;}
	bool isComplete(){return (m_uiFlags & UMCFF_FLAG_COMPLETE)?true:false;}
	bool isCompressed(){return (m_uiFlags & UMCFF_FLAG_COMPRESSED)?true:false;}

	bool isSame(UMcfFile* file);

	void remove(const wchar_t* dir);

	void genXml(TiXmlElement *element);
	bool verifyFile(FILEHANDLE hFile, uint64 baseOffset);

	uint8 readMCFAndSave(FILEHANDLE hFile, const wchar_t* path, uint64 offset, DelegateI<ProgressCB> *del = NULL);
	uint8 saveFile(const wchar_t *dir);

protected:
	bool verify(const char* hash);

	uint8 saveData(FILEHANDLE hSrc, FILEHANDLE hSink, uint64 offset, DelegateI<ProgressCB> *del = NULL);
	uint8 decompressAndSave(FILEHANDLE hSrc, FILEHANDLE hSink, uint64 offset, DelegateI<ProgressCB> *del = NULL);

private:
	gcWString m_szName;
	gcWString m_szPath;
	gcString m_szCsum;
	gcString m_szCCsum;
	
	uint16 m_uiFlags;
	uint32 m_uiHash;

	uint64 m_ullTimeStamp;
	uint64 m_ullSize;
	uint64 m_ullCSize;
	uint64 m_ullOffset;
};

#endif

